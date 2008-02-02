/* vsprintf with automatic memory allocation.
   Copyright (C) 1999, 2002-2007 Free Software Foundation, Inc.

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

/* This file can be parametrized with the following macros:
     VASNPRINTF         The name of the function being defined.
     FCHAR_T            The element type of the format string.
     DCHAR_T            The element type of the destination (result) string.
     FCHAR_T_ONLY_ASCII Set to 1 to enable verification that all characters
                        in the format string are ASCII. MUST be set if
                        FCHAR_T and DCHAR_T are not the same type.
     DIRECTIVE          Structure denoting a format directive.
                        Depends on FCHAR_T.
     DIRECTIVES         Structure denoting the set of format directives of a
                        format string.  Depends on FCHAR_T.
     PRINTF_PARSE       Function that parses a format string.
                        Depends on FCHAR_T.
     DCHAR_CPY          memcpy like function for DCHAR_T[] arrays.
     DCHAR_SET          memset like function for DCHAR_T[] arrays.
     DCHAR_MBSNLEN      mbsnlen like function for DCHAR_T[] arrays.
     SNPRINTF           The system's snprintf (or similar) function.
                        This may be either snprintf or swprintf.
     TCHAR_T            The element type of the argument and result string
                        of the said SNPRINTF function.  This may be either
                        char or wchar_t.  The code exploits that
                        sizeof (TCHAR_T) | sizeof (DCHAR_T) and
                        alignof (TCHAR_T) <= alignof (DCHAR_T).
     DCHAR_IS_TCHAR     Set to 1 if DCHAR_T and TCHAR_T are the same type.
     DCHAR_CONV_FROM_ENCODING A function to convert from char[] to DCHAR[].
     DCHAR_IS_UINT8_T   Set to 1 if DCHAR_T is uint8_t.
     DCHAR_IS_UINT16_T  Set to 1 if DCHAR_T is uint16_t.
     DCHAR_IS_UINT32_T  Set to 1 if DCHAR_T is uint32_t.  */

/* Tell glibc's <stdio.h> to provide a prototype for snprintf().
   This must come before <config.h> because <config.h> may include
   <features.h>, and once <features.h> has been included, it's too late.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE    1
#endif

#ifndef VASNPRINTF
# include <config.h>
#endif
#ifndef IN_LIBINTL
# include <alloca.h>
#endif

/* Specification.  */
#ifndef VASNPRINTF
# if WIDE_CHAR_VERSION
#  include "vasnwprintf.h"
# else
#  include "vasnprintf.h"
# endif
#endif

#include <locale.h>	/* localeconv() */
#include <stdio.h>	/* snprintf(), sprintf() */
#include <stdlib.h>	/* abort(), malloc(), realloc(), free() */
#include <string.h>	/* memcpy(), strlen() */
#include <errno.h>	/* errno */
#include <limits.h>	/* CHAR_BIT */
#include <float.h>	/* DBL_MAX_EXP, LDBL_MAX_EXP */
#if HAVE_NL_LANGINFO
# include <langinfo.h>
#endif
#ifndef VASNPRINTF
# if WIDE_CHAR_VERSION
#  include "wprintf-parse.h"
# else
#  include "printf-parse.h"
# endif
#endif

/* Checked size_t computations.  */
#include "xsize.h"

#if (NEED_PRINTF_DOUBLE || NEED_PRINTF_LONG_DOUBLE) && !defined IN_LIBINTL
# include <math.h>
# include "float+.h"
#endif

#if (NEED_PRINTF_DOUBLE || NEED_PRINTF_INFINITE_DOUBLE) && !defined IN_LIBINTL
# include <math.h>
# include "isnan.h"
#endif

#if (NEED_PRINTF_LONG_DOUBLE || NEED_PRINTF_INFINITE_LONG_DOUBLE) && !defined IN_LIBINTL
# include <math.h>
# include "isnanl-nolibm.h"
# include "fpucw.h"
#endif

#if (NEED_PRINTF_DIRECTIVE_A || NEED_PRINTF_DOUBLE) && !defined IN_LIBINTL
# include <math.h>
# include "isnan.h"
# include "printf-frexp.h"
#endif

#if (NEED_PRINTF_DIRECTIVE_A || NEED_PRINTF_LONG_DOUBLE) && !defined IN_LIBINTL
# include <math.h>
# include "isnanl-nolibm.h"
# include "printf-frexpl.h"
# include "fpucw.h"
#endif

/* Some systems, like OSF/1 4.0 and Woe32, don't have EOVERFLOW.  */
#ifndef EOVERFLOW
# define EOVERFLOW E2BIG
#endif

#if HAVE_WCHAR_T
# if HAVE_WCSLEN
#  define local_wcslen wcslen
# else
   /* Solaris 2.5.1 has wcslen() in a separate library libw.so. To avoid
      a dependency towards this library, here is a local substitute.
      Define this substitute only once, even if this file is included
      twice in the same compilation unit.  */
#  ifndef local_wcslen_defined
#   define local_wcslen_defined 1
static size_t
local_wcslen (const wchar_t *s)
{
  const wchar_t *ptr;

  for (ptr = s; *ptr != (wchar_t) 0; ptr++)
    ;
  return ptr - s;
}
#  endif
# endif
#endif

/* Default parameters.  */
#ifndef VASNPRINTF
# if WIDE_CHAR_VERSION
#  define VASNPRINTF vasnwprintf
#  define FCHAR_T wchar_t
#  define DCHAR_T wchar_t
#  define TCHAR_T wchar_t
#  define DCHAR_IS_TCHAR 1
#  define DIRECTIVE wchar_t_directive
#  define DIRECTIVES wchar_t_directives
#  define PRINTF_PARSE wprintf_parse
#  define DCHAR_CPY wmemcpy
# else
#  define VASNPRINTF vasnprintf
#  define FCHAR_T char
#  define DCHAR_T char
#  define TCHAR_T char
#  define DCHAR_IS_TCHAR 1
#  define DIRECTIVE char_directive
#  define DIRECTIVES char_directives
#  define PRINTF_PARSE printf_parse
#  define DCHAR_CPY memcpy
# endif
#endif
#if WIDE_CHAR_VERSION
  /* TCHAR_T is wchar_t.  */
# define USE_SNPRINTF 1
# if HAVE_DECL__SNWPRINTF
   /* On Windows, the function swprintf() has a different signature than
      on Unix; we use the _snwprintf() function instead.  */
#  define SNPRINTF _snwprintf
# else
   /* Unix.  */
#  define SNPRINTF swprintf
# endif
#else
  /* TCHAR_T is char.  */
# /* Use snprintf if it exists under the name 'snprintf' or '_snprintf'.
     But don't use it on BeOS, since BeOS snprintf produces no output if the
     size argument is >= 0x3000000.  */
# if (HAVE_DECL__SNPRINTF || HAVE_SNPRINTF) && !defined __BEOS__
#  define USE_SNPRINTF 1
# else
#  define USE_SNPRINTF 0
# endif
# if HAVE_DECL__SNPRINTF
   /* Windows.  */
#  define SNPRINTF _snprintf
# else
   /* Unix.  */
#  define SNPRINTF snprintf
   /* Here we need to call the native snprintf, not rpl_snprintf.  */
#  undef snprintf
# endif
#endif
/* Here we need to call the native sprintf, not rpl_sprintf.  */
#undef sprintf

#if (NEED_PRINTF_DIRECTIVE_A || NEED_PRINTF_LONG_DOUBLE || NEED_PRINTF_DOUBLE || NEED_PRINTF_INFINITE_DOUBLE) && !defined IN_LIBINTL
/* Determine the decimal-point character according to the current locale.  */
# ifndef decimal_point_char_defined
#  define decimal_point_char_defined 1
static char
decimal_point_char ()
{
  const char *point;
  /* Determine it in a multithread-safe way.  We know nl_langinfo is
     multithread-safe on glibc systems, but is not required to be multithread-
     safe by POSIX.  sprintf(), however, is multithread-safe.  localeconv()
     is rarely multithread-safe.  */
#  if HAVE_NL_LANGINFO && __GLIBC__
  point = nl_langinfo (RADIXCHAR);
#  elif 1
  char pointbuf[5];
  sprintf (pointbuf, "%#.0f", 1.0);
  point = &pointbuf[1];
#  else
  point = localeconv () -> decimal_point;
#  endif
  /* The decimal point is always a single byte: either '.' or ','.  */
  return (point[0] != '\0' ? point[0] : '.');
}
# endif
#endif

#if NEED_PRINTF_INFINITE_DOUBLE && !NEED_PRINTF_DOUBLE && !defined IN_LIBINTL

/* Equivalent to !isfinite(x) || x == 0, but does not require libm.  */
static int
is_infinite_or_zero (double x)
{
  return isnan (x) || x + x == x;
}

#endif

#if NEED_PRINTF_INFINITE_LONG_DOUBLE && !NEED_PRINTF_LONG_DOUBLE && !defined IN_LIBINTL

/* Equivalent to !isfinite(x), but does not require libm.  */
static int
is_infinitel (long double x)
{
  return isnanl (x) || (x + x == x && x != 0.0L);
}

#endif

#if (NEED_PRINTF_LONG_DOUBLE || NEED_PRINTF_DOUBLE) && !defined IN_LIBINTL

/* Converting 'long double' to decimal without rare rounding bugs requires
   real bignums.  We use the naming conventions of GNU gmp, but vastly simpler
   (and slower) algorithms.  */

typedef unsigned int mp_limb_t;
# define GMP_LIMB_BITS 32
typedef int mp_limb_verify[2 * (sizeof (mp_limb_t) * CHAR_BIT == GMP_LIMB_BITS) - 1];

typedef unsigned long long mp_twolimb_t;
# define GMP_TWOLIMB_BITS 64
typedef int mp_twolimb_verify[2 * (sizeof (mp_twolimb_t) * CHAR_BIT == GMP_TWOLIMB_BITS) - 1];

/* Representation of a bignum >= 0.  */
typedef struct
{
  size_t nlimbs;
  mp_limb_t *limbs; /* Bits in little-endian order, allocated with malloc().  */
} mpn_t;

/* Compute the product of two bignums >= 0.
   Return the allocated memory in case of success, NULL in case of memory
   allocation failure.  */
static void *
multiply (mpn_t src1, mpn_t src2, mpn_t *dest)
{
  const mp_limb_t *p1;
  const mp_limb_t *p2;
  size_t len1;
  size_t len2;

  if (src1.nlimbs <= src2.nlimbs)
    {
      len1 = src1.nlimbs;
      p1 = src1.limbs;
      len2 = src2.nlimbs;
      p2 = src2.limbs;
    }
  else
    {
      len1 = src2.nlimbs;
      p1 = src2.limbs;
      len2 = src1.nlimbs;
      p2 = src1.limbs;
    }
  /* Now 0 <= len1 <= len2.  */
  if (len1 == 0)
    {
      /* src1 or src2 is zero.  */
      dest->nlimbs = 0;
      dest->limbs = (mp_limb_t *) malloc (1);
    }
  else
    {
      /* Here 1 <= len1 <= len2.  */
      size_t dlen;
      mp_limb_t *dp;
      size_t k, i, j;

      dlen = len1 + len2;
      dp = (mp_limb_t *) malloc (dlen * sizeof (mp_limb_t));
      if (dp == NULL)
	return NULL;
      for (k = len2; k > 0; )
	dp[--k] = 0;
      for (i = 0; i < len1; i++)
	{
	  mp_limb_t digit1 = p1[i];
	  mp_twolimb_t carry = 0;
	  for (j = 0; j < len2; j++)
	    {
	      mp_limb_t digit2 = p2[j];
	      carry += (mp_twolimb_t) digit1 * (mp_twolimb_t) digit2;
	      carry += dp[i + j];
	      dp[i + j] = (mp_limb_t) carry;
	      carry = carry >> GMP_LIMB_BITS;
	    }
	  dp[i + len2] = (mp_limb_t) carry;
	}
      /* Normalise.  */
      while (dlen > 0 && dp[dlen - 1] == 0)
	dlen--;
      dest->nlimbs = dlen;
      dest->limbs = dp;
    }
  return dest->limbs;
}

/* Compute the quotient of a bignum a >= 0 and a bignum b > 0.
   a is written as  a = q * b + r  with 0 <= r < b.  q is the quotient, r
   the remainder.
   Finally, round-to-even is performed: If r > b/2 or if r = b/2 and q is odd,
   q is incremented.
   Return the allocated memory in case of success, NULL in case of memory
   allocation failure.  */
static void *
divide (mpn_t a, mpn_t b, mpn_t *q)
{
  /* Algorithm:
     First normalise a and b: a=[a[m-1],...,a[0]], b=[b[n-1],...,b[0]]
     with m>=0 and n>0 (in base beta = 2^GMP_LIMB_BITS).
     If m<n, then q:=0 and r:=a.
     If m>=n=1, perform a single-precision division:
       r:=0, j:=m,
       while j>0 do
         {Here (q[m-1]*beta^(m-1)+...+q[j]*beta^j) * b[0] + r*beta^j =
               = a[m-1]*beta^(m-1)+...+a[j]*beta^j und 0<=r<b[0]<beta}
         j:=j-1, r:=r*beta+a[j], q[j]:=floor(r/b[0]), r:=r-b[0]*q[j].
       Normalise [q[m-1],...,q[0]], yields q.
     If m>=n>1, perform a multiple-precision division:
       We have a/b < beta^(m-n+1).
       s:=intDsize-1-(hightest bit in b[n-1]), 0<=s<intDsize.
       Shift a and b left by s bits, copying them. r:=a.
       r=[r[m],...,r[0]], b=[b[n-1],...,b[0]] with b[n-1]>=beta/2.
       For j=m-n,...,0: {Here 0 <= r < b*beta^(j+1).}
         Compute q* :
           q* := floor((r[j+n]*beta+r[j+n-1])/b[n-1]).
           In case of overflow (q* >= beta) set q* := beta-1.
           Compute c2 := ((r[j+n]*beta+r[j+n-1]) - q* * b[n-1])*beta + r[j+n-2]
           and c3 := b[n-2] * q*.
           {We have 0 <= c2 < 2*beta^2, even 0 <= c2 < beta^2 if no overflow
            occurred.  Furthermore 0 <= c3 < beta^2.
            If there was overflow and
            r[j+n]*beta+r[j+n-1] - q* * b[n-1] >= beta, i.e. c2 >= beta^2,
            the next test can be skipped.}
           While c3 > c2, {Here 0 <= c2 < c3 < beta^2}
             Put q* := q* - 1, c2 := c2 + b[n-1]*beta, c3 := c3 - b[n-2].
           If q* > 0:
             Put r := r - b * q* * beta^j. In detail:
               [r[n+j],...,r[j]] := [r[n+j],...,r[j]] - q* * [b[n-1],...,b[0]].
               hence: u:=0, for i:=0 to n-1 do
                              u := u + q* * b[i],
                              r[j+i]:=r[j+i]-(u mod beta) (+ beta, if carry),
                              u:=u div beta (+ 1, if carry in subtraction)
                      r[n+j]:=r[n+j]-u.
               {Since always u = (q* * [b[i-1],...,b[0]] div beta^i) + 1
                               < q* + 1 <= beta,
                the carry u does not overflow.}
             If a negative carry occurs, put q* := q* - 1
               and [r[n+j],...,r[j]] := [r[n+j],...,r[j]] + [0,b[n-1],...,b[0]].
         Set q[j] := q*.
       Normalise [q[m-n],..,q[0]]; this yields the quotient q.
       Shift [r[n-1],...,r[0]] right by s bits and normalise; this yields the
       rest r.
       The room for q[j] can be allocated at the memory location of r[n+j].
     Finally, round-to-even:
       Shift r left by 1 bit.
       If r > b or if r = b and q[0] is odd, q := q+1.
   */
  const mp_limb_t *a_ptr = a.limbs;
  size_t a_len = a.nlimbs;
  const mp_limb_t *b_ptr = b.limbs;
  size_t b_len = b.nlimbs;
  mp_limb_t *roomptr;
  mp_limb_t *tmp_roomptr = NULL;
  mp_limb_t *q_ptr;
  size_t q_len;
  mp_limb_t *r_ptr;
  size_t r_len;

  /* Allocate room for a_len+2 digits.
     (Need a_len+1 digits for the real division and 1 more digit for the
     final rounding of q.)  */
  roomptr = (mp_limb_t *) malloc ((a_len + 2) * sizeof (mp_limb_t));
  if (roomptr == NULL)
    return NULL;

  /* Normalise a.  */
  while (a_len > 0 && a_ptr[a_len - 1] == 0)
    a_len--;

  /* Normalise b.  */
  for (;;)
    {
      if (b_len == 0)
	/* Division by zero.  */
	abort ();
      if (b_ptr[b_len - 1] == 0)
	b_len--;
      else
	break;
    }

  /* Here m = a_len >= 0 and n = b_len > 0.  */

  if (a_len < b_len)
    {
      /* m<n: trivial case.  q=0, r := copy of a.  */
      r_ptr = roomptr;
      r_len = a_len;
      memcpy (r_ptr, a_ptr, a_len * sizeof (mp_limb_t));
      q_ptr = roomptr + a_len;
      q_len = 0;
    }
  else if (b_len == 1)
    {
      /* n=1: single precision division.
	 beta^(m-1) <= a < beta^m  ==>  beta^(m-2) <= a/b < beta^m  */
      r_ptr = roomptr;
      q_ptr = roomptr + 1;
      {
	mp_limb_t den = b_ptr[0];
	mp_limb_t remainder = 0;
	const mp_limb_t *sourceptr = a_ptr + a_len;
	mp_limb_t *destptr = q_ptr + a_len;
	size_t count;
	for (count = a_len; count > 0; count--)
	  {
	    mp_twolimb_t num =
	      ((mp_twolimb_t) remainder << GMP_LIMB_BITS) | *--sourceptr;
	    *--destptr = num / den;
	    remainder = num % den;
	  }
	/* Normalise and store r.  */
	if (remainder > 0)
	  {
	    r_ptr[0] = remainder;
	    r_len = 1;
	  }
	else
	  r_len = 0;
	/* Normalise q.  */
	q_len = a_len;
	if (q_ptr[q_len - 1] == 0)
	  q_len--;
      }
    }
  else
    {
      /* n>1: multiple precision division.
	 beta^(m-1) <= a < beta^m, beta^(n-1) <= b < beta^n  ==>
	 beta^(m-n-1) <= a/b < beta^(m-n+1).  */
      /* Determine s.  */
      size_t s;
      {
	mp_limb_t msd = b_ptr[b_len - 1]; /* = b[n-1], > 0 */
	s = 31;
	if (msd >= 0x10000)
	  {
	    msd = msd >> 16;
	    s -= 16;
	  }
	if (msd >= 0x100)
	  {
	    msd = msd >> 8;
	    s -= 8;
	  }
	if (msd >= 0x10)
	  {
	    msd = msd >> 4;
	    s -= 4;
	  }
	if (msd >= 0x4)
	  {
	    msd = msd >> 2;
	    s -= 2;
	  }
	if (msd >= 0x2)
	  {
	    msd = msd >> 1;
	    s -= 1;
	  }
      }
      /* 0 <= s < GMP_LIMB_BITS.
	 Copy b, shifting it left by s bits.  */
      if (s > 0)
	{
	  tmp_roomptr = (mp_limb_t *) malloc (b_len * sizeof (mp_limb_t));
	  if (tmp_roomptr == NULL)
	    {
	      free (roomptr);
	      return NULL;
	    }
	  {
	    const mp_limb_t *sourceptr = b_ptr;
	    mp_limb_t *destptr = tmp_roomptr;
	    mp_twolimb_t accu = 0;
	    size_t count;
	    for (count = b_len; count > 0; count--)
	      {
		accu += (mp_twolimb_t) *sourceptr++ << s;
		*destptr++ = (mp_limb_t) accu;
		accu = accu >> GMP_LIMB_BITS;
	      }
	    /* accu must be zero, since that was how s was determined.  */
	    if (accu != 0)
	      abort ();
	  }
	  b_ptr = tmp_roomptr;
	}
      /* Copy a, shifting it left by s bits, yields r.
	 Memory layout:
	 At the beginning: r = roomptr[0..a_len],
	 at the end: r = roomptr[0..b_len-1], q = roomptr[b_len..a_len]  */
      r_ptr = roomptr;
      if (s == 0)
	{
	  memcpy (r_ptr, a_ptr, a_len * sizeof (mp_limb_t));
	  r_ptr[a_len] = 0;
	}
      else
	{
	  const mp_limb_t *sourceptr = a_ptr;
	  mp_limb_t *destptr = r_ptr;
	  mp_twolimb_t accu = 0;
	  size_t count;
	  for (count = a_len; count > 0; count--)
	    {
	      accu += (mp_twolimb_t) *sourceptr++ << s;
	      *destptr++ = (mp_limb_t) accu;
	      accu = accu >> GMP_LIMB_BITS;
	    }
	  *destptr++ = (mp_limb_t) accu;
	}
      q_ptr = roomptr + b_len;
      q_len = a_len - b_len + 1; /* q will have m-n+1 limbs */
      {
	size_t j = a_len - b_len; /* m-n */
	mp_limb_t b_msd = b_ptr[b_len - 1]; /* b[n-1] */
	mp_limb_t b_2msd = b_ptr[b_len - 2]; /* b[n-2] */
	mp_twolimb_t b_msdd = /* b[n-1]*beta+b[n-2] */
	  ((mp_twolimb_t) b_msd << GMP_LIMB_BITS) | b_2msd;
	/* Division loop, traversed m-n+1 times.
	   j counts down, b is unchanged, beta/2 <= b[n-1] < beta.  */
	for (;;)
	  {
	    mp_limb_t q_star;
	    mp_limb_t c1;
	    if (r_ptr[j + b_len] < b_msd) /* r[j+n] < b[n-1] ? */
	      {
		/* Divide r[j+n]*beta+r[j+n-1] by b[n-1], no overflow.  */
		mp_twolimb_t num =
		  ((mp_twolimb_t) r_ptr[j + b_len] << GMP_LIMB_BITS)
		  | r_ptr[j + b_len - 1];
		q_star = num / b_msd;
		c1 = num % b_msd;
	      }
	    else
	      {
		/* Overflow, hence r[j+n]*beta+r[j+n-1] >= beta*b[n-1].  */
		q_star = (mp_limb_t)~(mp_limb_t)0; /* q* = beta-1 */
		/* Test whether r[j+n]*beta+r[j+n-1] - (beta-1)*b[n-1] >= beta
		   <==> r[j+n]*beta+r[j+n-1] + b[n-1] >= beta*b[n-1]+beta
		   <==> b[n-1] < floor((r[j+n]*beta+r[j+n-1]+b[n-1])/beta)
		        {<= beta !}.
		   If yes, jump directly to the subtraction loop.
		   (Otherwise, r[j+n]*beta+r[j+n-1] - (beta-1)*b[n-1] < beta
		    <==> floor((r[j+n]*beta+r[j+n-1]+b[n-1])/beta) = b[n-1] ) */
		if (r_ptr[j + b_len] > b_msd
		    || (c1 = r_ptr[j + b_len - 1] + b_msd) < b_msd)
		  /* r[j+n] >= b[n-1]+1 or
		     r[j+n] = b[n-1] and the addition r[j+n-1]+b[n-1] gives a
		     carry.  */
		  goto subtract;
	      }
	    /* q_star = q*,
	       c1 = (r[j+n]*beta+r[j+n-1]) - q* * b[n-1] (>=0, <beta).  */
	    {
	      mp_twolimb_t c2 = /* c1*beta+r[j+n-2] */
		((mp_twolimb_t) c1 << GMP_LIMB_BITS) | r_ptr[j + b_len - 2];
	      mp_twolimb_t c3 = /* b[n-2] * q* */
		(mp_twolimb_t) b_2msd * (mp_twolimb_t) q_star;
	      /* While c2 < c3, increase c2 and decrease c3.
		 Consider c3-c2.  While it is > 0, decrease it by
		 b[n-1]*beta+b[n-2].  Because of b[n-1]*beta+b[n-2] >= beta^2/2
		 this can happen only twice.  */
	      if (c3 > c2)
		{
		  q_star = q_star - 1; /* q* := q* - 1 */
		  if (c3 - c2 > b_msdd)
		    q_star = q_star - 1; /* q* := q* - 1 */
		}
	    }
	    if (q_star > 0)
	      subtract:
	      {
		/* Subtract r := r - b * q* * beta^j.  */
		mp_limb_t cr;
		{
		  const mp_limb_t *sourceptr = b_ptr;
		  mp_limb_t *destptr = r_ptr + j;
		  mp_twolimb_t carry = 0;
		  size_t count;
		  for (count = b_len; count > 0; count--)
		    {
		      /* Here 0 <= carry <= q*.  */
		      carry =
			carry
			+ (mp_twolimb_t) q_star * (mp_twolimb_t) *sourceptr++
			+ (mp_limb_t) ~(*destptr);
		      /* Here 0 <= carry <= beta*q* + beta-1.  */
		      *destptr++ = ~(mp_limb_t) carry;
		      carry = carry >> GMP_LIMB_BITS; /* <= q* */
		    }
		  cr = (mp_limb_t) carry;
		}
		/* Subtract cr from r_ptr[j + b_len], then forget about
		   r_ptr[j + b_len].  */
		if (cr > r_ptr[j + b_len])
		  {
		    /* Subtraction gave a carry.  */
		    q_star = q_star - 1; /* q* := q* - 1 */
		    /* Add b back.  */
		    {
		      const mp_limb_t *sourceptr = b_ptr;
		      mp_limb_t *destptr = r_ptr + j;
		      mp_limb_t carry = 0;
		      size_t count;
		      for (count = b_len; count > 0; count--)
			{
			  mp_limb_t source1 = *sourceptr++;
			  mp_limb_t source2 = *destptr;
			  *destptr++ = source1 + source2 + carry;
			  carry =
			    (carry
			     ? source1 >= (mp_limb_t) ~source2
			     : source1 > (mp_limb_t) ~source2);
			}
		    }
		    /* Forget about the carry and about r[j+n].  */
		  }
	      }
	    /* q* is determined.  Store it as q[j].  */
	    q_ptr[j] = q_star;
	    if (j == 0)
	      break;
	    j--;
	  }
      }
      r_len = b_len;
      /* Normalise q.  */
      if (q_ptr[q_len - 1] == 0)
	q_len--;
# if 0 /* Not needed here, since we need r only to compare it with b/2, and
	  b is shifted left by s bits.  */
      /* Shift r right by s bits.  */
      if (s > 0)
	{
	  mp_limb_t ptr = r_ptr + r_len;
	  mp_twolimb_t accu = 0;
	  size_t count;
	  for (count = r_len; count > 0; count--)
	    {
	      accu = (mp_twolimb_t) (mp_limb_t) accu << GMP_LIMB_BITS;
	      accu += (mp_twolimb_t) *--ptr << (GMP_LIMB_BITS - s);
	      *ptr = (mp_limb_t) (accu >> GMP_LIMB_BITS);
	    }
	}
# endif
      /* Normalise r.  */
      while (r_len > 0 && r_ptr[r_len - 1] == 0)
	r_len--;
    }
  /* Compare r << 1 with b.  */
  if (r_len > b_len)
    goto increment_q;
  {
    size_t i;
    for (i = b_len;;)
      {
	mp_limb_t r_i =
	  (i <= r_len && i > 0 ? r_ptr[i - 1] >> (GMP_LIMB_BITS - 1) : 0)
	  | (i < r_len ? r_ptr[i] << 1 : 0);
	mp_limb_t b_i = (i < b_len ? b_ptr[i] : 0);
	if (r_i > b_i)
	  goto increment_q;
	if (r_i < b_i)
	  goto keep_q;
	if (i == 0)
	  break;
	i--;
      }
  }
  if (q_len > 0 && ((q_ptr[0] & 1) != 0))
    /* q is odd.  */
    increment_q:
    {
      size_t i;
      for (i = 0; i < q_len; i++)
	if (++(q_ptr[i]) != 0)
	  goto keep_q;
      q_ptr[q_len++] = 1;
    }
  keep_q:
  if (tmp_roomptr != NULL)
    free (tmp_roomptr);
  q->limbs = q_ptr;
  q->nlimbs = q_len;
  return roomptr;
}

/* Convert a bignum a >= 0, multiplied with 10^extra_zeroes, to decimal
   representation.
   Destroys the contents of a.
   Return the allocated memory - containing the decimal digits in low-to-high
   order, terminated with a NUL character - in case of success, NULL in case
   of memory allocation failure.  */
static char *
convert_to_decimal (mpn_t a, size_t extra_zeroes)
{
  mp_limb_t *a_ptr = a.limbs;
  size_t a_len = a.nlimbs;
  /* 0.03345 is slightly larger than log(2)/(9*log(10)).  */
  size_t c_len = 9 * ((size_t)(a_len * (GMP_LIMB_BITS * 0.03345f)) + 1);
  char *c_ptr = (char *) malloc (xsum (c_len, extra_zeroes));
  if (c_ptr != NULL)
    {
      char *d_ptr = c_ptr;
      for (; extra_zeroes > 0; extra_zeroes--)
	*d_ptr++ = '0';
      while (a_len > 0)
	{
	  /* Divide a by 10^9, in-place.  */
	  mp_limb_t remainder = 0;
	  mp_limb_t *ptr = a_ptr + a_len;
	  size_t count;
	  for (count = a_len; count > 0; count--)
	    {
	      mp_twolimb_t num =
		((mp_twolimb_t) remainder << GMP_LIMB_BITS) | *--ptr;
	      *ptr = num / 1000000000;
	      remainder = num % 1000000000;
	    }
	  /* Store the remainder as 9 decimal digits.  */
	  for (count = 9; count > 0; count--)
	    {
	      *d_ptr++ = '0' + (remainder % 10);
	      remainder = remainder / 10;
	    }
	  /* Normalize a.  */
	  if (a_ptr[a_len - 1] == 0)
	    a_len--;
	}
      /* Remove leading zeroes.  */
      while (d_ptr > c_ptr && d_ptr[-1] == '0')
	d_ptr--;
      /* But keep at least one zero.  */
      if (d_ptr == c_ptr)
	*d_ptr++ = '0';
      /* Terminate the string.  */
      *d_ptr = '\0';
    }
  return c_ptr;
}

# if NEED_PRINTF_LONG_DOUBLE

/* Assuming x is finite and >= 0:
   write x as x = 2^e * m, where m is a bignum.
   Return the allocated memory in case of success, NULL in case of memory
   allocation failure.  */
static void *
decode_long_double (long double x, int *ep, mpn_t *mp)
{
  mpn_t m;
  int exp;
  long double y;
  size_t i;

  /* Allocate memory for result.  */
  m.nlimbs = (LDBL_MANT_BIT + GMP_LIMB_BITS - 1) / GMP_LIMB_BITS;
  m.limbs = (mp_limb_t *) malloc (m.nlimbs * sizeof (mp_limb_t));
  if (m.limbs == NULL)
    return NULL;
  /* Split into exponential part and mantissa.  */
  y = frexpl (x, &exp);
  if (!(y >= 0.0L && y < 1.0L))
    abort ();
  /* x = 2^exp * y = 2^(exp - LDBL_MANT_BIT) * (y * LDBL_MANT_BIT), and the
     latter is an integer.  */
  /* Convert the mantissa (y * LDBL_MANT_BIT) to a sequence of limbs.
     I'm not sure whether it's safe to cast a 'long double' value between
     2^31 and 2^32 to 'unsigned int', therefore play safe and cast only
     'long double' values between 0 and 2^16 (to 'unsigned int' or 'int',
     doesn't matter).  */
#  if (LDBL_MANT_BIT % GMP_LIMB_BITS) != 0
#   if (LDBL_MANT_BIT % GMP_LIMB_BITS) > GMP_LIMB_BITS / 2
    {
      mp_limb_t hi, lo;
      y *= (mp_limb_t) 1 << (LDBL_MANT_BIT % (GMP_LIMB_BITS / 2));
      hi = (int) y;
      y -= hi;
      if (!(y >= 0.0L && y < 1.0L))
	abort ();
      y *= (mp_limb_t) 1 << (GMP_LIMB_BITS / 2);
      lo = (int) y;
      y -= lo;
      if (!(y >= 0.0L && y < 1.0L))
	abort ();
      m.limbs[LDBL_MANT_BIT / GMP_LIMB_BITS] = (hi << (GMP_LIMB_BITS / 2)) | lo;
    }
#   else
    {
      mp_limb_t d;
      y *= (mp_limb_t) 1 << (LDBL_MANT_BIT % GMP_LIMB_BITS);
      d = (int) y;
      y -= d;
      if (!(y >= 0.0L && y < 1.0L))
	abort ();
      m.limbs[LDBL_MANT_BIT / GMP_LIMB_BITS] = d;
    }
#   endif
#  endif
  for (i = LDBL_MANT_BIT / GMP_LIMB_BITS; i > 0; )
    {
      mp_limb_t hi, lo;
      y *= (mp_limb_t) 1 << (GMP_LIMB_BITS / 2);
      hi = (int) y;
      y -= hi;
      if (!(y >= 0.0L && y < 1.0L))
	abort ();
      y *= (mp_limb_t) 1 << (GMP_LIMB_BITS / 2);
      lo = (int) y;
      y -= lo;
      if (!(y >= 0.0L && y < 1.0L))
	abort ();
      m.limbs[--i] = (hi << (GMP_LIMB_BITS / 2)) | lo;
    }
  if (!(y == 0.0L))
    abort ();
  /* Normalise.  */
  while (m.nlimbs > 0 && m.limbs[m.nlimbs - 1] == 0)
    m.nlimbs--;
  *mp = m;
  *ep = exp - LDBL_MANT_BIT;
  return m.limbs;
}

# endif

# if NEED_PRINTF_DOUBLE

/* Assuming x is finite and >= 0:
   write x as x = 2^e * m, where m is a bignum.
   Return the allocated memory in case of success, NULL in case of memory
   allocation failure.  */
static void *
decode_double (double x, int *ep, mpn_t *mp)
{
  mpn_t m;
  int exp;
  double y;
  size_t i;

  /* Allocate memory for result.  */
  m.nlimbs = (DBL_MANT_BIT + GMP_LIMB_BITS - 1) / GMP_LIMB_BITS;
  m.limbs = (mp_limb_t *) malloc (m.nlimbs * sizeof (mp_limb_t));
  if (m.limbs == NULL)
    return NULL;
  /* Split into exponential part and mantissa.  */
  y = frexp (x, &exp);
  if (!(y >= 0.0 && y < 1.0))
    abort ();
  /* x = 2^exp * y = 2^(exp - DBL_MANT_BIT) * (y * DBL_MANT_BIT), and the
     latter is an integer.  */
  /* Convert the mantissa (y * DBL_MANT_BIT) to a sequence of limbs.
     I'm not sure whether it's safe to cast a 'double' value between
     2^31 and 2^32 to 'unsigned int', therefore play safe and cast only
     'double' values between 0 and 2^16 (to 'unsigned int' or 'int',
     doesn't matter).  */
#  if (DBL_MANT_BIT % GMP_LIMB_BITS) != 0
#   if (DBL_MANT_BIT % GMP_LIMB_BITS) > GMP_LIMB_BITS / 2
    {
      mp_limb_t hi, lo;
      y *= (mp_limb_t) 1 << (DBL_MANT_BIT % (GMP_LIMB_BITS / 2));
      hi = (int) y;
      y -= hi;
      if (!(y >= 0.0 && y < 1.0))
	abort ();
      y *= (mp_limb_t) 1 << (GMP_LIMB_BITS / 2);
      lo = (int) y;
      y -= lo;
      if (!(y >= 0.0 && y < 1.0))
	abort ();
      m.limbs[DBL_MANT_BIT / GMP_LIMB_BITS] = (hi << (GMP_LIMB_BITS / 2)) | lo;
    }
#   else
    {
      mp_limb_t d;
      y *= (mp_limb_t) 1 << (DBL_MANT_BIT % GMP_LIMB_BITS);
      d = (int) y;
      y -= d;
      if (!(y >= 0.0 && y < 1.0))
	abort ();
      m.limbs[DBL_MANT_BIT / GMP_LIMB_BITS] = d;
    }
#   endif
#  endif
  for (i = DBL_MANT_BIT / GMP_LIMB_BITS; i > 0; )
    {
      mp_limb_t hi, lo;
      y *= (mp_limb_t) 1 << (GMP_LIMB_BITS / 2);
      hi = (int) y;
      y -= hi;
      if (!(y >= 0.0 && y < 1.0))
	abort ();
      y *= (mp_limb_t) 1 << (GMP_LIMB_BITS / 2);
      lo = (int) y;
      y -= lo;
      if (!(y >= 0.0 && y < 1.0))
	abort ();
      m.limbs[--i] = (hi << (GMP_LIMB_BITS / 2)) | lo;
    }
  if (!(y == 0.0))
    abort ();
  /* Normalise.  */
  while (m.nlimbs > 0 && m.limbs[m.nlimbs - 1] == 0)
    m.nlimbs--;
  *mp = m;
  *ep = exp - DBL_MANT_BIT;
  return m.limbs;
}

# endif

/* Assuming x = 2^e * m is finite and >= 0, and n is an integer:
   Returns the decimal representation of round (x * 10^n).
   Return the allocated memory - containing the decimal digits in low-to-high
   order, terminated with a NUL character - in case of success, NULL in case
   of memory allocation failure.  */
static char *
scale10_round_decimal_decoded (int e, mpn_t m, void *memory, int n)
{
  int s;
  size_t extra_zeroes;
  unsigned int abs_n;
  unsigned int abs_s;
  mp_limb_t *pow5_ptr;
  size_t pow5_len;
  unsigned int s_limbs;
  unsigned int s_bits;
  mpn_t pow5;
  mpn_t z;
  void *z_memory;
  char *digits;

  if (memory == NULL)
    return NULL;
  /* x = 2^e * m, hence
     y = round (2^e * 10^n * m) = round (2^(e+n) * 5^n * m)
       = round (2^s * 5^n * m).  */
  s = e + n;
  extra_zeroes = 0;
  /* Factor out a common power of 10 if possible.  */
  if (s > 0 && n > 0)
    {
      extra_zeroes = (s < n ? s : n);
      s -= extra_zeroes;
      n -= extra_zeroes;
    }
  /* Here y = round (2^s * 5^n * m) * 10^extra_zeroes.
     Before converting to decimal, we need to compute
     z = round (2^s * 5^n * m).  */
  /* Compute 5^|n|, possibly shifted by |s| bits if n and s have the same
     sign.  2.322 is slightly larger than log(5)/log(2).  */
  abs_n = (n >= 0 ? n : -n);
  abs_s = (s >= 0 ? s : -s);
  pow5_ptr = (mp_limb_t *) malloc (((int)(abs_n * (2.322f / GMP_LIMB_BITS)) + 1
				    + abs_s / GMP_LIMB_BITS + 1)
				   * sizeof (mp_limb_t));
  if (pow5_ptr == NULL)
    {
      free (memory);
      return NULL;
    }
  /* Initialize with 1.  */
  pow5_ptr[0] = 1;
  pow5_len = 1;
  /* Multiply with 5^|n|.  */
  if (abs_n > 0)
    {
      static mp_limb_t const small_pow5[13 + 1] =
	{
	  1, 5, 25, 125, 625, 3125, 15625, 78125, 390625, 1953125, 9765625,
	  48828125, 244140625, 1220703125
	};
      unsigned int n13;
      for (n13 = 0; n13 <= abs_n; n13 += 13)
	{
	  mp_limb_t digit1 = small_pow5[n13 + 13 <= abs_n ? 13 : abs_n - n13];
	  size_t j;
	  mp_twolimb_t carry = 0;
	  for (j = 0; j < pow5_len; j++)
	    {
	      mp_limb_t digit2 = pow5_ptr[j];
	      carry += (mp_twolimb_t) digit1 * (mp_twolimb_t) digit2;
	      pow5_ptr[j] = (mp_limb_t) carry;
	      carry = carry >> GMP_LIMB_BITS;
	    }
	  if (carry > 0)
	    pow5_ptr[pow5_len++] = (mp_limb_t) carry;
	}
    }
  s_limbs = abs_s / GMP_LIMB_BITS;
  s_bits = abs_s % GMP_LIMB_BITS;
  if (n >= 0 ? s >= 0 : s <= 0)
    {
      /* Multiply with 2^|s|.  */
      if (s_bits > 0)
	{
	  mp_limb_t *ptr = pow5_ptr;
	  mp_twolimb_t accu = 0;
	  size_t count;
	  for (count = pow5_len; count > 0; count--)
	    {
	      accu += (mp_twolimb_t) *ptr << s_bits;
	      *ptr++ = (mp_limb_t) accu;
	      accu = accu >> GMP_LIMB_BITS;
	    }
	  if (accu > 0)
	    {
	      *ptr = (mp_limb_t) accu;
	      pow5_len++;
	    }
	}
      if (s_limbs > 0)
	{
	  size_t count;
	  for (count = pow5_len; count > 0;)
	    {
	      count--;
	      pow5_ptr[s_limbs + count] = pow5_ptr[count];
	    }
	  for (count = s_limbs; count > 0;)
	    {
	      count--;
	      pow5_ptr[count] = 0;
	    }
	  pow5_len += s_limbs;
	}
      pow5.limbs = pow5_ptr;
      pow5.nlimbs = pow5_len;
      if (n >= 0)
	{
	  /* Multiply m with pow5.  No division needed.  */
	  z_memory = multiply (m, pow5, &z);
	}
      else
	{
	  /* Divide m by pow5 and round.  */
	  z_memory = divide (m, pow5, &z);
	}
    }
  else
    {
      pow5.limbs = pow5_ptr;
      pow5.nlimbs = pow5_len;
      if (n >= 0)
	{
	  /* n >= 0, s < 0.
	     Multiply m with pow5, then divide by 2^|s|.  */
	  mpn_t numerator;
	  mpn_t denominator;
	  void *tmp_memory;
	  tmp_memory = multiply (m, pow5, &numerator);
	  if (tmp_memory == NULL)
	    {
	      free (pow5_ptr);
	      free (memory);
	      return NULL;
	    }
	  /* Construct 2^|s|.  */
	  {
	    mp_limb_t *ptr = pow5_ptr + pow5_len;
	    size_t i;
	    for (i = 0; i < s_limbs; i++)
	      ptr[i] = 0;
	    ptr[s_limbs] = (mp_limb_t) 1 << s_bits;
	    denominator.limbs = ptr;
	    denominator.nlimbs = s_limbs + 1;
	  }
	  z_memory = divide (numerator, denominator, &z);
	  free (tmp_memory);
	}
      else
	{
	  /* n < 0, s > 0.
	     Multiply m with 2^s, then divide by pow5.  */
	  mpn_t numerator;
	  mp_limb_t *num_ptr;
	  num_ptr = (mp_limb_t *) malloc ((m.nlimbs + s_limbs + 1)
					  * sizeof (mp_limb_t));
	  if (num_ptr == NULL)
	    {
	      free (pow5_ptr);
	      free (memory);
	      return NULL;
	    }
	  {
	    mp_limb_t *destptr = num_ptr;
	    {
	      size_t i;
	      for (i = 0; i < s_limbs; i++)
		*destptr++ = 0;
	    }
	    if (s_bits > 0)
	      {
		const mp_limb_t *sourceptr = m.limbs;
		mp_twolimb_t accu = 0;
		size_t count;
		for (count = m.nlimbs; count > 0; count--)
		  {
		    accu += (mp_twolimb_t) *sourceptr++ << s_bits;
		    *destptr++ = (mp_limb_t) accu;
		    accu = accu >> GMP_LIMB_BITS;
		  }
		if (accu > 0)
		  *destptr++ = (mp_limb_t) accu;
	      }
	    else
	      {
		const mp_limb_t *sourceptr = m.limbs;
		size_t count;
		for (count = m.nlimbs; count > 0; count--)
		  *destptr++ = *sourceptr++;
	      }
	    numerator.limbs = num_ptr;
	    numerator.nlimbs = destptr - num_ptr;
	  }
	  z_memory = divide (numerator, pow5, &z);
	  free (num_ptr);
	}
    }
  free (pow5_ptr);
  free (memory);

  /* Here y = round (x * 10^n) = z * 10^extra_zeroes.  */

  if (z_memory == NULL)
    return NULL;
  digits = convert_to_decimal (z, extra_zeroes);
  free (z_memory);
  return digits;
}

# if NEED_PRINTF_LONG_DOUBLE

/* Assuming x is finite and >= 0, and n is an integer:
   Returns the decimal representation of round (x * 10^n).
   Return the allocated memory - containing the decimal digits in low-to-high
   order, terminated with a NUL character - in case of success, NULL in case
   of memory allocation failure.  */
static char *
scale10_round_decimal_long_double (long double x, int n)
{
  int e;
  mpn_t m;
  void *memory = decode_long_double (x, &e, &m);
  return scale10_round_decimal_decoded (e, m, memory, n);
}

# endif

# if NEED_PRINTF_DOUBLE

/* Assuming x is finite and >= 0, and n is an integer:
   Returns the decimal representation of round (x * 10^n).
   Return the allocated memory - containing the decimal digits in low-to-high
   order, terminated with a NUL character - in case of success, NULL in case
   of memory allocation failure.  */
static char *
scale10_round_decimal_double (double x, int n)
{
  int e;
  mpn_t m;
  void *memory = decode_double (x, &e, &m);
  return scale10_round_decimal_decoded (e, m, memory, n);
}

# endif

# if NEED_PRINTF_LONG_DOUBLE

/* Assuming x is finite and > 0:
   Return an approximation for n with 10^n <= x < 10^(n+1).
   The approximation is usually the right n, but may be off by 1 sometimes.  */
static int
floorlog10l (long double x)
{
  int exp;
  long double y;
  double z;
  double l;

  /* Split into exponential part and mantissa.  */
  y = frexpl (x, &exp);
  if (!(y >= 0.0L && y < 1.0L))
    abort ();
  if (y == 0.0L)
    return INT_MIN;
  if (y < 0.5L)
    {
      while (y < (1.0L / (1 << (GMP_LIMB_BITS / 2)) / (1 << (GMP_LIMB_BITS / 2))))
	{
	  y *= 1.0L * (1 << (GMP_LIMB_BITS / 2)) * (1 << (GMP_LIMB_BITS / 2));
	  exp -= GMP_LIMB_BITS;
	}
      if (y < (1.0L / (1 << 16)))
	{
	  y *= 1.0L * (1 << 16);
	  exp -= 16;
	}
      if (y < (1.0L / (1 << 8)))
	{
	  y *= 1.0L * (1 << 8);
	  exp -= 8;
	}
      if (y < (1.0L / (1 << 4)))
	{
	  y *= 1.0L * (1 << 4);
	  exp -= 4;
	}
      if (y < (1.0L / (1 << 2)))
	{
	  y *= 1.0L * (1 << 2);
	  exp -= 2;
	}
      if (y < (1.0L / (1 << 1)))
	{
	  y *= 1.0L * (1 << 1);
	  exp -= 1;
	}
    }
  if (!(y >= 0.5L && y < 1.0L))
    abort ();
  /* Compute an approximation for l = log2(x) = exp + log2(y).  */
  l = exp;
  z = y;
  if (z < 0.70710678118654752444)
    {
      z *= 1.4142135623730950488;
      l -= 0.5;
    }
  if (z < 0.8408964152537145431)
    {
      z *= 1.1892071150027210667;
      l -= 0.25;
    }
  if (z < 0.91700404320467123175)
    {
      z *= 1.0905077326652576592;
      l -= 0.125;
    }
  if (z < 0.9576032806985736469)
    {
      z *= 1.0442737824274138403;
      l -= 0.0625;
    }
  /* Now 0.95 <= z <= 1.01.  */
  z = 1 - z;
  /* log(1-z) = - z - z^2/2 - z^3/3 - z^4/4 - ...
     Four terms are enough to get an approximation with error < 10^-7.  */
  l -= z * (1.0 + z * (0.5 + z * ((1.0 / 3) + z * 0.25)));
  /* Finally multiply with log(2)/log(10), yields an approximation for
     log10(x).  */
  l *= 0.30102999566398119523;
  /* Round down to the next integer.  */
  return (int) l + (l < 0 ? -1 : 0);
}

# endif

# if NEED_PRINTF_DOUBLE

/* Assuming x is finite and > 0:
   Return an approximation for n with 10^n <= x < 10^(n+1).
   The approximation is usually the right n, but may be off by 1 sometimes.  */
static int
floorlog10 (double x)
{
  int exp;
  double y;
  double z;
  double l;

  /* Split into exponential part and mantissa.  */
  y = frexp (x, &exp);
  if (!(y >= 0.0 && y < 1.0))
    abort ();
  if (y == 0.0)
    return INT_MIN;
  if (y < 0.5)
    {
      while (y < (1.0 / (1 << (GMP_LIMB_BITS / 2)) / (1 << (GMP_LIMB_BITS / 2))))
	{
	  y *= 1.0 * (1 << (GMP_LIMB_BITS / 2)) * (1 << (GMP_LIMB_BITS / 2));
	  exp -= GMP_LIMB_BITS;
	}
      if (y < (1.0 / (1 << 16)))
	{
	  y *= 1.0 * (1 << 16);
	  exp -= 16;
	}
      if (y < (1.0 / (1 << 8)))
	{
	  y *= 1.0 * (1 << 8);
	  exp -= 8;
	}
      if (y < (1.0 / (1 << 4)))
	{
	  y *= 1.0 * (1 << 4);
	  exp -= 4;
	}
      if (y < (1.0 / (1 << 2)))
	{
	  y *= 1.0 * (1 << 2);
	  exp -= 2;
	}
      if (y < (1.0 / (1 << 1)))
	{
	  y *= 1.0 * (1 << 1);
	  exp -= 1;
	}
    }
  if (!(y >= 0.5 && y < 1.0))
    abort ();
  /* Compute an approximation for l = log2(x) = exp + log2(y).  */
  l = exp;
  z = y;
  if (z < 0.70710678118654752444)
    {
      z *= 1.4142135623730950488;
      l -= 0.5;
    }
  if (z < 0.8408964152537145431)
    {
      z *= 1.1892071150027210667;
      l -= 0.25;
    }
  if (z < 0.91700404320467123175)
    {
      z *= 1.0905077326652576592;
      l -= 0.125;
    }
  if (z < 0.9576032806985736469)
    {
      z *= 1.0442737824274138403;
      l -= 0.0625;
    }
  /* Now 0.95 <= z <= 1.01.  */
  z = 1 - z;
  /* log(1-z) = - z - z^2/2 - z^3/3 - z^4/4 - ...
     Four terms are enough to get an approximation with error < 10^-7.  */
  l -= z * (1.0 + z * (0.5 + z * ((1.0 / 3) + z * 0.25)));
  /* Finally multiply with log(2)/log(10), yields an approximation for
     log10(x).  */
  l *= 0.30102999566398119523;
  /* Round down to the next integer.  */
  return (int) l + (l < 0 ? -1 : 0);
}

# endif

#endif

DCHAR_T *
VASNPRINTF (DCHAR_T *resultbuf, size_t *lengthp,
	    const FCHAR_T *format, va_list args)
{
  DIRECTIVES d;
  arguments a;

  if (PRINTF_PARSE (format, &d, &a) < 0)
    /* errno is already set.  */
    return NULL;

#define CLEANUP() \
  free (d.dir);								\
  if (a.arg)								\
    free (a.arg);

  if (PRINTF_FETCHARGS (args, &a) < 0)
    {
      CLEANUP ();
      errno = EINVAL;
      return NULL;
    }

  {
    size_t buf_neededlength;
    TCHAR_T *buf;
    TCHAR_T *buf_malloced;
    const FCHAR_T *cp;
    size_t i;
    DIRECTIVE *dp;
    /* Output string accumulator.  */
    DCHAR_T *result;
    size_t allocated;
    size_t length;

    /* Allocate a small buffer that will hold a directive passed to
       sprintf or snprintf.  */
    buf_neededlength =
      xsum4 (7, d.max_width_length, d.max_precision_length, 6);
#if HAVE_ALLOCA
    if (buf_neededlength < 4000 / sizeof (TCHAR_T))
      {
	buf = (TCHAR_T *) alloca (buf_neededlength * sizeof (TCHAR_T));
	buf_malloced = NULL;
      }
    else
#endif
      {
	size_t buf_memsize = xtimes (buf_neededlength, sizeof (TCHAR_T));
	if (size_overflow_p (buf_memsize))
	  goto out_of_memory_1;
	buf = (TCHAR_T *) malloc (buf_memsize);
	if (buf == NULL)
	  goto out_of_memory_1;
	buf_malloced = buf;
      }

    if (resultbuf != NULL)
      {
	result = resultbuf;
	allocated = *lengthp;
      }
    else
      {
	result = NULL;
	allocated = 0;
      }
    length = 0;
    /* Invariants:
       result is either == resultbuf or == NULL or malloc-allocated.
       If length > 0, then result != NULL.  */

    /* Ensures that allocated >= needed.  Aborts through a jump to
       out_of_memory if needed is SIZE_MAX or otherwise too big.  */
#define ENSURE_ALLOCATION(needed) \
    if ((needed) > allocated)						     \
      {									     \
	size_t memory_size;						     \
	DCHAR_T *memory;						     \
									     \
	allocated = (allocated > 0 ? xtimes (allocated, 2) : 12);	     \
	if ((needed) > allocated)					     \
	  allocated = (needed);						     \
	memory_size = xtimes (allocated, sizeof (DCHAR_T));		     \
	if (size_overflow_p (memory_size))				     \
	  goto out_of_memory;						     \
	if (result == resultbuf || result == NULL)			     \
	  memory = (DCHAR_T *) malloc (memory_size);			     \
	else								     \
	  memory = (DCHAR_T *) realloc (result, memory_size);		     \
	if (memory == NULL)						     \
	  goto out_of_memory;						     \
	if (result == resultbuf && length > 0)				     \
	  DCHAR_CPY (memory, result, length);				     \
	result = memory;						     \
      }

    for (cp = format, i = 0, dp = &d.dir[0]; ; cp = dp->dir_end, i++, dp++)
      {
	if (cp != dp->dir_start)
	  {
	    size_t n = dp->dir_start - cp;
	    size_t augmented_length = xsum (length, n);

	    ENSURE_ALLOCATION (augmented_length);
	    /* This copies a piece of FCHAR_T[] into a DCHAR_T[].  Here we
	       need that the format string contains only ASCII characters
	       if FCHAR_T and DCHAR_T are not the same type.  */
	    if (sizeof (FCHAR_T) == sizeof (DCHAR_T))
	      {
		DCHAR_CPY (result + length, (const DCHAR_T *) cp, n);
		length = augmented_length;
	      }
	    else
	      {
		do
		  result[length++] = (unsigned char) *cp++;
		while (--n > 0);
	      }
	  }
	if (i == d.count)
	  break;

	/* Execute a single directive.  */
	if (dp->conversion == '%')
	  {
	    size_t augmented_length;

	    if (!(dp->arg_index == ARG_NONE))
	      abort ();
	    augmented_length = xsum (length, 1);
	    ENSURE_ALLOCATION (augmented_length);
	    result[length] = '%';
	    length = augmented_length;
	  }
	else
	  {
	    if (!(dp->arg_index != ARG_NONE))
	      abort ();

	    if (dp->conversion == 'n')
	      {
		switch (a.arg[dp->arg_index].type)
		  {
		  case TYPE_COUNT_SCHAR_POINTER:
		    *a.arg[dp->arg_index].a.a_count_schar_pointer = length;
		    break;
		  case TYPE_COUNT_SHORT_POINTER:
		    *a.arg[dp->arg_index].a.a_count_short_pointer = length;
		    break;
		  case TYPE_COUNT_INT_POINTER:
		    *a.arg[dp->arg_index].a.a_count_int_pointer = length;
		    break;
		  case TYPE_COUNT_LONGINT_POINTER:
		    *a.arg[dp->arg_index].a.a_count_longint_pointer = length;
		    break;
#if HAVE_LONG_LONG_INT
		  case TYPE_COUNT_LONGLONGINT_POINTER:
		    *a.arg[dp->arg_index].a.a_count_longlongint_pointer = length;
		    break;
#endif
		  default:
		    abort ();
		  }
	      }
#if ENABLE_UNISTDIO
	    /* The unistdio extensions.  */
	    else if (dp->conversion == 'U')
	      {
		arg_type type = a.arg[dp->arg_index].type;
		int flags = dp->flags;
		int has_width;
		size_t width;
		int has_precision;
		size_t precision;

		has_width = 0;
		width = 0;
		if (dp->width_start != dp->width_end)
		  {
		    if (dp->width_arg_index != ARG_NONE)
		      {
			int arg;

			if (!(a.arg[dp->width_arg_index].type == TYPE_INT))
			  abort ();
			arg = a.arg[dp->width_arg_index].a.a_int;
			if (arg < 0)
			  {
			    /* "A negative field width is taken as a '-' flag
			        followed by a positive field width."  */
			    flags |= FLAG_LEFT;
			    width = (unsigned int) (-arg);
			  }
			else
			  width = arg;
		      }
		    else
		      {
			const FCHAR_T *digitp = dp->width_start;

			do
			  width = xsum (xtimes (width, 10), *digitp++ - '0');
			while (digitp != dp->width_end);
		      }
		    has_width = 1;
		  }

		has_precision = 0;
		precision = 0;
		if (dp->precision_start != dp->precision_end)
		  {
		    if (dp->precision_arg_index != ARG_NONE)
		      {
			int arg;

			if (!(a.arg[dp->precision_arg_index].type == TYPE_INT))
			  abort ();
			arg = a.arg[dp->precision_arg_index].a.a_int;
			/* "A negative precision is taken as if the precision
			    were omitted."  */
			if (arg >= 0)
			  {
			    precision = arg;
			    has_precision = 1;
			  }
		      }
		    else
		      {
			const FCHAR_T *digitp = dp->precision_start + 1;

			precision = 0;
			while (digitp != dp->precision_end)
			  precision = xsum (xtimes (precision, 10), *digitp++ - '0');
			has_precision = 1;
		      }
		  }

		switch (type)
		  {
		  case TYPE_U8_STRING:
		    {
		      const uint8_t *arg = a.arg[dp->arg_index].a.a_u8_string;
		      const uint8_t *arg_end;
		      size_t characters;

		      if (has_precision)
			{
			  /* Use only PRECISION characters, from the left.  */
			  arg_end = arg;
			  characters = 0;
			  for (; precision > 0; precision--)
			    {
			      int count = u8_strmblen (arg_end);
			      if (count == 0)
				break;
			      if (count < 0)
				{
				  if (!(result == resultbuf || result == NULL))
				    free (result);
				  if (buf_malloced != NULL)
				    free (buf_malloced);
				  CLEANUP ();
				  errno = EILSEQ;
				  return NULL;
				}
			      arg_end += count;
			      characters++;
			    }
			}
		      else if (has_width)
			{
			  /* Use the entire string, and count the number of
			     characters.  */
			  arg_end = arg;
			  characters = 0;
			  for (;;)
			    {
			      int count = u8_strmblen (arg_end);
			      if (count == 0)
				break;
			      if (count < 0)
				{
				  if (!(result == resultbuf || result == NULL))
				    free (result);
				  if (buf_malloced != NULL)
				    free (buf_malloced);
				  CLEANUP ();
				  errno = EILSEQ;
				  return NULL;
				}
			      arg_end += count;
			      characters++;
			    }
			}
		      else
			{
			  /* Use the entire string.  */
			  arg_end = arg + u8_strlen (arg);
			  /* The number of characters doesn't matter.  */
			  characters = 0;
			}

		      if (has_width && width > characters
			  && !(dp->flags & FLAG_LEFT))
			{
			  size_t n = width - characters;
			  ENSURE_ALLOCATION (xsum (length, n));
			  DCHAR_SET (result + length, ' ', n);
			  length += n;
			}

# if DCHAR_IS_UINT8_T
		      {
			size_t n = arg_end - arg;
			ENSURE_ALLOCATION (xsum (length, n));
			DCHAR_CPY (result + length, arg, n);
			length += n;
		      }
# else
		      { /* Convert.  */
			DCHAR_T *converted = result + length;
			size_t converted_len = allocated - length;
#  if DCHAR_IS_TCHAR
			/* Convert from UTF-8 to locale encoding.  */
			if (u8_conv_to_encoding (locale_charset (),
						 iconveh_question_mark,
						 arg, arg_end - arg, NULL,
						 &converted, &converted_len)
			    < 0)
#  else
			/* Convert from UTF-8 to UTF-16/UTF-32.  */
			converted =
			  U8_TO_DCHAR (arg, arg_end - arg,
				       converted, &converted_len);
			if (converted == NULL)
#  endif
			  {
			    int saved_errno = errno;
			    if (!(result == resultbuf || result == NULL))
			      free (result);
			    if (buf_malloced != NULL)
			      free (buf_malloced);
			    CLEANUP ();
			    errno = saved_errno;
			    return NULL;
			  }
			if (converted != result + length)
			  {
			    ENSURE_ALLOCATION (xsum (length, converted_len));
			    DCHAR_CPY (result + length, converted, converted_len);
			    free (converted);
			  }
			length += converted_len;
		      }
# endif

		      if (has_width && width > characters
			  && (dp->flags & FLAG_LEFT))
			{
			  size_t n = width - characters;
			  ENSURE_ALLOCATION (xsum (length, n));
			  DCHAR_SET (result + length, ' ', n);
			  length += n;
			}
		    }
		    break;

		  case TYPE_U16_STRING:
		    {
		      const uint16_t *arg = a.arg[dp->arg_index].a.a_u16_string;
		      const uint16_t *arg_end;
		      size_t characters;

		      if (has_precision)
			{
			  /* Use only PRECISION characters, from the left.  */
			  arg_end = arg;
			  characters = 0;
			  for (; precision > 0; precision--)
			    {
			      int count = u16_strmblen (arg_end);
			      if (count == 0)
				break;
			      if (count < 0)
				{
				  if (!(result == resultbuf || result == NULL))
				    free (result);
				  if (buf_malloced != NULL)
				    free (buf_malloced);
				  CLEANUP ();
				  errno = EILSEQ;
				  return NULL;
				}
			      arg_end += count;
			      characters++;
			    }
			}
		      else if (has_width)
			{
			  /* Use the entire string, and count the number of
			     characters.  */
			  arg_end = arg;
			  characters = 0;
			  for (;;)
			    {
			      int count = u16_strmblen (arg_end);
			      if (count == 0)
				break;
			      if (count < 0)
				{
				  if (!(result == resultbuf || result == NULL))
				    free (result);
				  if (buf_malloced != NULL)
				    free (buf_malloced);
				  CLEANUP ();
				  errno = EILSEQ;
				  return NULL;
				}
			      arg_end += count;
			      characters++;
			    }
			}
		      else
			{
			  /* Use the entire string.  */
			  arg_end = arg + u16_strlen (arg);
			  /* The number of characters doesn't matter.  */
			  characters = 0;
			}

		      if (has_width && width > characters
			  && !(dp->flags & FLAG_LEFT))
			{
			  size_t n = width - characters;
			  ENSURE_ALLOCATION (xsum (length, n));
			  DCHAR_SET (result + length, ' ', n);
			  length += n;
			}

# if DCHAR_IS_UINT16_T
		      {
			size_t n = arg_end - arg;
			ENSURE_ALLOCATION (xsum (length, n));
			DCHAR_CPY (result + length, arg, n);
			length += n;
		      }
# else
		      { /* Convert.  */
			DCHAR_T *converted = result + length;
			size_t converted_len = allocated - length;
#  if DCHAR_IS_TCHAR
			/* Convert from UTF-16 to locale encoding.  */
			if (u16_conv_to_encoding (locale_charset (),
						  iconveh_question_mark,
						  arg, arg_end - arg, NULL,
						  &converted, &converted_len)
			    < 0)
#  else
			/* Convert from UTF-16 to UTF-8/UTF-32.  */
			converted =
			  U16_TO_DCHAR (arg, arg_end - arg,
					converted, &converted_len);
			if (converted == NULL)
#  endif
			  {
			    int saved_errno = errno;
			    if (!(result == resultbuf || result == NULL))
			      free (result);
			    if (buf_malloced != NULL)
			      free (buf_malloced);
			    CLEANUP ();
			    errno = saved_errno;
			    return NULL;
			  }
			if (converted != result + length)
			  {
			    ENSURE_ALLOCATION (xsum (length, converted_len));
			    DCHAR_CPY (result + length, converted, converted_len);
			    free (converted);
			  }
			length += converted_len;
		      }
# endif

		      if (has_width && width > characters
			  && (dp->flags & FLAG_LEFT))
			{
			  size_t n = width - characters;
			  ENSURE_ALLOCATION (xsum (length, n));
			  DCHAR_SET (result + length, ' ', n);
			  length += n;
			}
		    }
		    break;

		  case TYPE_U32_STRING:
		    {
		      const uint32_t *arg = a.arg[dp->arg_index].a.a_u32_string;
		      const uint32_t *arg_end;
		      size_t characters;

		      if (has_precision)
			{
			  /* Use only PRECISION characters, from the left.  */
			  arg_end = arg;
			  characters = 0;
			  for (; precision > 0; precision--)
			    {
			      int count = u32_strmblen (arg_end);
			      if (count == 0)
				break;
			      if (count < 0)
				{
				  if (!(result == resultbuf || result == NULL))
				    free (result);
				  if (buf_malloced != NULL)
				    free (buf_malloced);
				  CLEANUP ();
				  errno = EILSEQ;
				  return NULL;
				}
			      arg_end += count;
			      characters++;
			    }
			}
		      else if (has_width)
			{
			  /* Use the entire string, and count the number of
			     characters.  */
			  arg_end = arg;
			  characters = 0;
			  for (;;)
			    {
			      int count = u32_strmblen (arg_end);
			      if (count == 0)
				break;
			      if (count < 0)
				{
				  if (!(result == resultbuf || result == NULL))
				    free (result);
				  if (buf_malloced != NULL)
				    free (buf_malloced);
				  CLEANUP ();
				  errno = EILSEQ;
				  return NULL;
				}
			      arg_end += count;
			      characters++;
			    }
			}
		      else
			{
			  /* Use the entire string.  */
			  arg_end = arg + u32_strlen (arg);
			  /* The number of characters doesn't matter.  */
			  characters = 0;
			}

		      if (has_width && width > characters
			  && !(dp->flags & FLAG_LEFT))
			{
			  size_t n = width - characters;
			  ENSURE_ALLOCATION (xsum (length, n));
			  DCHAR_SET (result + length, ' ', n);
			  length += n;
			}

# if DCHAR_IS_UINT32_T
		      {
			size_t n = arg_end - arg;
			ENSURE_ALLOCATION (xsum (length, n));
			DCHAR_CPY (result + length, arg, n);
			length += n;
		      }
# else
		      { /* Convert.  */
			DCHAR_T *converted = result + length;
			size_t converted_len = allocated - length;
#  if DCHAR_IS_TCHAR
			/* Convert from UTF-32 to locale encoding.  */
			if (u32_conv_to_encoding (locale_charset (),
						  iconveh_question_mark,
						  arg, arg_end - arg, NULL,
						  &converted, &converted_len)
			    < 0)
#  else
			/* Convert from UTF-32 to UTF-8/UTF-16.  */
			converted =
			  U32_TO_DCHAR (arg, arg_end - arg,
					converted, &converted_len);
			if (converted == NULL)
#  endif
			  {
			    int saved_errno = errno;
			    if (!(result == resultbuf || result == NULL))
			      free (result);
			    if (buf_malloced != NULL)
			      free (buf_malloced);
			    CLEANUP ();
			    errno = saved_errno;
			    return NULL;
			  }
			if (converted != result + length)
			  {
			    ENSURE_ALLOCATION (xsum (length, converted_len));
			    DCHAR_CPY (result + length, converted, converted_len);
			    free (converted);
			  }
			length += converted_len;
		      }
# endif

		      if (has_width && width > characters
			  && (dp->flags & FLAG_LEFT))
			{
			  size_t n = width - characters;
			  ENSURE_ALLOCATION (xsum (length, n));
			  DCHAR_SET (result + length, ' ', n);
			  length += n;
			}
		    }
		    break;

		  default:
		    abort ();
		  }
	      }
#endif
#if (NEED_PRINTF_DIRECTIVE_A || NEED_PRINTF_LONG_DOUBLE || NEED_PRINTF_DOUBLE) && !defined IN_LIBINTL
	    else if ((dp->conversion == 'a' || dp->conversion == 'A')
# if !(NEED_PRINTF_DIRECTIVE_A || (NEED_PRINTF_LONG_DOUBLE && NEED_PRINTF_DOUBLE))
		     && (0
#  if NEED_PRINTF_DOUBLE
			 || a.arg[dp->arg_index].type == TYPE_DOUBLE
#  endif
#  if NEED_PRINTF_LONG_DOUBLE
			 || a.arg[dp->arg_index].type == TYPE_LONGDOUBLE
#  endif
			)
# endif
		    )
	      {
		arg_type type = a.arg[dp->arg_index].type;
		int flags = dp->flags;
		int has_width;
		size_t width;
		int has_precision;
		size_t precision;
		size_t tmp_length;
		DCHAR_T tmpbuf[700];
		DCHAR_T *tmp;
		DCHAR_T *pad_ptr;
		DCHAR_T *p;

		has_width = 0;
		width = 0;
		if (dp->width_start != dp->width_end)
		  {
		    if (dp->width_arg_index != ARG_NONE)
		      {
			int arg;

			if (!(a.arg[dp->width_arg_index].type == TYPE_INT))
			  abort ();
			arg = a.arg[dp->width_arg_index].a.a_int;
			if (arg < 0)
			  {
			    /* "A negative field width is taken as a '-' flag
			        followed by a positive field width."  */
			    flags |= FLAG_LEFT;
			    width = (unsigned int) (-arg);
			  }
			else
			  width = arg;
		      }
		    else
		      {
			const FCHAR_T *digitp = dp->width_start;

			do
			  width = xsum (xtimes (width, 10), *digitp++ - '0');
			while (digitp != dp->width_end);
		      }
		    has_width = 1;
		  }

		has_precision = 0;
		precision = 0;
		if (dp->precision_start != dp->precision_end)
		  {
		    if (dp->precision_arg_index != ARG_NONE)
		      {
			int arg;

			if (!(a.arg[dp->precision_arg_index].type == TYPE_INT))
			  abort ();
			arg = a.arg[dp->precision_arg_index].a.a_int;
			/* "A negative precision is taken as if the precision
			    were omitted."  */
			if (arg >= 0)
			  {
			    precision = arg;
			    has_precision = 1;
			  }
		      }
		    else
		      {
			const FCHAR_T *digitp = dp->precision_start + 1;

			precision = 0;
			while (digitp != dp->precision_end)
			  precision = xsum (xtimes (precision, 10), *digitp++ - '0');
			has_precision = 1;
		      }
		  }

		/* Allocate a temporary buffer of sufficient size.  */
		if (type == TYPE_LONGDOUBLE)
		  tmp_length =
		    (unsigned int) ((LDBL_DIG + 1)
				    * 0.831 /* decimal -> hexadecimal */
				   )
		    + 1; /* turn floor into ceil */
		else
		  tmp_length =
		    (unsigned int) ((DBL_DIG + 1)
				    * 0.831 /* decimal -> hexadecimal */
				   )
		    + 1; /* turn floor into ceil */
		if (tmp_length < precision)
		  tmp_length = precision;
		/* Account for sign, decimal point etc. */
		tmp_length = xsum (tmp_length, 12);

		if (tmp_length < width)
		  tmp_length = width;

		tmp_length = xsum (tmp_length, 1); /* account for trailing NUL */

		if (tmp_length <= sizeof (tmpbuf) / sizeof (DCHAR_T))
		  tmp = tmpbuf;
		else
		  {
		    size_t tmp_memsize = xtimes (tmp_length, sizeof (DCHAR_T));

		    if (size_overflow_p (tmp_memsize))
		      /* Overflow, would lead to out of memory.  */
		      goto out_of_memory;
		    tmp = (DCHAR_T *) malloc (tmp_memsize);
		    if (tmp == NULL)
		      /* Out of memory.  */
		      goto out_of_memory;
		  }

		pad_ptr = NULL;
		p = tmp;
		if (type == TYPE_LONGDOUBLE)
		  {
# if NEED_PRINTF_DIRECTIVE_A || NEED_PRINTF_LONG_DOUBLE
		    long double arg = a.arg[dp->arg_index].a.a_longdouble;

		    if (isnanl (arg))
		      {
			if (dp->conversion == 'A')
			  {
			    *p++ = 'N'; *p++ = 'A'; *p++ = 'N';
			  }
			else
			  {
			    *p++ = 'n'; *p++ = 'a'; *p++ = 'n';
			  }
		      }
		    else
		      {
			int sign = 0;
			DECL_LONG_DOUBLE_ROUNDING

			BEGIN_LONG_DOUBLE_ROUNDING ();

			if (signbit (arg)) /* arg < 0.0L or negative zero */
			  {
			    sign = -1;
			    arg = -arg;
			  }

			if (sign < 0)
			  *p++ = '-';
			else if (flags & FLAG_SHOWSIGN)
			  *p++ = '+';
			else if (flags & FLAG_SPACE)
			  *p++ = ' ';

			if (arg > 0.0L && arg + arg == arg)
			  {
			    if (dp->conversion == 'A')
			      {
				*p++ = 'I'; *p++ = 'N'; *p++ = 'F';
			      }
			    else
			      {
				*p++ = 'i'; *p++ = 'n'; *p++ = 'f';
			      }
			  }
			else
			  {
			    int exponent;
			    long double mantissa;

			    if (arg > 0.0L)
			      mantissa = printf_frexpl (arg, &exponent);
			    else
			      {
				exponent = 0;
				mantissa = 0.0L;
			      }

			    if (has_precision
				&& precision < (unsigned int) ((LDBL_DIG + 1) * 0.831) + 1)
			      {
				/* Round the mantissa.  */
				long double tail = mantissa;
				size_t q;

				for (q = precision; ; q--)
				  {
				    int digit = (int) tail;
				    tail -= digit;
				    if (q == 0)
				      {
					if (digit & 1 ? tail >= 0.5L : tail > 0.5L)
					  tail = 1 - tail;
					else
					  tail = - tail;
					break;
				      }
				    tail *= 16.0L;
				  }
				if (tail != 0.0L)
				  for (q = precision; q > 0; q--)
				    tail *= 0.0625L;
				mantissa += tail;
			      }

			    *p++ = '0';
			    *p++ = dp->conversion - 'A' + 'X';
			    pad_ptr = p;
			    {
			      int digit;

			      digit = (int) mantissa;
			      mantissa -= digit;
			      *p++ = '0' + digit;
			      if ((flags & FLAG_ALT)
				  || mantissa > 0.0L || precision > 0)
				{
				  *p++ = decimal_point_char ();
				  /* This loop terminates because we assume
				     that FLT_RADIX is a power of 2.  */
				  while (mantissa > 0.0L)
				    {
				      mantissa *= 16.0L;
				      digit = (int) mantissa;
				      mantissa -= digit;
				      *p++ = digit
					     + (digit < 10
						? '0'
						: dp->conversion - 10);
				      if (precision > 0)
					precision--;
				    }
				  while (precision > 0)
				    {
				      *p++ = '0';
				      precision--;
				    }
				}
			      }
			      *p++ = dp->conversion - 'A' + 'P';
#  if WIDE_CHAR_VERSION
			      {
				static const wchar_t decimal_format[] =
				  { '%', '+', 'd', '\0' };
				SNPRINTF (p, 6 + 1, decimal_format, exponent);
			      }
			      while (*p != '\0')
				p++;
#  else
			      if (sizeof (DCHAR_T) == 1)
				{
				  sprintf ((char *) p, "%+d", exponent);
				  while (*p != '\0')
				    p++;
				}
			      else
				{
				  char expbuf[6 + 1];
				  const char *ep;
				  sprintf (expbuf, "%+d", exponent);
				  for (ep = expbuf; (*p = *ep) != '\0'; ep++)
				    p++;
				}
#  endif
			  }

			END_LONG_DOUBLE_ROUNDING ();
		      }
# else
		    abort ();
# endif
		  }
		else
		  {
# if NEED_PRINTF_DIRECTIVE_A || NEED_PRINTF_DOUBLE
		    double arg = a.arg[dp->arg_index].a.a_double;

		    if (isnan (arg))
		      {
			if (dp->conversion == 'A')
			  {
			    *p++ = 'N'; *p++ = 'A'; *p++ = 'N';
			  }
			else
			  {
			    *p++ = 'n'; *p++ = 'a'; *p++ = 'n';
			  }
		      }
		    else
		      {
			int sign = 0;

			if (signbit (arg)) /* arg < 0.0 or negative zero */
			  {
			    sign = -1;
			    arg = -arg;
			  }

			if (sign < 0)
			  *p++ = '-';
			else if (flags & FLAG_SHOWSIGN)
			  *p++ = '+';
			else if (flags & FLAG_SPACE)
			  *p++ = ' ';

			if (arg > 0.0 && arg + arg == arg)
			  {
			    if (dp->conversion == 'A')
			      {
				*p++ = 'I'; *p++ = 'N'; *p++ = 'F';
			      }
			    else
			      {
				*p++ = 'i'; *p++ = 'n'; *p++ = 'f';
			      }
			  }
			else
			  {
			    int exponent;
			    double mantissa;

			    if (arg > 0.0)
			      mantissa = printf_frexp (arg, &exponent);
			    else
			      {
				exponent = 0;
				mantissa = 0.0;
			      }

			    if (has_precision
				&& precision < (unsigned int) ((DBL_DIG + 1) * 0.831) + 1)
			      {
				/* Round the mantissa.  */
				double tail = mantissa;
				size_t q;

				for (q = precision; ; q--)
				  {
				    int digit = (int) tail;
				    tail -= digit;
				    if (q == 0)
				      {
					if (digit & 1 ? tail >= 0.5 : tail > 0.5)
					  tail = 1 - tail;
					else
					  tail = - tail;
					break;
				      }
				    tail *= 16.0;
				  }
				if (tail != 0.0)
				  for (q = precision; q > 0; q--)
				    tail *= 0.0625;
				mantissa += tail;
			      }

			    *p++ = '0';
			    *p++ = dp->conversion - 'A' + 'X';
			    pad_ptr = p;
			    {
			      int digit;

			      digit = (int) mantissa;
			      mantissa -= digit;
			      *p++ = '0' + digit;
			      if ((flags & FLAG_ALT)
				  || mantissa > 0.0 || precision > 0)
				{
				  *p++ = decimal_point_char ();
				  /* This loop terminates because we assume
				     that FLT_RADIX is a power of 2.  */
				  while (mantissa > 0.0)
				    {
				      mantissa *= 16.0;
				      digit = (int) mantissa;
				      mantissa -= digit;
				      *p++ = digit
					     + (digit < 10
						? '0'
						: dp->conversion - 10);
				      if (precision > 0)
					precision--;
				    }
				  while (precision > 0)
				    {
				      *p++ = '0';
				      precision--;
				    }
				}
			      }
			      *p++ = dp->conversion - 'A' + 'P';
#  if WIDE_CHAR_VERSION
			      {
				static const wchar_t decimal_format[] =
				  { '%', '+', 'd', '\0' };
				SNPRINTF (p, 6 + 1, decimal_format, exponent);
			      }
			      while (*p != '\0')
				p++;
#  else
			      if (sizeof (DCHAR_T) == 1)
				{
				  sprintf ((char *) p, "%+d", exponent);
				  while (*p != '\0')
				    p++;
				}
			      else
				{
				  char expbuf[6 + 1];
				  const char *ep;
				  sprintf (expbuf, "%+d", exponent);
				  for (ep = expbuf; (*p = *ep) != '\0'; ep++)
				    p++;
				}
#  endif
			  }
		      }
# else
		    abort ();
# endif
		  }
		/* The generated string now extends from tmp to p, with the
		   zero padding insertion point being at pad_ptr.  */
		if (has_width && p - tmp < width)
		  {
		    size_t pad = width - (p - tmp);
		    DCHAR_T *end = p + pad;

		    if (flags & FLAG_LEFT)
		      {
			/* Pad with spaces on the right.  */
			for (; pad > 0; pad--)
			  *p++ = ' ';
		      }
		    else if ((flags & FLAG_ZERO) && pad_ptr != NULL)
		      {
			/* Pad with zeroes.  */
			DCHAR_T *q = end;

			while (p > pad_ptr)
			  *--q = *--p;
			for (; pad > 0; pad--)
			  *p++ = '0';
		      }
		    else
		      {
			/* Pad with spaces on the left.  */
			DCHAR_T *q = end;

			while (p > tmp)
			  *--q = *--p;
			for (; pad > 0; pad--)
			  *p++ = ' ';
		      }

		    p = end;
		  }

		{
		  size_t count = p - tmp;

		  if (count >= tmp_length)
		    /* tmp_length was incorrectly calculated - fix the
		       code above!  */
		    abort ();

		  /* Make room for the result.  */
		  if (count >= allocated - length)
		    {
		      size_t n = xsum (length, count);

		      ENSURE_ALLOCATION (n);
		    }

		  /* Append the result.  */
		  memcpy (result + length, tmp, count * sizeof (DCHAR_T));
		  if (tmp != tmpbuf)
		    free (tmp);
		  length += count;
		}
	      }
#endif
#if (NEED_PRINTF_INFINITE_DOUBLE || NEED_PRINTF_DOUBLE || NEED_PRINTF_INFINITE_LONG_DOUBLE || NEED_PRINTF_LONG_DOUBLE) && !defined IN_LIBINTL
	    else if ((dp->conversion == 'f' || dp->conversion == 'F'
		      || dp->conversion == 'e' || dp->conversion == 'E'
		      || dp->conversion == 'g' || dp->conversion == 'G'
		      || dp->conversion == 'a' || dp->conversion == 'A')
		     && (0
# if NEED_PRINTF_DOUBLE
			 || a.arg[dp->arg_index].type == TYPE_DOUBLE
# elif NEED_PRINTF_INFINITE_DOUBLE
			 || (a.arg[dp->arg_index].type == TYPE_DOUBLE
			     /* The systems (mingw) which produce wrong output
				for Inf, -Inf, and NaN also do so for -0.0.
				Therefore we treat this case here as well.  */
			     && is_infinite_or_zero (a.arg[dp->arg_index].a.a_double))
# endif
# if NEED_PRINTF_LONG_DOUBLE
			 || a.arg[dp->arg_index].type == TYPE_LONGDOUBLE
# elif NEED_PRINTF_INFINITE_LONG_DOUBLE
			 || (a.arg[dp->arg_index].type == TYPE_LONGDOUBLE
			     /* Some systems produce wrong output for Inf,
				-Inf, and NaN.  */
			     && is_infinitel (a.arg[dp->arg_index].a.a_longdouble))
# endif
			))
	      {
# if (NEED_PRINTF_DOUBLE || NEED_PRINTF_INFINITE_DOUBLE) && (NEED_PRINTF_LONG_DOUBLE || NEED_PRINTF_INFINITE_LONG_DOUBLE)
		arg_type type = a.arg[dp->arg_index].type;
# endif
		int flags = dp->flags;
		int has_width;
		size_t width;
		int has_precision;
		size_t precision;
		size_t tmp_length;
		DCHAR_T tmpbuf[700];
		DCHAR_T *tmp;
		DCHAR_T *pad_ptr;
		DCHAR_T *p;

		has_width = 0;
		width = 0;
		if (dp->width_start != dp->width_end)
		  {
		    if (dp->width_arg_index != ARG_NONE)
		      {
			int arg;

			if (!(a.arg[dp->width_arg_index].type == TYPE_INT))
			  abort ();
			arg = a.arg[dp->width_arg_index].a.a_int;
			if (arg < 0)
			  {
			    /* "A negative field width is taken as a '-' flag
			        followed by a positive field width."  */
			    flags |= FLAG_LEFT;
			    width = (unsigned int) (-arg);
			  }
			else
			  width = arg;
		      }
		    else
		      {
			const FCHAR_T *digitp = dp->width_start;

			do
			  width = xsum (xtimes (width, 10), *digitp++ - '0');
			while (digitp != dp->width_end);
		      }
		    has_width = 1;
		  }

		has_precision = 0;
		precision = 0;
		if (dp->precision_start != dp->precision_end)
		  {
		    if (dp->precision_arg_index != ARG_NONE)
		      {
			int arg;

			if (!(a.arg[dp->precision_arg_index].type == TYPE_INT))
			  abort ();
			arg = a.arg[dp->precision_arg_index].a.a_int;
			/* "A negative precision is taken as if the precision
			    were omitted."  */
			if (arg >= 0)
			  {
			    precision = arg;
			    has_precision = 1;
			  }
		      }
		    else
		      {
			const FCHAR_T *digitp = dp->precision_start + 1;

			precision = 0;
			while (digitp != dp->precision_end)
			  precision = xsum (xtimes (precision, 10), *digitp++ - '0');
			has_precision = 1;
		      }
		  }

		/* POSIX specifies the default precision to be 6 for %f, %F,
		   %e, %E, but not for %g, %G.  Implementations appear to use
		   the same default precision also for %g, %G.  */
		if (!has_precision)
		  precision = 6;

		/* Allocate a temporary buffer of sufficient size.  */
# if NEED_PRINTF_DOUBLE && NEED_PRINTF_LONG_DOUBLE
		tmp_length = (type == TYPE_LONGDOUBLE ? LDBL_DIG + 1 : DBL_DIG + 1);
# elif NEED_PRINTF_INFINITE_DOUBLE && NEED_PRINTF_LONG_DOUBLE
		tmp_length = (type == TYPE_LONGDOUBLE ? LDBL_DIG + 1 : 0);
# elif NEED_PRINTF_LONG_DOUBLE
		tmp_length = LDBL_DIG + 1;
# elif NEED_PRINTF_DOUBLE
		tmp_length = DBL_DIG + 1;
# else
		tmp_length = 0;
# endif
		if (tmp_length < precision)
		  tmp_length = precision;
# if NEED_PRINTF_LONG_DOUBLE
#  if NEED_PRINTF_DOUBLE || NEED_PRINTF_INFINITE_DOUBLE
		if (type == TYPE_LONGDOUBLE)
#  endif
		  if (dp->conversion == 'f' || dp->conversion == 'F')
		    {
		      long double arg = a.arg[dp->arg_index].a.a_longdouble;
		      if (!(isnanl (arg) || arg + arg == arg))
			{
			  /* arg is finite and nonzero.  */
			  int exponent = floorlog10l (arg < 0 ? -arg : arg);
			  if (exponent >= 0 && tmp_length < exponent + precision)
			    tmp_length = exponent + precision;
			}
		    }
# endif
# if NEED_PRINTF_DOUBLE
#  if NEED_PRINTF_LONG_DOUBLE || NEED_PRINTF_INFINITE_LONG_DOUBLE
		if (type == TYPE_DOUBLE)
#  endif
		  if (dp->conversion == 'f' || dp->conversion == 'F')
		    {
		      double arg = a.arg[dp->arg_index].a.a_double;
		      if (!(isnan (arg) || arg + arg == arg))
			{
			  /* arg is finite and nonzero.  */
			  int exponent = floorlog10 (arg < 0 ? -arg : arg);
			  if (exponent >= 0 && tmp_length < exponent + precision)
			    tmp_length = exponent + precision;
			}
		    }
# endif
		/* Account for sign, decimal point etc. */
		tmp_length = xsum (tmp_length, 12);

		if (tmp_length < width)
		  tmp_length = width;

		tmp_length = xsum (tmp_length, 1); /* account for trailing NUL */

		if (tmp_length <= sizeof (tmpbuf) / sizeof (DCHAR_T))
		  tmp = tmpbuf;
		else
		  {
		    size_t tmp_memsize = xtimes (tmp_length, sizeof (DCHAR_T));

		    if (size_overflow_p (tmp_memsize))
		      /* Overflow, would lead to out of memory.  */
		      goto out_of_memory;
		    tmp = (DCHAR_T *) malloc (tmp_memsize);
		    if (tmp == NULL)
		      /* Out of memory.  */
		      goto out_of_memory;
		  }

		pad_ptr = NULL;
		p = tmp;

# if NEED_PRINTF_LONG_DOUBLE || NEED_PRINTF_INFINITE_LONG_DOUBLE
#  if NEED_PRINTF_DOUBLE || NEED_PRINTF_INFINITE_DOUBLE
		if (type == TYPE_LONGDOUBLE)
#  endif
		  {
		    long double arg = a.arg[dp->arg_index].a.a_longdouble;

		    if (isnanl (arg))
		      {
			if (dp->conversion >= 'A' && dp->conversion <= 'Z')
			  {
			    *p++ = 'N'; *p++ = 'A'; *p++ = 'N';
			  }
			else
			  {
			    *p++ = 'n'; *p++ = 'a'; *p++ = 'n';
			  }
		      }
		    else
		      {
			int sign = 0;
			DECL_LONG_DOUBLE_ROUNDING

			BEGIN_LONG_DOUBLE_ROUNDING ();

			if (signbit (arg)) /* arg < 0.0L or negative zero */
			  {
			    sign = -1;
			    arg = -arg;
			  }

			if (sign < 0)
			  *p++ = '-';
			else if (flags & FLAG_SHOWSIGN)
			  *p++ = '+';
			else if (flags & FLAG_SPACE)
			  *p++ = ' ';

			if (arg > 0.0L && arg + arg == arg)
			  {
			    if (dp->conversion >= 'A' && dp->conversion <= 'Z')
			      {
				*p++ = 'I'; *p++ = 'N'; *p++ = 'F';
			      }
			    else
			      {
				*p++ = 'i'; *p++ = 'n'; *p++ = 'f';
			      }
			  }
			else
			  {
#  if NEED_PRINTF_LONG_DOUBLE
			    pad_ptr = p;

			    if (dp->conversion == 'f' || dp->conversion == 'F')
			      {
				char *digits;
				size_t ndigits;

				digits =
				  scale10_round_decimal_long_double (arg, precision);
				if (digits == NULL)
				  {
				    END_LONG_DOUBLE_ROUNDING ();
				    goto out_of_memory;
				  }
				ndigits = strlen (digits);

				if (ndigits > precision)
				  do
				    {
				      --ndigits;
				      *p++ = digits[ndigits];
				    }
				  while (ndigits > precision);
				else
				  *p++ = '0';
				/* Here ndigits <= precision.  */
				if ((flags & FLAG_ALT) || precision > 0)
				  {
				    *p++ = decimal_point_char ();
				    for (; precision > ndigits; precision--)
				      *p++ = '0';
				    while (ndigits > 0)
				      {
					--ndigits;
					*p++ = digits[ndigits];
				      }
				  }

				free (digits);
			      }
			    else if (dp->conversion == 'e' || dp->conversion == 'E')
			      {
				int exponent;

				if (arg == 0.0L)
				  {
				    exponent = 0;
				    *p++ = '0';
				    if ((flags & FLAG_ALT) || precision > 0)
				      {
					*p++ = decimal_point_char ();
					for (; precision > 0; precision--)
					  *p++ = '0';
				      }
				  }
				else
				  {
				    /* arg > 0.0L.  */
				    int adjusted;
				    char *digits;
				    size_t ndigits;

				    exponent = floorlog10l (arg);
				    adjusted = 0;
				    for (;;)
				      {
					digits =
					  scale10_round_decimal_long_double (arg,
									     (int)precision - exponent);
					if (digits == NULL)
					  {
					    END_LONG_DOUBLE_ROUNDING ();
					    goto out_of_memory;
					  }
					ndigits = strlen (digits);

					if (ndigits == precision + 1)
					  break;
					if (ndigits < precision
					    || ndigits > precision + 2)
					  /* The exponent was not guessed
					     precisely enough.  */
					  abort ();
					if (adjusted)
					  /* None of two values of exponent is
					     the right one.  Prevent an endless
					     loop.  */
					  abort ();
					free (digits);
					if (ndigits == precision)
					  exponent -= 1;
					else
					  exponent += 1;
					adjusted = 1;
				      }

				    /* Here ndigits = precision+1.  */
				    *p++ = digits[--ndigits];
				    if ((flags & FLAG_ALT) || precision > 0)
				      {
					*p++ = decimal_point_char ();
					while (ndigits > 0)
					  {
					    --ndigits;
					    *p++ = digits[ndigits];
					  }
				      }

				    free (digits);
				  }

				*p++ = dp->conversion; /* 'e' or 'E' */
#   if WIDE_CHAR_VERSION
				{
				  static const wchar_t decimal_format[] =
				    { '%', '+', '.', '2', 'd', '\0' };
				  SNPRINTF (p, 6 + 1, decimal_format, exponent);
				}
				while (*p != '\0')
				  p++;
#   else
				if (sizeof (DCHAR_T) == 1)
				  {
				    sprintf ((char *) p, "%+.2d", exponent);
				    while (*p != '\0')
				      p++;
				  }
				else
				  {
				    char expbuf[6 + 1];
				    const char *ep;
				    sprintf (expbuf, "%+.2d", exponent);
				    for (ep = expbuf; (*p = *ep) != '\0'; ep++)
				      p++;
				  }
#   endif
			      }
			    else if (dp->conversion == 'g' || dp->conversion == 'G')
			      {
				if (precision == 0)
				  precision = 1;
				/* precision >= 1.  */

				if (arg == 0.0L)
				  /* The exponent is 0, >= -4, < precision.
				     Use fixed-point notation.  */
				  {
				    size_t ndigits = precision;
				    /* Number of trailing zeroes that have to be
				       dropped.  */
				    size_t nzeroes =
				      (flags & FLAG_ALT ? 0 : precision - 1);

				    --ndigits;
				    *p++ = '0';
				    if ((flags & FLAG_ALT) || ndigits > nzeroes)
				      {
					*p++ = decimal_point_char ();
					while (ndigits > nzeroes)
					  {
					    --ndigits;
					    *p++ = '0';
					  }
				      }
				  }
				else
				  {
				    /* arg > 0.0L.  */
				    int exponent;
				    int adjusted;
				    char *digits;
				    size_t ndigits;
				    size_t nzeroes;

				    exponent = floorlog10l (arg);
				    adjusted = 0;
				    for (;;)
				      {
					digits =
					  scale10_round_decimal_long_double (arg,
									     (int)(precision - 1) - exponent);
					if (digits == NULL)
					  {
					    END_LONG_DOUBLE_ROUNDING ();
					    goto out_of_memory;
					  }
					ndigits = strlen (digits);

					if (ndigits == precision)
					  break;
					if (ndigits < precision - 1
					    || ndigits > precision + 1)
					  /* The exponent was not guessed
					     precisely enough.  */
					  abort ();
					if (adjusted)
					  /* None of two values of exponent is
					     the right one.  Prevent an endless
					     loop.  */
					  abort ();
					free (digits);
					if (ndigits < precision)
					  exponent -= 1;
					else
					  exponent += 1;
					adjusted = 1;
				      }
				    /* Here ndigits = precision.  */

				    /* Determine the number of trailing zeroes
				       that have to be dropped.  */
				    nzeroes = 0;
				    if ((flags & FLAG_ALT) == 0)
				      while (nzeroes < ndigits
					     && digits[nzeroes] == '0')
					nzeroes++;

				    /* The exponent is now determined.  */
				    if (exponent >= -4
					&& exponent < (long)precision)
				      {
					/* Fixed-point notation:
					   max(exponent,0)+1 digits, then the
					   decimal point, then the remaining
					   digits without trailing zeroes.  */
					if (exponent >= 0)
					  {
					    size_t count = exponent + 1;
					    /* Note: count <= precision = ndigits.  */
					    for (; count > 0; count--)
					      *p++ = digits[--ndigits];
					    if ((flags & FLAG_ALT) || ndigits > nzeroes)
					      {
						*p++ = decimal_point_char ();
						while (ndigits > nzeroes)
						  {
						    --ndigits;
						    *p++ = digits[ndigits];
						  }
					      }
					  }
					else
					  {
					    size_t count = -exponent - 1;
					    *p++ = '0';
					    *p++ = decimal_point_char ();
					    for (; count > 0; count--)
					      *p++ = '0';
					    while (ndigits > nzeroes)
					      {
						--ndigits;
						*p++ = digits[ndigits];
					      }
					  }
				      }
				    else
				      {
					/* Exponential notation.  */
					*p++ = digits[--ndigits];
					if ((flags & FLAG_ALT) || ndigits > nzeroes)
					  {
					    *p++ = decimal_point_char ();
					    while (ndigits > nzeroes)
					      {
						--ndigits;
						*p++ = digits[ndigits];
					      }
					  }
					*p++ = dp->conversion - 'G' + 'E'; /* 'e' or 'E' */
#   if WIDE_CHAR_VERSION
					{
					  static const wchar_t decimal_format[] =
					    { '%', '+', '.', '2', 'd', '\0' };
					  SNPRINTF (p, 6 + 1, decimal_format, exponent);
					}
					while (*p != '\0')
					  p++;
#   else
					if (sizeof (DCHAR_T) == 1)
					  {
					    sprintf ((char *) p, "%+.2d", exponent);
					    while (*p != '\0')
					      p++;
					  }
					else
					  {
					    char expbuf[6 + 1];
					    const char *ep;
					    sprintf (expbuf, "%+.2d", exponent);
					    for (ep = expbuf; (*p = *ep) != '\0'; ep++)
					      p++;
					  }
#   endif
				      }

				    free (digits);
				  }
			      }
			    else
			      abort ();
#  else
			    /* arg is finite.  */
			    abort ();
#  endif
			  }

			END_LONG_DOUBLE_ROUNDING ();
		      }
		  }
#  if NEED_PRINTF_DOUBLE || NEED_PRINTF_INFINITE_DOUBLE
		else
#  endif
# endif
# if NEED_PRINTF_DOUBLE || NEED_PRINTF_INFINITE_DOUBLE
		  {
		    double arg = a.arg[dp->arg_index].a.a_double;

		    if (isnan (arg))
		      {
			if (dp->conversion >= 'A' && dp->conversion <= 'Z')
			  {
			    *p++ = 'N'; *p++ = 'A'; *p++ = 'N';
			  }
			else
			  {
			    *p++ = 'n'; *p++ = 'a'; *p++ = 'n';
			  }
		      }
		    else
		      {
			int sign = 0;

			if (signbit (arg)) /* arg < 0.0 or negative zero */
			  {
			    sign = -1;
			    arg = -arg;
			  }

			if (sign < 0)
			  *p++ = '-';
			else if (flags & FLAG_SHOWSIGN)
			  *p++ = '+';
			else if (flags & FLAG_SPACE)
			  *p++ = ' ';

			if (arg > 0.0 && arg + arg == arg)
			  {
			    if (dp->conversion >= 'A' && dp->conversion <= 'Z')
			      {
				*p++ = 'I'; *p++ = 'N'; *p++ = 'F';
			      }
			    else
			      {
				*p++ = 'i'; *p++ = 'n'; *p++ = 'f';
			      }
			  }
			else
			  {
#  if NEED_PRINTF_DOUBLE
			    pad_ptr = p;

			    if (dp->conversion == 'f' || dp->conversion == 'F')
			      {
				char *digits;
				size_t ndigits;

				digits =
				  scale10_round_decimal_double (arg, precision);
				if (digits == NULL)
				  goto out_of_memory;
				ndigits = strlen (digits);

				if (ndigits > precision)
				  do
				    {
				      --ndigits;
				      *p++ = digits[ndigits];
				    }
				  while (ndigits > precision);
				else
				  *p++ = '0';
				/* Here ndigits <= precision.  */
				if ((flags & FLAG_ALT) || precision > 0)
				  {
				    *p++ = decimal_point_char ();
				    for (; precision > ndigits; precision--)
				      *p++ = '0';
				    while (ndigits > 0)
				      {
					--ndigits;
					*p++ = digits[ndigits];
				      }
				  }

				free (digits);
			      }
			    else if (dp->conversion == 'e' || dp->conversion == 'E')
			      {
				int exponent;

				if (arg == 0.0)
				  {
				    exponent = 0;
				    *p++ = '0';
				    if ((flags & FLAG_ALT) || precision > 0)
				      {
					*p++ = decimal_point_char ();
					for (; precision > 0; precision--)
					  *p++ = '0';
				      }
				  }
				else
				  {
				    /* arg > 0.0.  */
				    int adjusted;
				    char *digits;
				    size_t ndigits;

				    exponent = floorlog10 (arg);
				    adjusted = 0;
				    for (;;)
				      {
					digits =
					  scale10_round_decimal_double (arg,
									(int)precision - exponent);
					if (digits == NULL)
					  goto out_of_memory;
					ndigits = strlen (digits);

					if (ndigits == precision + 1)
					  break;
					if (ndigits < precision
					    || ndigits > precision + 2)
					  /* The exponent was not guessed
					     precisely enough.  */
					  abort ();
					if (adjusted)
					  /* None of two values of exponent is
					     the right one.  Prevent an endless
					     loop.  */
					  abort ();
					free (digits);
					if (ndigits == precision)
					  exponent -= 1;
					else
					  exponent += 1;
					adjusted = 1;
				      }

				    /* Here ndigits = precision+1.  */
				    *p++ = digits[--ndigits];
				    if ((flags & FLAG_ALT) || precision > 0)
				      {
					*p++ = decimal_point_char ();
					while (ndigits > 0)
					  {
					    --ndigits;
					    *p++ = digits[ndigits];
					  }
				      }

				    free (digits);
				  }

				*p++ = dp->conversion; /* 'e' or 'E' */
#   if WIDE_CHAR_VERSION
				{
				  static const wchar_t decimal_format[] =
				    /* Produce the same number of exponent digits
				       as the native printf implementation.  */
#    if (defined _WIN32 || defined __WIN32__) && ! defined __CYGWIN__
				    { '%', '+', '.', '3', 'd', '\0' };
#    else
				    { '%', '+', '.', '2', 'd', '\0' };
#    endif
				  SNPRINTF (p, 6 + 1, decimal_format, exponent);
				}
				while (*p != '\0')
				  p++;
#   else
				{
				  static const char decimal_format[] =
				    /* Produce the same number of exponent digits
				       as the native printf implementation.  */
#    if (defined _WIN32 || defined __WIN32__) && ! defined __CYGWIN__
				    "%+.3d";
#    else
				    "%+.2d";
#    endif
				  if (sizeof (DCHAR_T) == 1)
				    {
				      sprintf ((char *) p, decimal_format, exponent);
				      while (*p != '\0')
					p++;
				    }
				  else
				    {
				      char expbuf[6 + 1];
				      const char *ep;
				      sprintf (expbuf, decimal_format, exponent);
				      for (ep = expbuf; (*p = *ep) != '\0'; ep++)
					p++;
				    }
				}
#   endif
			      }
			    else if (dp->conversion == 'g' || dp->conversion == 'G')
			      {
				if (precision == 0)
				  precision = 1;
				/* precision >= 1.  */

				if (arg == 0.0)
				  /* The exponent is 0, >= -4, < precision.
				     Use fixed-point notation.  */
				  {
				    size_t ndigits = precision;
				    /* Number of trailing zeroes that have to be
				       dropped.  */
				    size_t nzeroes =
				      (flags & FLAG_ALT ? 0 : precision - 1);

				    --ndigits;
				    *p++ = '0';
				    if ((flags & FLAG_ALT) || ndigits > nzeroes)
				      {
					*p++ = decimal_point_char ();
					while (ndigits > nzeroes)
					  {
					    --ndigits;
					    *p++ = '0';
					  }
				      }
				  }
				else
				  {
				    /* arg > 0.0.  */
				    int exponent;
				    int adjusted;
				    char *digits;
				    size_t ndigits;
				    size_t nzeroes;

				    exponent = floorlog10 (arg);
				    adjusted = 0;
				    for (;;)
				      {
					digits =
					  scale10_round_decimal_double (arg,
									(int)(precision - 1) - exponent);
					if (digits == NULL)
					  goto out_of_memory;
					ndigits = strlen (digits);

					if (ndigits == precision)
					  break;
					if (ndigits < precision - 1
					    || ndigits > precision + 1)
					  /* The exponent was not guessed
					     precisely enough.  */
					  abort ();
					if (adjusted)
					  /* None of two values of exponent is
					     the right one.  Prevent an endless
					     loop.  */
					  abort ();
					free (digits);
					if (ndigits < precision)
					  exponent -= 1;
					else
					  exponent += 1;
					adjusted = 1;
				      }
				    /* Here ndigits = precision.  */

				    /* Determine the number of trailing zeroes
				       that have to be dropped.  */
				    nzeroes = 0;
				    if ((flags & FLAG_ALT) == 0)
				      while (nzeroes < ndigits
					     && digits[nzeroes] == '0')
					nzeroes++;

				    /* The exponent is now determined.  */
				    if (exponent >= -4
					&& exponent < (long)precision)
				      {
					/* Fixed-point notation:
					   max(exponent,0)+1 digits, then the
					   decimal point, then the remaining
					   digits without trailing zeroes.  */
					if (exponent >= 0)
					  {
					    size_t count = exponent + 1;
					    /* Note: count <= precision = ndigits.  */
					    for (; count > 0; count--)
					      *p++ = digits[--ndigits];
					    if ((flags & FLAG_ALT) || ndigits > nzeroes)
					      {
						*p++ = decimal_point_char ();
						while (ndigits > nzeroes)
						  {
						    --ndigits;
						    *p++ = digits[ndigits];
						  }
					      }
					  }
					else
					  {
					    size_t count = -exponent - 1;
					    *p++ = '0';
					    *p++ = decimal_point_char ();
					    for (; count > 0; count--)
					      *p++ = '0';
					    while (ndigits > nzeroes)
					      {
						--ndigits;
						*p++ = digits[ndigits];
					      }
					  }
				      }
				    else
				      {
					/* Exponential notation.  */
					*p++ = digits[--ndigits];
					if ((flags & FLAG_ALT) || ndigits > nzeroes)
					  {
					    *p++ = decimal_point_char ();
					    while (ndigits > nzeroes)
					      {
						--ndigits;
						*p++ = digits[ndigits];
					      }
					  }
					*p++ = dp->conversion - 'G' + 'E'; /* 'e' or 'E' */
#   if WIDE_CHAR_VERSION
					{
					  static const wchar_t decimal_format[] =
					    /* Produce the same number of exponent digits
					       as the native printf implementation.  */
#    if (defined _WIN32 || defined __WIN32__) && ! defined __CYGWIN__
					    { '%', '+', '.', '3', 'd', '\0' };
#    else
					    { '%', '+', '.', '2', 'd', '\0' };
#    endif
					  SNPRINTF (p, 6 + 1, decimal_format, exponent);
					}
					while (*p != '\0')
					  p++;
#   else
					{
					  static const char decimal_format[] =
					    /* Produce the same number of exponent digits
					       as the native printf implementation.  */
#    if (defined _WIN32 || defined __WIN32__) && ! defined __CYGWIN__
					    "%+.3d";
#    else
					    "%+.2d";
#    endif
					  if (sizeof (DCHAR_T) == 1)
					    {
					      sprintf ((char *) p, decimal_format, exponent);
					      while (*p != '\0')
						p++;
					    }
					  else
					    {
					      char expbuf[6 + 1];
					      const char *ep;
					      sprintf (expbuf, decimal_format, exponent);
					      for (ep = expbuf; (*p = *ep) != '\0'; ep++)
						p++;
					    }
					}
#   endif
				      }

				    free (digits);
				  }
			      }
			    else
			      abort ();
#  else
			    /* arg is finite.  */
			    if (!(arg == 0.0))
			      abort ();

			    pad_ptr = p;

			    if (dp->conversion == 'f' || dp->conversion == 'F')
			      {
				*p++ = '0';
				if ((flags & FLAG_ALT) || precision > 0)
				  {
				    *p++ = decimal_point_char ();
				    for (; precision > 0; precision--)
				      *p++ = '0';
				  }
			      }
			    else if (dp->conversion == 'e' || dp->conversion == 'E')
			      {
				*p++ = '0';
				if ((flags & FLAG_ALT) || precision > 0)
				  {
				    *p++ = decimal_point_char ();
				    for (; precision > 0; precision--)
				      *p++ = '0';
				  }
				*p++ = dp->conversion; /* 'e' or 'E' */
				*p++ = '+';
				/* Produce the same number of exponent digits as
				   the native printf implementation.  */
#   if (defined _WIN32 || defined __WIN32__) && ! defined __CYGWIN__
				*p++ = '0';
#   endif
				*p++ = '0';
				*p++ = '0';
			      }
			    else if (dp->conversion == 'g' || dp->conversion == 'G')
			      {
				*p++ = '0';
				if (flags & FLAG_ALT)
				  {
				    size_t ndigits =
				      (precision > 0 ? precision - 1 : 0);
				    *p++ = decimal_point_char ();
				    for (; ndigits > 0; --ndigits)
				      *p++ = '0';
				  }
			      }
			    else
			      abort ();
#  endif
			  }
		      }
		  }
# endif

		/* The generated string now extends from tmp to p, with the
		   zero padding insertion point being at pad_ptr.  */
		if (has_width && p - tmp < width)
		  {
		    size_t pad = width - (p - tmp);
		    DCHAR_T *end = p + pad;

		    if (flags & FLAG_LEFT)
		      {
			/* Pad with spaces on the right.  */
			for (; pad > 0; pad--)
			  *p++ = ' ';
		      }
		    else if ((flags & FLAG_ZERO) && pad_ptr != NULL)
		      {
			/* Pad with zeroes.  */
			DCHAR_T *q = end;

			while (p > pad_ptr)
			  *--q = *--p;
			for (; pad > 0; pad--)
			  *p++ = '0';
		      }
		    else
		      {
			/* Pad with spaces on the left.  */
			DCHAR_T *q = end;

			while (p > tmp)
			  *--q = *--p;
			for (; pad > 0; pad--)
			  *p++ = ' ';
		      }

		    p = end;
		  }

		{
		  size_t count = p - tmp;

		  if (count >= tmp_length)
		    /* tmp_length was incorrectly calculated - fix the
		       code above!  */
		    abort ();

		  /* Make room for the result.  */
		  if (count >= allocated - length)
		    {
		      size_t n = xsum (length, count);

		      ENSURE_ALLOCATION (n);
		    }

		  /* Append the result.  */
		  memcpy (result + length, tmp, count * sizeof (DCHAR_T));
		  if (tmp != tmpbuf)
		    free (tmp);
		  length += count;
		}
	      }
#endif
	    else
	      {
		arg_type type = a.arg[dp->arg_index].type;
		int flags = dp->flags;
#if !USE_SNPRINTF || !DCHAR_IS_TCHAR || ENABLE_UNISTDIO || NEED_PRINTF_FLAG_ZERO || NEED_PRINTF_UNBOUNDED_PRECISION
		int has_width;
		size_t width;
#endif
#if !USE_SNPRINTF || NEED_PRINTF_UNBOUNDED_PRECISION
		int has_precision;
		size_t precision;
#endif
#if NEED_PRINTF_UNBOUNDED_PRECISION
		int prec_ourselves;
#else
#		define prec_ourselves 0
#endif
#if !DCHAR_IS_TCHAR || ENABLE_UNISTDIO || NEED_PRINTF_FLAG_ZERO || NEED_PRINTF_UNBOUNDED_PRECISION
		int pad_ourselves;
#else
#		define pad_ourselves 0
#endif
		TCHAR_T *fbp;
		unsigned int prefix_count;
		int prefixes[2];
#if !USE_SNPRINTF
		size_t tmp_length;
		TCHAR_T tmpbuf[700];
		TCHAR_T *tmp;
#endif

#if !USE_SNPRINTF || !DCHAR_IS_TCHAR || ENABLE_UNISTDIO || NEED_PRINTF_FLAG_ZERO || NEED_PRINTF_UNBOUNDED_PRECISION
		has_width = 0;
		width = 0;
		if (dp->width_start != dp->width_end)
		  {
		    if (dp->width_arg_index != ARG_NONE)
		      {
			int arg;

			if (!(a.arg[dp->width_arg_index].type == TYPE_INT))
			  abort ();
			arg = a.arg[dp->width_arg_index].a.a_int;
			if (arg < 0)
			  {
			    /* "A negative field width is taken as a '-' flag
			        followed by a positive field width."  */
			    flags |= FLAG_LEFT;
			    width = (unsigned int) (-arg);
			  }
			else
			  width = arg;
		      }
		    else
		      {
			const FCHAR_T *digitp = dp->width_start;

			do
			  width = xsum (xtimes (width, 10), *digitp++ - '0');
			while (digitp != dp->width_end);
		      }
		    has_width = 1;
		  }
#endif

#if !USE_SNPRINTF || NEED_PRINTF_UNBOUNDED_PRECISION
		has_precision = 0;
		precision = 6;
		if (dp->precision_start != dp->precision_end)
		  {
		    if (dp->precision_arg_index != ARG_NONE)
		      {
			int arg;

			if (!(a.arg[dp->precision_arg_index].type == TYPE_INT))
			  abort ();
			arg = a.arg[dp->precision_arg_index].a.a_int;
			/* "A negative precision is taken as if the precision
			    were omitted."  */
			if (arg >= 0)
			  {
			    precision = arg;
			    has_precision = 1;
			  }
		      }
		    else
		      {
			const FCHAR_T *digitp = dp->precision_start + 1;

			precision = 0;
			while (digitp != dp->precision_end)
			  precision = xsum (xtimes (precision, 10), *digitp++ - '0');
			has_precision = 1;
		      }
		  }
#endif

#if !USE_SNPRINTF
		/* Allocate a temporary buffer of sufficient size for calling
		   sprintf.  */
		{
		  switch (dp->conversion)
		    {

		    case 'd': case 'i': case 'u':
# if HAVE_LONG_LONG_INT
		      if (type == TYPE_LONGLONGINT || type == TYPE_ULONGLONGINT)
			tmp_length =
			  (unsigned int) (sizeof (unsigned long long) * CHAR_BIT
					  * 0.30103 /* binary -> decimal */
					 )
			  + 1; /* turn floor into ceil */
		      else
# endif
		      if (type == TYPE_LONGINT || type == TYPE_ULONGINT)
			tmp_length =
			  (unsigned int) (sizeof (unsigned long) * CHAR_BIT
					  * 0.30103 /* binary -> decimal */
					 )
			  + 1; /* turn floor into ceil */
		      else
			tmp_length =
			  (unsigned int) (sizeof (unsigned int) * CHAR_BIT
					  * 0.30103 /* binary -> decimal */
					 )
			  + 1; /* turn floor into ceil */
		      if (tmp_length < precision)
			tmp_length = precision;
		      /* Multiply by 2, as an estimate for FLAG_GROUP.  */
		      tmp_length = xsum (tmp_length, tmp_length);
		      /* Add 1, to account for a leading sign.  */
		      tmp_length = xsum (tmp_length, 1);
		      break;

		    case 'o':
# if HAVE_LONG_LONG_INT
		      if (type == TYPE_LONGLONGINT || type == TYPE_ULONGLONGINT)
			tmp_length =
			  (unsigned int) (sizeof (unsigned long long) * CHAR_BIT
					  * 0.333334 /* binary -> octal */
					 )
			  + 1; /* turn floor into ceil */
		      else
# endif
		      if (type == TYPE_LONGINT || type == TYPE_ULONGINT)
			tmp_length =
			  (unsigned int) (sizeof (unsigned long) * CHAR_BIT
					  * 0.333334 /* binary -> octal */
					 )
			  + 1; /* turn floor into ceil */
		      else
			tmp_length =
			  (unsigned int) (sizeof (unsigned int) * CHAR_BIT
					  * 0.333334 /* binary -> octal */
					 )
			  + 1; /* turn floor into ceil */
		      if (tmp_length < precision)
			tmp_length = precision;
		      /* Add 1, to account for a leading sign.  */
		      tmp_length = xsum (tmp_length, 1);
		      break;

		    case 'x': case 'X':
# if HAVE_LONG_LONG_INT
		      if (type == TYPE_LONGLONGINT || type == TYPE_ULONGLONGINT)
			tmp_length =
			  (unsigned int) (sizeof (unsigned long long) * CHAR_BIT
					  * 0.25 /* binary -> hexadecimal */
					 )
			  + 1; /* turn floor into ceil */
		      else
# endif
		      if (type == TYPE_LONGINT || type == TYPE_ULONGINT)
			tmp_length =
			  (unsigned int) (sizeof (unsigned long) * CHAR_BIT
					  * 0.25 /* binary -> hexadecimal */
					 )
			  + 1; /* turn floor into ceil */
		      else
			tmp_length =
			  (unsigned int) (sizeof (unsigned int) * CHAR_BIT
					  * 0.25 /* binary -> hexadecimal */
					 )
			  + 1; /* turn floor into ceil */
		      if (tmp_length < precision)
			tmp_length = precision;
		      /* Add 2, to account for a leading sign or alternate form.  */
		      tmp_length = xsum (tmp_length, 2);
		      break;

		    case 'f': case 'F':
		      if (type == TYPE_LONGDOUBLE)
			tmp_length =
			  (unsigned int) (LDBL_MAX_EXP
					  * 0.30103 /* binary -> decimal */
					  * 2 /* estimate for FLAG_GROUP */
					 )
			  + 1 /* turn floor into ceil */
			  + 10; /* sign, decimal point etc. */
		      else
			tmp_length =
			  (unsigned int) (DBL_MAX_EXP
					  * 0.30103 /* binary -> decimal */
					  * 2 /* estimate for FLAG_GROUP */
					 )
			  + 1 /* turn floor into ceil */
			  + 10; /* sign, decimal point etc. */
		      tmp_length = xsum (tmp_length, precision);
		      break;

		    case 'e': case 'E': case 'g': case 'G':
		      tmp_length =
			12; /* sign, decimal point, exponent etc. */
		      tmp_length = xsum (tmp_length, precision);
		      break;

		    case 'a': case 'A':
		      if (type == TYPE_LONGDOUBLE)
			tmp_length =
			  (unsigned int) (LDBL_DIG
					  * 0.831 /* decimal -> hexadecimal */
					 )
			  + 1; /* turn floor into ceil */
		      else
			tmp_length =
			  (unsigned int) (DBL_DIG
					  * 0.831 /* decimal -> hexadecimal */
					 )
			  + 1; /* turn floor into ceil */
		      if (tmp_length < precision)
			tmp_length = precision;
		      /* Account for sign, decimal point etc. */
		      tmp_length = xsum (tmp_length, 12);
		      break;

		    case 'c':
# if HAVE_WINT_T && !WIDE_CHAR_VERSION
		      if (type == TYPE_WIDE_CHAR)
			tmp_length = MB_CUR_MAX;
		      else
# endif
			tmp_length = 1;
		      break;

		    case 's':
# if HAVE_WCHAR_T
		      if (type == TYPE_WIDE_STRING)
			{
			  tmp_length =
			    local_wcslen (a.arg[dp->arg_index].a.a_wide_string);

#  if !WIDE_CHAR_VERSION
			  tmp_length = xtimes (tmp_length, MB_CUR_MAX);
#  endif
			}
		      else
# endif
			tmp_length = strlen (a.arg[dp->arg_index].a.a_string);
		      break;

		    case 'p':
		      tmp_length =
			(unsigned int) (sizeof (void *) * CHAR_BIT
					* 0.25 /* binary -> hexadecimal */
				       )
			  + 1 /* turn floor into ceil */
			  + 2; /* account for leading 0x */
		      break;

		    default:
		      abort ();
		    }

# if ENABLE_UNISTDIO
		  /* Padding considers the number of characters, therefore the
		     number of elements after padding may be
		       > max (tmp_length, width)
		     but is certainly
		       <= tmp_length + width.  */
		  tmp_length = xsum (tmp_length, width);
# else
		  /* Padding considers the number of elements, says POSIX.  */
		  if (tmp_length < width)
		    tmp_length = width;
# endif

		  tmp_length = xsum (tmp_length, 1); /* account for trailing NUL */
		}

		if (tmp_length <= sizeof (tmpbuf) / sizeof (TCHAR_T))
		  tmp = tmpbuf;
		else
		  {
		    size_t tmp_memsize = xtimes (tmp_length, sizeof (TCHAR_T));

		    if (size_overflow_p (tmp_memsize))
		      /* Overflow, would lead to out of memory.  */
		      goto out_of_memory;
		    tmp = (TCHAR_T *) malloc (tmp_memsize);
		    if (tmp == NULL)
		      /* Out of memory.  */
		      goto out_of_memory;
		  }
#endif

		/* Decide whether to handle the precision ourselves.  */
#if NEED_PRINTF_UNBOUNDED_PRECISION
		switch (dp->conversion)
		  {
		  case 'd': case 'i': case 'u':
		  case 'o':
		  case 'x': case 'X': case 'p':
		    prec_ourselves = has_precision && (precision > 0);
		    break;
		  default:
		    prec_ourselves = 0;
		    break;
		  }
#endif

		/* Decide whether to perform the padding ourselves.  */
#if !DCHAR_IS_TCHAR || ENABLE_UNISTDIO || NEED_PRINTF_FLAG_ZERO || NEED_PRINTF_UNBOUNDED_PRECISION
		switch (dp->conversion)
		  {
# if !DCHAR_IS_TCHAR || ENABLE_UNISTDIO
		  /* If we need conversion from TCHAR_T[] to DCHAR_T[], we need
		     to perform the padding after this conversion.  Functions
		     with unistdio extensions perform the padding based on
		     character count rather than element count.  */
		  case 'c': case 's':
# endif
# if NEED_PRINTF_FLAG_ZERO
		  case 'f': case 'F': case 'e': case 'E': case 'g': case 'G':
		  case 'a': case 'A':
# endif
		    pad_ourselves = 1;
		    break;
		  default:
		    pad_ourselves = prec_ourselves;
		    break;
		  }
#endif

		/* Construct the format string for calling snprintf or
		   sprintf.  */
		fbp = buf;
		*fbp++ = '%';
#if NEED_PRINTF_FLAG_GROUPING
		/* The underlying implementation doesn't support the ' flag.
		   Produce no grouping characters in this case; this is
		   acceptable because the grouping is locale dependent.  */
#else
		if (flags & FLAG_GROUP)
		  *fbp++ = '\'';
#endif
		if (flags & FLAG_LEFT)
		  *fbp++ = '-';
		if (flags & FLAG_SHOWSIGN)
		  *fbp++ = '+';
		if (flags & FLAG_SPACE)
		  *fbp++ = ' ';
		if (flags & FLAG_ALT)
		  *fbp++ = '#';
		if (!pad_ourselves)
		  {
		    if (flags & FLAG_ZERO)
		      *fbp++ = '0';
		    if (dp->width_start != dp->width_end)
		      {
			size_t n = dp->width_end - dp->width_start;
			/* The width specification is known to consist only
			   of standard ASCII characters.  */
			if (sizeof (FCHAR_T) == sizeof (TCHAR_T))
			  {
			    memcpy (fbp, dp->width_start, n * sizeof (TCHAR_T));
			    fbp += n;
			  }
			else
			  {
			    const FCHAR_T *mp = dp->width_start;
			    do
			      *fbp++ = (unsigned char) *mp++;
			    while (--n > 0);
			  }
		      }
		  }
		if (!prec_ourselves)
		  {
		    if (dp->precision_start != dp->precision_end)
		      {
			size_t n = dp->precision_end - dp->precision_start;
			/* The precision specification is known to consist only
			   of standard ASCII characters.  */
			if (sizeof (FCHAR_T) == sizeof (TCHAR_T))
			  {
			    memcpy (fbp, dp->precision_start, n * sizeof (TCHAR_T));
			    fbp += n;
			  }
			else
			  {
			    const FCHAR_T *mp = dp->precision_start;
			    do
			      *fbp++ = (unsigned char) *mp++;
			    while (--n > 0);
			  }
		      }
		  }

		switch (type)
		  {
#if HAVE_LONG_LONG_INT
		  case TYPE_LONGLONGINT:
		  case TYPE_ULONGLONGINT:
# if (defined _WIN32 || defined __WIN32__) && ! defined __CYGWIN__
		    *fbp++ = 'I';
		    *fbp++ = '6';
		    *fbp++ = '4';
		    break;
# else
		    *fbp++ = 'l';
		    /*FALLTHROUGH*/
# endif
#endif
		  case TYPE_LONGINT:
		  case TYPE_ULONGINT:
#if HAVE_WINT_T
		  case TYPE_WIDE_CHAR:
#endif
#if HAVE_WCHAR_T
		  case TYPE_WIDE_STRING:
#endif
		    *fbp++ = 'l';
		    break;
		  case TYPE_LONGDOUBLE:
		    *fbp++ = 'L';
		    break;
		  default:
		    break;
		  }
#if NEED_PRINTF_DIRECTIVE_F
		if (dp->conversion == 'F')
		  *fbp = 'f';
		else
#endif
		  *fbp = dp->conversion;
#if USE_SNPRINTF
# if !(__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 3))
		fbp[1] = '%';
		fbp[2] = 'n';
		fbp[3] = '\0';
# else
		/* On glibc2 systems from glibc >= 2.3 - probably also older
		   ones - we know that snprintf's returns value conforms to
		   ISO C 99: the gl_SNPRINTF_DIRECTIVE_N test passes.
		   Therefore we can avoid using %n in this situation.
		   On glibc2 systems from 2004-10-18 or newer, the use of %n
		   in format strings in writable memory may crash the program
		   (if compiled with _FORTIFY_SOURCE=2), so we should avoid it
		   in this situation.  */
		fbp[1] = '\0';
# endif
#else
		fbp[1] = '\0';
#endif

		/* Construct the arguments for calling snprintf or sprintf.  */
		prefix_count = 0;
		if (!pad_ourselves && dp->width_arg_index != ARG_NONE)
		  {
		    if (!(a.arg[dp->width_arg_index].type == TYPE_INT))
		      abort ();
		    prefixes[prefix_count++] = a.arg[dp->width_arg_index].a.a_int;
		  }
		if (dp->precision_arg_index != ARG_NONE)
		  {
		    if (!(a.arg[dp->precision_arg_index].type == TYPE_INT))
		      abort ();
		    prefixes[prefix_count++] = a.arg[dp->precision_arg_index].a.a_int;
		  }

#if USE_SNPRINTF
		/* The SNPRINTF result is appended after result[0..length].
		   The latter is an array of DCHAR_T; SNPRINTF appends an
		   array of TCHAR_T to it.  This is possible because
		   sizeof (TCHAR_T) divides sizeof (DCHAR_T) and
		   alignof (TCHAR_T) <= alignof (DCHAR_T).  */
# define TCHARS_PER_DCHAR (sizeof (DCHAR_T) / sizeof (TCHAR_T))
		/* Prepare checking whether snprintf returns the count
		   via %n.  */
		ENSURE_ALLOCATION (xsum (length, 1));
		*(TCHAR_T *) (result + length) = '\0';
#endif

		for (;;)
		  {
		    int count = -1;

#if USE_SNPRINTF
		    int retcount = 0;
		    size_t maxlen = allocated - length;
		    /* SNPRINTF can fail if its second argument is
		       > INT_MAX.  */
		    if (maxlen > INT_MAX / TCHARS_PER_DCHAR)
		      maxlen = INT_MAX / TCHARS_PER_DCHAR;
		    maxlen = maxlen * TCHARS_PER_DCHAR;
# define SNPRINTF_BUF(arg) \
		    switch (prefix_count)				    \
		      {							    \
		      case 0:						    \
			retcount = SNPRINTF ((TCHAR_T *) (result + length), \
					     maxlen, buf,		    \
					     arg, &count);		    \
			break;						    \
		      case 1:						    \
			retcount = SNPRINTF ((TCHAR_T *) (result + length), \
					     maxlen, buf,		    \
					     prefixes[0], arg, &count);	    \
			break;						    \
		      case 2:						    \
			retcount = SNPRINTF ((TCHAR_T *) (result + length), \
					     maxlen, buf,		    \
					     prefixes[0], prefixes[1], arg, \
					     &count);			    \
			break;						    \
		      default:						    \
			abort ();					    \
		      }
#else
# define SNPRINTF_BUF(arg) \
		    switch (prefix_count)				    \
		      {							    \
		      case 0:						    \
			count = sprintf (tmp, buf, arg);		    \
			break;						    \
		      case 1:						    \
			count = sprintf (tmp, buf, prefixes[0], arg);	    \
			break;						    \
		      case 2:						    \
			count = sprintf (tmp, buf, prefixes[0], prefixes[1],\
					 arg);				    \
			break;						    \
		      default:						    \
			abort ();					    \
		      }
#endif

		    switch (type)
		      {
		      case TYPE_SCHAR:
			{
			  int arg = a.arg[dp->arg_index].a.a_schar;
			  SNPRINTF_BUF (arg);
			}
			break;
		      case TYPE_UCHAR:
			{
			  unsigned int arg = a.arg[dp->arg_index].a.a_uchar;
			  SNPRINTF_BUF (arg);
			}
			break;
		      case TYPE_SHORT:
			{
			  int arg = a.arg[dp->arg_index].a.a_short;
			  SNPRINTF_BUF (arg);
			}
			break;
		      case TYPE_USHORT:
			{
			  unsigned int arg = a.arg[dp->arg_index].a.a_ushort;
			  SNPRINTF_BUF (arg);
			}
			break;
		      case TYPE_INT:
			{
			  int arg = a.arg[dp->arg_index].a.a_int;
			  SNPRINTF_BUF (arg);
			}
			break;
		      case TYPE_UINT:
			{
			  unsigned int arg = a.arg[dp->arg_index].a.a_uint;
			  SNPRINTF_BUF (arg);
			}
			break;
		      case TYPE_LONGINT:
			{
			  long int arg = a.arg[dp->arg_index].a.a_longint;
			  SNPRINTF_BUF (arg);
			}
			break;
		      case TYPE_ULONGINT:
			{
			  unsigned long int arg = a.arg[dp->arg_index].a.a_ulongint;
			  SNPRINTF_BUF (arg);
			}
			break;
#if HAVE_LONG_LONG_INT
		      case TYPE_LONGLONGINT:
			{
			  long long int arg = a.arg[dp->arg_index].a.a_longlongint;
			  SNPRINTF_BUF (arg);
			}
			break;
		      case TYPE_ULONGLONGINT:
			{
			  unsigned long long int arg = a.arg[dp->arg_index].a.a_ulonglongint;
			  SNPRINTF_BUF (arg);
			}
			break;
#endif
		      case TYPE_DOUBLE:
			{
			  double arg = a.arg[dp->arg_index].a.a_double;
			  SNPRINTF_BUF (arg);
			}
			break;
		      case TYPE_LONGDOUBLE:
			{
			  long double arg = a.arg[dp->arg_index].a.a_longdouble;
			  SNPRINTF_BUF (arg);
			}
			break;
		      case TYPE_CHAR:
			{
			  int arg = a.arg[dp->arg_index].a.a_char;
			  SNPRINTF_BUF (arg);
			}
			break;
#if HAVE_WINT_T
		      case TYPE_WIDE_CHAR:
			{
			  wint_t arg = a.arg[dp->arg_index].a.a_wide_char;
			  SNPRINTF_BUF (arg);
			}
			break;
#endif
		      case TYPE_STRING:
			{
			  const char *arg = a.arg[dp->arg_index].a.a_string;
			  SNPRINTF_BUF (arg);
			}
			break;
#if HAVE_WCHAR_T
		      case TYPE_WIDE_STRING:
			{
			  const wchar_t *arg = a.arg[dp->arg_index].a.a_wide_string;
			  SNPRINTF_BUF (arg);
			}
			break;
#endif
		      case TYPE_POINTER:
			{
			  void *arg = a.arg[dp->arg_index].a.a_pointer;
			  SNPRINTF_BUF (arg);
			}
			break;
		      default:
			abort ();
		      }

#if USE_SNPRINTF
		    /* Portability: Not all implementations of snprintf()
		       are ISO C 99 compliant.  Determine the number of
		       bytes that snprintf() has produced or would have
		       produced.  */
		    if (count >= 0)
		      {
			/* Verify that snprintf() has NUL-terminated its
			   result.  */
			if (count < maxlen
			    && ((TCHAR_T *) (result + length)) [count] != '\0')
			  abort ();
			/* Portability hack.  */
			if (retcount > count)
			  count = retcount;
		      }
		    else
		      {
			/* snprintf() doesn't understand the '%n'
			   directive.  */
			if (fbp[1] != '\0')
			  {
			    /* Don't use the '%n' directive; instead, look
			       at the snprintf() return value.  */
			    fbp[1] = '\0';
			    continue;
			  }
			else
			  {
			    /* Look at the snprintf() return value.  */
			    if (retcount < 0)
			      {
				/* HP-UX 10.20 snprintf() is doubly deficient:
				   It doesn't understand the '%n' directive,
				   *and* it returns -1 (rather than the length
				   that would have been required) when the
				   buffer is too small.  */
				size_t bigger_need =
				  xsum (xtimes (allocated, 2), 12);
				ENSURE_ALLOCATION (bigger_need);
				continue;
			      }
			    else
			      count = retcount;
			  }
		      }
#endif

		    /* Attempt to handle failure.  */
		    if (count < 0)
		      {
			if (!(result == resultbuf || result == NULL))
			  free (result);
			if (buf_malloced != NULL)
			  free (buf_malloced);
			CLEANUP ();
			errno = EINVAL;
			return NULL;
		      }

#if USE_SNPRINTF
		    /* Handle overflow of the allocated buffer.
		       If such an overflow occurs, a C99 compliant snprintf()
		       returns a count >= maxlen.  However, a non-compliant
		       snprintf() function returns only count = maxlen - 1.  To
		       cover both cases, test whether count >= maxlen - 1.  */
		    if ((unsigned int) count + 1 >= maxlen)
		      {
			/* If maxlen already has attained its allowed maximum,
			   allocating more memory will not increase maxlen.
			   Instead of looping, bail out.  */
			if (maxlen == INT_MAX / TCHARS_PER_DCHAR)
			  goto overflow;
			else
			  {
			    /* Need at least count * sizeof (TCHAR_T) bytes.
			       But allocate proportionally, to avoid looping
			       eternally if snprintf() reports a too small
			       count.  */
			    size_t n =
			      xmax (xsum (length,
					  (count + TCHARS_PER_DCHAR - 1)
					  / TCHARS_PER_DCHAR),
				    xtimes (allocated, 2));

			    ENSURE_ALLOCATION (n);
			    continue;
			  }
		      }
#endif

#if NEED_PRINTF_UNBOUNDED_PRECISION
		    if (prec_ourselves)
		      {
			/* Handle the precision.  */
			TCHAR_T *prec_ptr = 
# if USE_SNPRINTF
			  (TCHAR_T *) (result + length);
# else
			  tmp;
# endif
			size_t prefix_count;
			size_t move;

			prefix_count = 0;
			/* Put the additional zeroes after the sign.  */
			if (count >= 1
			    && (*prec_ptr == '-' || *prec_ptr == '+'
				|| *prec_ptr == ' '))
			  prefix_count = 1;
			/* Put the additional zeroes after the 0x prefix if
			   (flags & FLAG_ALT) || (dp->conversion == 'p').  */
			else if (count >= 2
				 && prec_ptr[0] == '0'
				 && (prec_ptr[1] == 'x' || prec_ptr[1] == 'X'))
			  prefix_count = 2;

			move = count - prefix_count;
			if (precision > move)
			  {
			    /* Insert zeroes.  */
			    size_t insert = precision - move;
			    TCHAR_T *prec_end;

# if USE_SNPRINTF
			    size_t n =
			      xsum (length,
				    (count + insert + TCHARS_PER_DCHAR - 1)
				    / TCHARS_PER_DCHAR);
			    length += (count + TCHARS_PER_DCHAR - 1) / TCHARS_PER_DCHAR;
			    ENSURE_ALLOCATION (n);
			    length -= (count + TCHARS_PER_DCHAR - 1) / TCHARS_PER_DCHAR;
			    prec_ptr = (TCHAR_T *) (result + length);
# endif

			    prec_end = prec_ptr + count;
			    prec_ptr += prefix_count;

			    while (prec_end > prec_ptr)
			      {
				prec_end--;
				prec_end[insert] = prec_end[0];
			      }

			    prec_end += insert;
			    do
			      *--prec_end = '0';
			    while (prec_end > prec_ptr);

			    count += insert;
			  }
		      }
#endif

#if !DCHAR_IS_TCHAR
# if !USE_SNPRINTF
		    if (count >= tmp_length)
		      /* tmp_length was incorrectly calculated - fix the
			 code above!  */
		      abort ();
# endif

		    /* Convert from TCHAR_T[] to DCHAR_T[].  */
		    if (dp->conversion == 'c' || dp->conversion == 's')
		      {
			/* type = TYPE_CHAR or TYPE_WIDE_CHAR or TYPE_STRING
			   TYPE_WIDE_STRING.
			   The result string is not certainly ASCII.  */
			const TCHAR_T *tmpsrc;
			DCHAR_T *tmpdst;
			size_t tmpdst_len;
			/* This code assumes that TCHAR_T is 'char'.  */
			typedef int TCHAR_T_verify
				    [2 * (sizeof (TCHAR_T) == 1) - 1];
# if USE_SNPRINTF
			tmpsrc = (TCHAR_T *) (result + length);
# else
			tmpsrc = tmp;
# endif
			tmpdst = NULL;
			tmpdst_len = 0;
			if (DCHAR_CONV_FROM_ENCODING (locale_charset (),
						      iconveh_question_mark,
						      tmpsrc, count,
						      NULL,
						      &tmpdst, &tmpdst_len)
			    < 0)
			  {
			    int saved_errno = errno;
			    if (!(result == resultbuf || result == NULL))
			      free (result);
			    if (buf_malloced != NULL)
			      free (buf_malloced);
			    CLEANUP ();
			    errno = saved_errno;
			    return NULL;
			  }
			ENSURE_ALLOCATION (xsum (length, tmpdst_len));
			DCHAR_CPY (result + length, tmpdst, tmpdst_len);
			free (tmpdst);
			count = tmpdst_len;
		      }
		    else
		      {
			/* The result string is ASCII.
			   Simple 1:1 conversion.  */
# if USE_SNPRINTF
			/* If sizeof (DCHAR_T) == sizeof (TCHAR_T), it's a
			   no-op conversion, in-place on the array starting
			   at (result + length).  */
			if (sizeof (DCHAR_T) != sizeof (TCHAR_T))
# endif
			  {
			    const TCHAR_T *tmpsrc;
			    DCHAR_T *tmpdst;
			    size_t n;

# if USE_SNPRINTF
			    if (result == resultbuf)
			      {
				tmpsrc = (TCHAR_T *) (result + length);
				/* ENSURE_ALLOCATION will not move tmpsrc
				   (because it's part of resultbuf).  */
				ENSURE_ALLOCATION (xsum (length, count));
			      }
			    else
			      {
				/* ENSURE_ALLOCATION will move the array
				   (because it uses realloc().  */
				ENSURE_ALLOCATION (xsum (length, count));
				tmpsrc = (TCHAR_T *) (result + length);
			      }
# else
			    tmpsrc = tmp;
			    ENSURE_ALLOCATION (xsum (length, count));
# endif
			    tmpdst = result + length;
			    /* Copy backwards, because of overlapping.  */
			    tmpsrc += count;
			    tmpdst += count;
			    for (n = count; n > 0; n--)
			      *--tmpdst = (unsigned char) *--tmpsrc;
			  }
		      }
#endif

#if DCHAR_IS_TCHAR && !USE_SNPRINTF
		    /* Make room for the result.  */
		    if (count > allocated - length)
		      {
			/* Need at least count elements.  But allocate
			   proportionally.  */
			size_t n =
			  xmax (xsum (length, count), xtimes (allocated, 2));

			ENSURE_ALLOCATION (n);
		      }
#endif

		    /* Here count <= allocated - length.  */

		    /* Perform padding.  */
#if !DCHAR_IS_TCHAR || ENABLE_UNISTDIO || NEED_PRINTF_FLAG_ZERO || NEED_PRINTF_UNBOUNDED_PRECISION
		    if (pad_ourselves && has_width)
		      {
			size_t w;
# if ENABLE_UNISTDIO
			/* Outside POSIX, it's preferrable to compare the width
			   against the number of _characters_ of the converted
			   value.  */
			w = DCHAR_MBSNLEN (result + length, count);
# else
			/* The width is compared against the number of _bytes_
			   of the converted value, says POSIX.  */
			w = count;
# endif
			if (w < width)
			  {
			    size_t pad = width - w;
# if USE_SNPRINTF
			    /* Make room for the result.  */
			    if (xsum (count, pad) > allocated - length)
			      {
				/* Need at least count + pad elements.  But
				   allocate proportionally.  */
				size_t n =
				  xmax (xsum3 (length, count, pad),
					xtimes (allocated, 2));

				length += count;
				ENSURE_ALLOCATION (n);
				length -= count;
			      }
			    /* Here count + pad <= allocated - length.  */
# endif
			    {
# if !DCHAR_IS_TCHAR || USE_SNPRINTF
			      DCHAR_T * const rp = result + length;
# else
			      DCHAR_T * const rp = tmp;
# endif
			      DCHAR_T *p = rp + count;
			      DCHAR_T *end = p + pad;
# if NEED_PRINTF_FLAG_ZERO
			      DCHAR_T *pad_ptr;
#  if !DCHAR_IS_TCHAR
			      if (dp->conversion == 'c'
				  || dp->conversion == 's')
				/* No zero-padding for string directives.  */
				pad_ptr = NULL;
			      else
#  endif
				{
				  pad_ptr = (*rp == '-' ? rp + 1 : rp);
				  /* No zero-padding of "inf" and "nan".  */
				  if ((*pad_ptr >= 'A' && *pad_ptr <= 'Z')
				      || (*pad_ptr >= 'a' && *pad_ptr <= 'z'))
				    pad_ptr = NULL;
				}
# endif
			      /* The generated string now extends from rp to p,
				 with the zero padding insertion point being at
				 pad_ptr.  */

			      count = count + pad; /* = end - rp */

			      if (flags & FLAG_LEFT)
				{
				  /* Pad with spaces on the right.  */
				  for (; pad > 0; pad--)
				    *p++ = ' ';
				}
# if NEED_PRINTF_FLAG_ZERO
			      else if ((flags & FLAG_ZERO) && pad_ptr != NULL)
				{
				  /* Pad with zeroes.  */
				  DCHAR_T *q = end;

				  while (p > pad_ptr)
				    *--q = *--p;
				  for (; pad > 0; pad--)
				    *p++ = '0';
				}
# endif
			      else
				{
				  /* Pad with spaces on the left.  */
				  DCHAR_T *q = end;

				  while (p > rp)
				    *--q = *--p;
				  for (; pad > 0; pad--)
				    *p++ = ' ';
				}
			    }
			  }
		      }
#endif

#if DCHAR_IS_TCHAR && !USE_SNPRINTF
		    if (count >= tmp_length)
		      /* tmp_length was incorrectly calculated - fix the
			 code above!  */
		      abort ();
#endif

		    /* Here still count <= allocated - length.  */

#if !DCHAR_IS_TCHAR || USE_SNPRINTF
		    /* The snprintf() result did fit.  */
#else
		    /* Append the sprintf() result.  */
		    memcpy (result + length, tmp, count * sizeof (DCHAR_T));
#endif
#if !USE_SNPRINTF
		    if (tmp != tmpbuf)
		      free (tmp);
#endif

#if NEED_PRINTF_DIRECTIVE_F
		    if (dp->conversion == 'F')
		      {
			/* Convert the %f result to upper case for %F.  */
			DCHAR_T *rp = result + length;
			size_t rc;
			for (rc = count; rc > 0; rc--, rp++)
			  if (*rp >= 'a' && *rp <= 'z')
			    *rp = *rp - 'a' + 'A';
		      }
#endif

		    length += count;
		    break;
		  }
	      }
	  }
      }

    /* Add the final NUL.  */
    ENSURE_ALLOCATION (xsum (length, 1));
    result[length] = '\0';

    if (result != resultbuf && length + 1 < allocated)
      {
	/* Shrink the allocated memory if possible.  */
	DCHAR_T *memory;

	memory = (DCHAR_T *) realloc (result, (length + 1) * sizeof (DCHAR_T));
	if (memory != NULL)
	  result = memory;
      }

    if (buf_malloced != NULL)
      free (buf_malloced);
    CLEANUP ();
    *lengthp = length;
    /* Note that we can produce a big string of a length > INT_MAX.  POSIX
       says that snprintf() fails with errno = EOVERFLOW in this case, but
       that's only because snprintf() returns an 'int'.  This function does
       not have this limitation.  */
    return result;

  overflow:
    if (!(result == resultbuf || result == NULL))
      free (result);
    if (buf_malloced != NULL)
      free (buf_malloced);
    CLEANUP ();
    errno = EOVERFLOW;
    return NULL;

  out_of_memory:
    if (!(result == resultbuf || result == NULL))
      free (result);
    if (buf_malloced != NULL)
      free (buf_malloced);
  out_of_memory_1:
    CLEANUP ();
    errno = ENOMEM;
    return NULL;
  }
}

#undef TCHARS_PER_DCHAR
#undef SNPRINTF
#undef USE_SNPRINTF
#undef DCHAR_CPY
#undef PRINTF_PARSE
#undef DIRECTIVES
#undef DIRECTIVE
#undef DCHAR_IS_TCHAR
#undef TCHAR_T
#undef DCHAR_T
#undef FCHAR_T
#undef VASNPRINTF
