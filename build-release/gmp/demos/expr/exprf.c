/* mpf expression evaluation

Copyright 2000, 2001, 2002 Free Software Foundation, Inc.

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

#include <stdio.h>
#include <string.h>
#include "gmp.h"
#include "expr-impl.h"


/* Change this to "#define TRACE(x) x" to get some traces. */
#define TRACE(x)


static int
e_mpf_sgn (mpf_srcptr x)
{
  return mpf_sgn (x);
}


static const struct mpexpr_operator_t  _mpf_expr_standard_table[] = {

  { "**",  (mpexpr_fun_t) mpf_pow_ui,
    MPEXPR_TYPE_BINARY_UI | MPEXPR_TYPE_RIGHTASSOC,                   220 },

  { "!",   (mpexpr_fun_t) e_mpf_sgn,
    MPEXPR_TYPE_LOGICAL_NOT | MPEXPR_TYPE_PREFIX,                     210 },
  { "-",   (mpexpr_fun_t) mpf_neg,
    MPEXPR_TYPE_UNARY | MPEXPR_TYPE_PREFIX,                           210 },

  { "*",   (mpexpr_fun_t) mpf_mul,           MPEXPR_TYPE_BINARY,      200 },
  { "/",   (mpexpr_fun_t) mpf_div,           MPEXPR_TYPE_BINARY,      200 },

  { "+",   (mpexpr_fun_t) mpf_add,           MPEXPR_TYPE_BINARY,      190 },
  { "-",   (mpexpr_fun_t) mpf_sub,           MPEXPR_TYPE_BINARY,      190 },

  { "<<",  (mpexpr_fun_t) mpf_mul_2exp,      MPEXPR_TYPE_BINARY_UI,   180 },
  { ">>",  (mpexpr_fun_t) mpf_div_2exp,      MPEXPR_TYPE_BINARY_UI,   180 },

  { "<=",  (mpexpr_fun_t) mpf_cmp,           MPEXPR_TYPE_CMP_LE,      170 },
  { "<",   (mpexpr_fun_t) mpf_cmp,           MPEXPR_TYPE_CMP_LT,      170 },
  { ">=",  (mpexpr_fun_t) mpf_cmp,           MPEXPR_TYPE_CMP_GE,      170 },
  { ">",   (mpexpr_fun_t) mpf_cmp,           MPEXPR_TYPE_CMP_GT,      170 },

  { "==",  (mpexpr_fun_t) mpf_cmp,           MPEXPR_TYPE_CMP_EQ,      160 },
  { "!=",  (mpexpr_fun_t) mpf_cmp,           MPEXPR_TYPE_CMP_NE,      160 },

  { "&&",  (mpexpr_fun_t) e_mpf_sgn,         MPEXPR_TYPE_LOGICAL_AND, 120 },
  { "||",  (mpexpr_fun_t) e_mpf_sgn,         MPEXPR_TYPE_LOGICAL_OR,  110 },

  { ":",   NULL,                             MPEXPR_TYPE_COLON,       101 },
  { "?",   (mpexpr_fun_t) e_mpf_sgn,         MPEXPR_TYPE_QUESTION,    100 },

  { ")",   NULL,                             MPEXPR_TYPE_CLOSEPAREN,    4 },
  { "(",   NULL,                             MPEXPR_TYPE_OPENPAREN,     3 },
  { ",",   NULL,                             MPEXPR_TYPE_ARGSEP,        2 },
  { "$",   NULL,                             MPEXPR_TYPE_VARIABLE,      1 },

  { "abs",      (mpexpr_fun_t) mpf_abs,          MPEXPR_TYPE_UNARY        },
  { "ceil",     (mpexpr_fun_t) mpf_ceil,         MPEXPR_TYPE_UNARY        },
  { "cmp",      (mpexpr_fun_t) mpf_cmp,          MPEXPR_TYPE_I_BINARY     },
  { "eq",       (mpexpr_fun_t) mpf_eq,           MPEXPR_TYPE_I_TERNARY_UI },
  { "floor",    (mpexpr_fun_t) mpf_floor,        MPEXPR_TYPE_UNARY        },
  { "integer_p",(mpexpr_fun_t) mpf_integer_p,    MPEXPR_TYPE_I_UNARY      },
  { "max",   (mpexpr_fun_t) mpf_cmp, MPEXPR_TYPE_MAX | MPEXPR_TYPE_PAIRWISE },
  { "min",   (mpexpr_fun_t) mpf_cmp, MPEXPR_TYPE_MIN | MPEXPR_TYPE_PAIRWISE },
  { "reldiff",  (mpexpr_fun_t) mpf_reldiff,      MPEXPR_TYPE_BINARY       },
  { "sgn",      (mpexpr_fun_t) e_mpf_sgn,        MPEXPR_TYPE_I_UNARY      },
  { "sqrt",     (mpexpr_fun_t) mpf_sqrt,         MPEXPR_TYPE_UNARY        },
  { "trunc",    (mpexpr_fun_t) mpf_trunc,        MPEXPR_TYPE_UNARY        },

  { NULL }
};

const struct mpexpr_operator_t * const mpf_expr_standard_table
= _mpf_expr_standard_table;


int
#if HAVE_STDARG
mpf_expr (mpf_ptr res, int base, const char *e, ...)
#else
mpf_expr (va_alist)
     va_dcl
#endif
{
  mpf_srcptr  var[MPEXPR_VARIABLES];
  va_list     ap;
  int         ret;
#if HAVE_STDARG
  va_start (ap, e);
#else
  mpf_ptr     res;
  int         base;
  const char  *e;
  va_start (ap);
  res  = va_arg (ap, mpf_ptr);
  base = va_arg (ap, int);
  e    = va_arg (ap, const char *);
#endif

  TRACE (printf ("mpf_expr(): base %d, %s\n", base, e));
  ret = mpexpr_va_to_var ((void **) var, ap);
  va_end (ap);

  if (ret != MPEXPR_RESULT_OK)
    return ret;

  return mpf_expr_a (mpf_expr_standard_table, res, base,
		     mpf_get_prec (res), e, strlen(e), var);
}
