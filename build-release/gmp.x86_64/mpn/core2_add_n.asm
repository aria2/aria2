dnl  mpn_add_n - from x86_64/core2 directory for fat binary.
dnl  Generated by configure - DO NOT EDIT.

define(OPERATION_add_n)
define(__gmpn_add_n, __gmpn_add_n_core2)
define(__gmpn_add_nc,__gmpn_add_nc_core2)
define(__gmpn_preinv_add_n,__gmpn_preinv_add_n_core2)
define(__gmpn_add_n_cps,__gmpn_add_n_cps_core2)

dnl  For k6 and k7 gcd_1 calling their corresponding mpn_modexact_1_odd
ifdef(`__gmpn_modexact_1_odd',,
`define(__gmpn_modexact_1_odd,__gmpn_modexact_1_odd_core2)')

define(MUL_TOOM22_THRESHOLD,23)
define(MUL_TOOM33_THRESHOLD,65)
define(SQR_TOOM2_THRESHOLD,28)
define(SQR_TOOM3_THRESHOLD,101)
define(BMOD_1_TO_MOD_1_THRESHOLD,23)

include(../../gmp/mpn/x86_64/core2/aors_n.asm)

