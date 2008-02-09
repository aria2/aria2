/* Copyright (C) 1995, 1996, 1997, 2000, 2006 Free Software Foundation, Inc.
   Contributed by Bernd Schmidt <crux@Pool.Informatik.RWTH-Aachen.DE>, 1997.

   NOTE: The canonical source of this file is maintained with the GNU C
   Library.  Bugs can be reported to bug-glibc@gnu.org.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.  */

/* Tree search for red/black trees.
   The algorithm for adding nodes is taken from one of the many "Algorithms"
   books by Robert Sedgewick, although the implementation differs.
   The algorithm for deleting nodes can probably be found in a book named
   "Introduction to Algorithms" by Cormen/Leiserson/Rivest.  At least that's
   the book that my professor took most algorithms from during the "Data
   Structures" course...

   Totally public domain.  */

/* Red/black trees are binary trees in which the edges are colored either red
   or black.  They have the following properties:
   1. The number of black edges on every path from the root to a leaf is
      constant.
   2. No two red edges are adjacent.
   Therefore there is an upper bound on the length of every path, it's
   O(log n) where n is the number of nodes in the tree.  No path can be longer
   than 1+2*P where P is the length of the shortest path in the tree.
   Useful for the implementation:
   3. If one of the children of a node is NULL, then the other one is red
      (if it exists).

   In the implementation, not the edges are colored, but the nodes.  The color
   interpreted as the color of the edge leading to this node.  The color is
   meaningless for the root node, but we color the root node black for
   convenience.  All added nodes are red initially.

   Adding to a red/black tree is rather easy.  The right place is searched
   with a usual binary tree search.  Additionally, whenever a node N is
   reached that has two red successors, the successors are colored black and
   the node itself colored red.  This moves red edges up the tree where they
   pose less of a problem once we get to really insert the new node.  Changing
   N's color to red may violate rule 2, however, so rotations may become
   necessary to restore the invariants.  Adding a new red leaf may violate
   the same rule, so afterwards an additional check is run and the tree
   possibly rotated.

   Deleting is hairy.  There are mainly two nodes involved: the node to be
   deleted (n1), and another node that is to be unchained from the tree (n2).
   If n1 has a successor (the node with a smallest key that is larger than
   n1), then the successor becomes n2 and its contents are copied into n1,
   otherwise n1 becomes n2.
   Unchaining a node may violate rule 1: if n2 is black, one subtree is
   missing one black edge afterwards.  The algorithm must try to move this
   error upwards towards the root, so that the subtree that does not have
   enough black edges becomes the whole tree.  Once that happens, the error
   has disappeared.  It may not be necessary to go all the way up, since it
   is possible that rotations and recoloring can fix the error before that.

   Although the deletion algorithm must walk upwards through the tree, we
   do not store parent pointers in the nodes.  Instead, delete allocates a
   small array of parent pointers and fills it while descending the tree.
   Since we know that the length of a path is O(log n), where n is the number
   of nodes, this is likely to use less memory.  */

/* Tree rotations look like this:
      A                C
     / \              / \
    B   C            A   G
   / \ / \  -->     / \
   D E F G         B   F
                  / \
                 D   E

   In this case, A has been rotated left.  This preserves the ordering of the
   binary tree.  */

#include <config.h>

/* Specification.  */
#ifdef IN_LIBINTL
# include "tsearch.h"
#else
# include <search.h>
#endif

#include <stdlib.h>

typedef int (*__compar_fn_t) (const void *, const void *);
typedef void (*__action_fn_t) (const void *, VISIT, int);

#ifndef weak_alias
# define __tsearch tsearch
# define __tfind tfind
# define __tdelete tdelete
# define __twalk twalk
#endif

#ifndef internal_function
/* Inside GNU libc we mark some function in a special way.  In other
   environments simply ignore the marking.  */
# define internal_function
#endif

typedef struct node_t
{
  /* Callers expect this to be the first element in the structure - do not
     move!  */
  const void *key;
  struct node_t *left;
  struct node_t *right;
  unsigned int red:1;
} *node;
typedef const struct node_t *const_node;

#undef DEBUGGING

#ifdef DEBUGGING

/* Routines to check tree invariants.  */

#include <assert.h>

#define CHECK_TREE(a) check_tree(a)

static void
check_tree_recurse (node p, int d_sofar, int d_total)
{
  if (p == NULL)
    {
      assert (d_sofar == d_total);
      return;
    }

  check_tree_recurse (p->left, d_sofar + (p->left && !p->left->red), d_total);
  check_tree_recurse (p->right, d_sofar + (p->right && !p->right->red), d_total);
  if (p->left)
    assert (!(p->left->red && p->red));
  if (p->right)
    assert (!(p->right->red && p->red));
}

static void
check_tree (node root)
{
  int cnt = 0;
  node p;
  if (root == NULL)
    return;
  root->red = 0;
  for(p = root->left; p; p = p->left)
    cnt += !p->red;
  check_tree_recurse (root, 0, cnt);
}


#else

#define CHECK_TREE(a)

#endif

/* Possibly "split" a node with two red successors, and/or fix up two red
   edges in a row.  ROOTP is a pointer to the lowest node we visited, PARENTP
   and GPARENTP pointers to its parent/grandparent.  P_R and GP_R contain the
   comparison values that determined which way was taken in the tree to reach
   ROOTP.  MODE is 1 if we need not do the split, but must check for two red
   edges between GPARENTP and ROOTP.  */
static void
maybe_split_for_insert (node *rootp, node *parentp, node *gparentp,
			int p_r, int gp_r, int mode)
{
  node root = *rootp;
  node *rp, *lp;
  rp = &(*rootp)->right;
  lp = &(*rootp)->left;

  /* See if we have to split this node (both successors red).  */
  if (mode == 1
      || ((*rp) != NULL && (*lp) != NULL && (*rp)->red && (*lp)->red))
    {
      /* This node becomes red, its successors black.  */
      root->red = 1;
      if (*rp)
	(*rp)->red = 0;
      if (*lp)
	(*lp)->red = 0;

      /* If the parent of this node is also red, we have to do
	 rotations.  */
      if (parentp != NULL && (*parentp)->red)
	{
	  node gp = *gparentp;
	  node p = *parentp;
	  /* There are two main cases:
	     1. The edge types (left or right) of the two red edges differ.
	     2. Both red edges are of the same type.
	     There exist two symmetries of each case, so there is a total of
	     4 cases.  */
	  if ((p_r > 0) != (gp_r > 0))
	    {
	      /* Put the child at the top of the tree, with its parent
		 and grandparent as successors.  */
	      p->red = 1;
	      gp->red = 1;
	      root->red = 0;
	      if (p_r < 0)
		{
		  /* Child is left of parent.  */
		  p->left = *rp;
		  *rp = p;
		  gp->right = *lp;
		  *lp = gp;
		}
	      else
		{
		  /* Child is right of parent.  */
		  p->right = *lp;
		  *lp = p;
		  gp->left = *rp;
		  *rp = gp;
		}
	      *gparentp = root;
	    }
	  else
	    {
	      *gparentp = *parentp;
	      /* Parent becomes the top of the tree, grandparent and
		 child are its successors.  */
	      p->red = 0;
	      gp->red = 1;
	      if (p_r < 0)
		{
		  /* Left edges.  */
		  gp->left = p->right;
		  p->right = gp;
		}
	      else
		{
		  /* Right edges.  */
		  gp->right = p->left;
		  p->left = gp;
		}
	    }
	}
    }
}

/* Find or insert datum into search tree.
   KEY is the key to be located, ROOTP is the address of tree root,
   COMPAR the ordering function.  */
void *
__tsearch (const void *key, void **vrootp, __compar_fn_t compar)
{
  node q;
  node *parentp = NULL, *gparentp = NULL;
  node *rootp = (node *) vrootp;
  node *nextp;
  int r = 0, p_r = 0, gp_r = 0; /* No they might not, Mr Compiler.  */

  if (rootp == NULL)
    return NULL;

  /* This saves some additional tests below.  */
  if (*rootp != NULL)
    (*rootp)->red = 0;

  CHECK_TREE (*rootp);

  nextp = rootp;
  while (*nextp != NULL)
    {
      node root = *rootp;
      r = (*compar) (key, root->key);
      if (r == 0)
	return root;

      maybe_split_for_insert (rootp, parentp, gparentp, p_r, gp_r, 0);
      /* If that did any rotations, parentp and gparentp are now garbage.
	 That doesn't matter, because the values they contain are never
	 used again in that case.  */

      nextp = r < 0 ? &root->left : &root->right;
      if (*nextp == NULL)
	break;

      gparentp = parentp;
      parentp = rootp;
      rootp = nextp;

      gp_r = p_r;
      p_r = r;
    }

  q = (struct node_t *) malloc (sizeof (struct node_t));
  if (q != NULL)
    {
      *nextp = q;			/* link new node to old */
      q->key = key;			/* initialize new node */
      q->red = 1;
      q->left = q->right = NULL;

      if (nextp != rootp)
	/* There may be two red edges in a row now, which we must avoid by
	   rotating the tree.  */
	maybe_split_for_insert (nextp, rootp, parentp, r, p_r, 1);
    }

  return q;
}
#ifdef weak_alias
weak_alias (__tsearch, tsearch)
#endif


/* Find datum in search tree.
   KEY is the key to be located, ROOTP is the address of tree root,
   COMPAR the ordering function.  */
void *
__tfind (key, vrootp, compar)
     const void *key;
     void *const *vrootp;
     __compar_fn_t compar;
{
  node *rootp = (node *) vrootp;

  if (rootp == NULL)
    return NULL;

  CHECK_TREE (*rootp);

  while (*rootp != NULL)
    {
      node root = *rootp;
      int r;

      r = (*compar) (key, root->key);
      if (r == 0)
	return root;

      rootp = r < 0 ? &root->left : &root->right;
    }
  return NULL;
}
#ifdef weak_alias
weak_alias (__tfind, tfind)
#endif


/* Delete node with given key.
   KEY is the key to be deleted, ROOTP is the address of the root of tree,
   COMPAR the comparison function.  */
void *
__tdelete (const void *key, void **vrootp, __compar_fn_t compar)
{
  node p, q, r, retval;
  int cmp;
  node *rootp = (node *) vrootp;
  node root, unchained;
  /* Stack of nodes so we remember the parents without recursion.  It's
     _very_ unlikely that there are paths longer than 40 nodes.  The tree
     would need to have around 250.000 nodes.  */
  int stacksize = 100;
  int sp = 0;
  node *nodestack[100];

  if (rootp == NULL)
    return NULL;
  p = *rootp;
  if (p == NULL)
    return NULL;

  CHECK_TREE (p);

  while ((cmp = (*compar) (key, (*rootp)->key)) != 0)
    {
      if (sp == stacksize)
	abort ();

      nodestack[sp++] = rootp;
      p = *rootp;
      rootp = ((cmp < 0)
	       ? &(*rootp)->left
	       : &(*rootp)->right);
      if (*rootp == NULL)
	return NULL;
    }

  /* This is bogus if the node to be deleted is the root... this routine
     really should return an integer with 0 for success, -1 for failure
     and errno = ESRCH or something.  */
  retval = p;

  /* We don't unchain the node we want to delete. Instead, we overwrite
     it with its successor and unchain the successor.  If there is no
     successor, we really unchain the node to be deleted.  */

  root = *rootp;

  r = root->right;
  q = root->left;

  if (q == NULL || r == NULL)
    unchained = root;
  else
    {
      node *parent = rootp, *up = &root->right;
      for (;;)
	{
	  if (sp == stacksize)
	    abort ();
	  nodestack[sp++] = parent;
	  parent = up;
	  if ((*up)->left == NULL)
	    break;
	  up = &(*up)->left;
	}
      unchained = *up;
    }

  /* We know that either the left or right successor of UNCHAINED is NULL.
     R becomes the other one, it is chained into the parent of UNCHAINED.  */
  r = unchained->left;
  if (r == NULL)
    r = unchained->right;
  if (sp == 0)
    *rootp = r;
  else
    {
      q = *nodestack[sp-1];
      if (unchained == q->right)
	q->right = r;
      else
	q->left = r;
    }

  if (unchained != root)
    root->key = unchained->key;
  if (!unchained->red)
    {
      /* Now we lost a black edge, which means that the number of black
	 edges on every path is no longer constant.  We must balance the
	 tree.  */
      /* NODESTACK now contains all parents of R.  R is likely to be NULL
	 in the first iteration.  */
      /* NULL nodes are considered black throughout - this is necessary for
	 correctness.  */
      while (sp > 0 && (r == NULL || !r->red))
	{
	  node *pp = nodestack[sp - 1];
	  p = *pp;
	  /* Two symmetric cases.  */
	  if (r == p->left)
	    {
	      /* Q is R's brother, P is R's parent.  The subtree with root
		 R has one black edge less than the subtree with root Q.  */
	      q = p->right;
	      if (q->red)
		{
		  /* If Q is red, we know that P is black. We rotate P left
		     so that Q becomes the top node in the tree, with P below
		     it.  P is colored red, Q is colored black.
		     This action does not change the black edge count for any
		     leaf in the tree, but we will be able to recognize one
		     of the following situations, which all require that Q
		     is black.  */
		  q->red = 0;
		  p->red = 1;
		  /* Left rotate p.  */
		  p->right = q->left;
		  q->left = p;
		  *pp = q;
		  /* Make sure pp is right if the case below tries to use
		     it.  */
		  nodestack[sp++] = pp = &q->left;
		  q = p->right;
		}
	      /* We know that Q can't be NULL here.  We also know that Q is
		 black.  */
	      if ((q->left == NULL || !q->left->red)
		  && (q->right == NULL || !q->right->red))
		{
		  /* Q has two black successors.  We can simply color Q red.
		     The whole subtree with root P is now missing one black
		     edge.  Note that this action can temporarily make the
		     tree invalid (if P is red).  But we will exit the loop
		     in that case and set P black, which both makes the tree
		     valid and also makes the black edge count come out
		     right.  If P is black, we are at least one step closer
		     to the root and we'll try again the next iteration.  */
		  q->red = 1;
		  r = p;
		}
	      else
		{
		  /* Q is black, one of Q's successors is red.  We can
		     repair the tree with one operation and will exit the
		     loop afterwards.  */
		  if (q->right == NULL || !q->right->red)
		    {
		      /* The left one is red.  We perform the same action as
			 in maybe_split_for_insert where two red edges are
			 adjacent but point in different directions:
			 Q's left successor (let's call it Q2) becomes the
			 top of the subtree we are looking at, its parent (Q)
			 and grandparent (P) become its successors. The former
			 successors of Q2 are placed below P and Q.
			 P becomes black, and Q2 gets the color that P had.
			 This changes the black edge count only for node R and
			 its successors.  */
		      node q2 = q->left;
		      q2->red = p->red;
		      p->right = q2->left;
		      q->left = q2->right;
		      q2->right = q;
		      q2->left = p;
		      *pp = q2;
		      p->red = 0;
		    }
		  else
		    {
		      /* It's the right one.  Rotate P left. P becomes black,
			 and Q gets the color that P had.  Q's right successor
			 also becomes black.  This changes the black edge
			 count only for node R and its successors.  */
		      q->red = p->red;
		      p->red = 0;

		      q->right->red = 0;

		      /* left rotate p */
		      p->right = q->left;
		      q->left = p;
		      *pp = q;
		    }

		  /* We're done.  */
		  sp = 1;
		  r = NULL;
		}
	    }
	  else
	    {
	      /* Comments: see above.  */
	      q = p->left;
	      if (q->red)
		{
		  q->red = 0;
		  p->red = 1;
		  p->left = q->right;
		  q->right = p;
		  *pp = q;
		  nodestack[sp++] = pp = &q->right;
		  q = p->left;
		}
	      if ((q->right == NULL || !q->right->red)
		       && (q->left == NULL || !q->left->red))
		{
		  q->red = 1;
		  r = p;
		}
	      else
		{
		  if (q->left == NULL || !q->left->red)
		    {
		      node q2 = q->right;
		      q2->red = p->red;
		      p->left = q2->right;
		      q->right = q2->left;
		      q2->left = q;
		      q2->right = p;
		      *pp = q2;
		      p->red = 0;
		    }
		  else
		    {
		      q->red = p->red;
		      p->red = 0;
		      q->left->red = 0;
		      p->left = q->right;
		      q->right = p;
		      *pp = q;
		    }
		  sp = 1;
		  r = NULL;
		}
	    }
	  --sp;
	}
      if (r != NULL)
	r->red = 0;
    }

  free (unchained);
  return retval;
}
#ifdef weak_alias
weak_alias (__tdelete, tdelete)
#endif


/* Walk the nodes of a tree.
   ROOT is the root of the tree to be walked, ACTION the function to be
   called at each node.  LEVEL is the level of ROOT in the whole tree.  */
static void
internal_function
trecurse (const void *vroot, __action_fn_t action, int level)
{
  const_node root = (const_node) vroot;

  if (root->left == NULL && root->right == NULL)
    (*action) (root, leaf, level);
  else
    {
      (*action) (root, preorder, level);
      if (root->left != NULL)
	trecurse (root->left, action, level + 1);
      (*action) (root, postorder, level);
      if (root->right != NULL)
	trecurse (root->right, action, level + 1);
      (*action) (root, endorder, level);
    }
}


/* Walk the nodes of a tree.
   ROOT is the root of the tree to be walked, ACTION the function to be
   called at each node.  */
void
__twalk (const void *vroot, __action_fn_t action)
{
  const_node root = (const_node) vroot;

  CHECK_TREE (root);

  if (root != NULL && action != NULL)
    trecurse (root, action, 0);
}
#ifdef weak_alias
weak_alias (__twalk, twalk)
#endif


#ifdef _LIBC

/* The standardized functions miss an important functionality: the
   tree cannot be removed easily.  We provide a function to do this.  */
static void
internal_function
tdestroy_recurse (node root, __free_fn_t freefct)
{
  if (root->left != NULL)
    tdestroy_recurse (root->left, freefct);
  if (root->right != NULL)
    tdestroy_recurse (root->right, freefct);
  (*freefct) ((void *) root->key);
  /* Free the node itself.  */
  free (root);
}

void
__tdestroy (void *vroot, __free_fn_t freefct)
{
  node root = (node) vroot;

  CHECK_TREE (root);

  if (root != NULL)
    tdestroy_recurse (root, freefct);
}
weak_alias (__tdestroy, tdestroy)

#endif /* _LIBC */
