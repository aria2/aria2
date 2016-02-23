dnl  mpn_submul_1 - from x86_64/bobcat directory for fat binary.
dnl  Generated by configure - DO NOT EDIT.

define(OPERATION_submul_1)
define(__gmpn_submul_1, __gmpn_submul_1_bobcat)
define(__gmpn_submul_1c,__gmpn_submul_1c_bobcat)
define(__gmpn_preinv_submul_1,__gmpn_preinv_submul_1_bobcat)
define(__gmpn_submul_1_cps,__gmpn_submul_1_cps_bobcat)

dnl  For k6 and k7 gcd_1 calling their corresponding mpn_modexact_1_odd
ifdef(`__gmpn_modexact_1_odd',,
`define(__gmpn_modexact_1_odd,__gmpn_modexact_1_odd_bobcat)')

define(MUL_TOOM22_THRESHOLD,27)
define(MUL_TOOM33_THRESHOLD,32)
define(SQR_TOOM2_THRESHOLD,28)
define(SQR_TOOM3_THRESHOLD,93)
define(BMOD_1_TO_MOD_1_THRESHOLD,17)

include(../../gmp/mpn/x86_64/bobcat/aorsmul_1.asm)

