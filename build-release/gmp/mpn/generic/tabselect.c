/* mpn_tabselect.

   THIS IS AN INTERNAL FUNCTION WITH A MUTABLE INTERFACE.  IT IS ONLY
   SAFE TO REACH THIS FUNCTION THROUGH DOCUMENTED INTERFACES.

Copyright 2007, 2008, 2009, 2011 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

The GNU MP Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.  */

#include "gmp.h"
#include "gmp-impl.h"


/* Select entry `which' from table `tab', which has nents entries, each `n'
   limbs.  Store the selected entry at rp.  Reads entire table to avoid
   side-channel information leaks.  O(n*nents).
   FIXME: Move to its own file.  */
void
mpn_tabselect (volatile mp_limb_t *rp, volatile mp_limb_t *tab, mp_size_t n,
	       mp_size_t nents, mp_size_t which)
{
  mp_size_t k, i;
  mp_limb_t mask;
  volatile mp_limb_t *tp;

  for (k = 0; k < nents; k++)
    {
      mask = -(mp_limb_t) (which == k);
      tp = tab + n * k;
      for (i = 0; i < n; i++)
	{
	  rp[i] = (rp[i] & ~mask) | (tp[i] & mask);
	}
    }
}
