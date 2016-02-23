dnl  mpn_rshift - from x86_64/k10 directory for fat binary.
dnl  Generated by configure - DO NOT EDIT.

define(OPERATION_rshift)
define(__gmpn_rshift, __gmpn_rshift_k10)
define(__gmpn_rshiftc,__gmpn_rshiftc_k10)
define(__gmpn_preinv_rshift,__gmpn_preinv_rshift_k10)
define(__gmpn_rshift_cps,__gmpn_rshift_cps_k10)

dnl  For k6 and k7 gcd_1 calling their corresponding mpn_modexact_1_odd
ifdef(`__gmpn_modexact_1_odd',,
`define(__gmpn_modexact_1_odd,__gmpn_modexact_1_odd_k10)')

define(MUL_TOOM22_THRESHOLD,28)
define(MUL_TOOM33_THRESHOLD,81)
define(SQR_TOOM2_THRESHOLD,34)
define(SQR_TOOM3_THRESHOLD,113)
define(BMOD_1_TO_MOD_1_THRESHOLD,15)

include(../../gmp/mpn/x86_64/k10/rshift.asm)

