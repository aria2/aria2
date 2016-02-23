/* Intel P55 gmp-mparam.h -- Compiler/machine parameter header file.

Copyright 1991, 1993, 1994, 1999, 2000, 2001, 2002, 2004, 2009, 2010 Free
Software Foundation, Inc.

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


#define GMP_LIMB_BITS 32
#define BYTES_PER_MP_LIMB 4


/* For mpn/x86/pentium/mod_1.asm */
#define COUNT_LEADING_ZEROS_NEED_CLZ_TAB


/* 233MHz P55 */

#define MOD_1_NORM_THRESHOLD                 5
#define MOD_1_UNNORM_THRESHOLD           MP_SIZE_T_MAX  /* never */
#define MOD_1N_TO_MOD_1_1_THRESHOLD      MP_SIZE_T_MAX  /* never */
#define MOD_1U_TO_MOD_1_1_THRESHOLD         12
#define MOD_1_1_TO_MOD_1_2_THRESHOLD         0
#define MOD_1_2_TO_MOD_1_4_THRESHOLD        11
#define PREINV_MOD_1_TO_MOD_1_THRESHOLD     63
#define USE_PREINV_DIVREM_1                  0
#define DIVEXACT_1_THRESHOLD                 0  /* always (native) */
#define BMOD_1_TO_MOD_1_THRESHOLD           51

#define MUL_TOOM22_THRESHOLD                16
#define MUL_TOOM33_THRESHOLD                53
#define MUL_TOOM44_THRESHOLD               128
#define MUL_TOOM6H_THRESHOLD               189
#define MUL_TOOM8H_THRESHOLD               260

#define MUL_TOOM32_TO_TOOM43_THRESHOLD      89
#define MUL_TOOM32_TO_TOOM53_THRESHOLD      91
#define MUL_TOOM42_TO_TOOM53_THRESHOLD      90
#define MUL_TOOM42_TO_TOOM63_THRESHOLD      88

#define SQR_BASECASE_THRESHOLD               0  /* always (native) */
#define SQR_TOOM2_THRESHOLD                 20
#define SQR_TOOM3_THRESHOLD                 73
#define SQR_TOOM4_THRESHOLD                178
#define SQR_TOOM6_THRESHOLD                210
#define SQR_TOOM8_THRESHOLD                375

#define MULMOD_BNM1_THRESHOLD               11
#define SQRMOD_BNM1_THRESHOLD               12

#define MUL_FFT_MODF_THRESHOLD             364  /* k = 5 */
#define MUL_FFT_TABLE3                                      \
  { {    364, 5}, {     15, 6}, {      8, 5}, {     17, 6}, \
    {      9, 5}, {     19, 6}, {     17, 7}, {      9, 6}, \
    {     21, 7}, {     11, 6}, {     23, 7}, {     15, 6}, \
    {     31, 7}, {     21, 8}, {     11, 7}, {     27, 8}, \
    {     15, 7}, {     33, 8}, {     19, 7}, {     39, 8}, \
    {     23, 7}, {     47, 8}, {     27, 9}, {     15, 8}, \
    {     31, 7}, {     63, 8}, {     39, 9}, {     23, 8}, \
    {     47,10}, {     15, 9}, {     31, 8}, {     67, 9}, \
    {     39, 8}, {     79, 9}, {     47, 8}, {     95, 9}, \
    {     55,10}, {     31, 9}, {     79,10}, {     47, 9}, \
    {     95,11}, {     31,10}, {     63, 9}, {    135,10}, \
    {     79, 9}, {    159, 8}, {    319, 9}, {    167,10}, \
    {     95, 9}, {    191, 8}, {    383,11}, {     63,10}, \
    {    127, 9}, {    255,10}, {    143, 9}, {    287,10}, \
    {    159, 9}, {    319,11}, {     95,10}, {    191, 9}, \
    {    383,12}, {     63,11}, {    127,10}, {    271, 9}, \
    {    543,10}, {    287,11}, {    159,10}, {    351,11}, \
    {    191,10}, {    415,11}, {    223,12}, {    127,11}, \
    {    255,10}, {    511,11}, {    287,10}, {    575,11}, \
    {    351,12}, {    191,11}, {    415,13}, {    127,12}, \
    {    255,11}, {    575,12}, {    319,11}, {    703,12}, \
    {    383,11}, {    831,12}, {    447,13}, {   8192,14}, \
    {  16384,15}, {  32768,16} }
#define MUL_FFT_TABLE3_SIZE 90
#define MUL_FFT_THRESHOLD                 3520

#define SQR_FFT_MODF_THRESHOLD             340  /* k = 5 */
#define SQR_FFT_TABLE3                                      \
  { {    340, 5}, {     17, 6}, {      9, 5}, {     19, 6}, \
    {     17, 7}, {      9, 6}, {     21, 7}, {     11, 6}, \
    {     23, 7}, {     15, 6}, {     31, 7}, {     21, 8}, \
    {     11, 7}, {     29, 8}, {     15, 7}, {     33, 8}, \
    {     19, 7}, {     39, 8}, {     27, 7}, {     55, 9}, \
    {     15, 8}, {     31, 7}, {     65, 8}, {     43, 9}, \
    {     23, 8}, {     47,10}, {     15, 9}, {     31, 8}, \
    {     67, 9}, {     39, 8}, {     83, 9}, {     47, 8}, \
    {     95,10}, {     31, 9}, {     63, 8}, {    127, 9}, \
    {     79,10}, {     47, 9}, {     95,11}, {     31,10}, \
    {     63, 9}, {    127, 8}, {    255, 9}, {    135,10}, \
    {     79, 9}, {    159, 8}, {    319,10}, {     95, 9}, \
    {    191,11}, {     63,10}, {    127, 9}, {    255, 8}, \
    {    511, 9}, {    271,10}, {    143, 9}, {    287, 8}, \
    {    575, 9}, {    303,10}, {    159, 9}, {    319,11}, \
    {     95,10}, {    191, 9}, {    383,10}, {    207,12}, \
    {     63,11}, {    127,10}, {    271, 9}, {    543,10}, \
    {    287, 9}, {    575,10}, {    303,11}, {    159,10}, \
    {    351,11}, {    191,10}, {    415,11}, {    223,10}, \
    {    447,12}, {    127,11}, {    255,10}, {    543,11}, \
    {    287,10}, {    607,11}, {    351,12}, {    191,11}, \
    {    479,13}, {    127,12}, {    255,11}, {    575,12}, \
    {    319,11}, {    703,12}, {    383,11}, {    767,12}, \
    {    447,13}, {   8192,14}, {  16384,15}, {  32768,16} }
#define SQR_FFT_TABLE3_SIZE 96
#define SQR_FFT_THRESHOLD                 5504

#define MULLO_BASECASE_THRESHOLD             0  /* always */
#define MULLO_DC_THRESHOLD                  48
#define MULLO_MUL_N_THRESHOLD             6633

#define DC_DIV_QR_THRESHOLD                 43
#define DC_DIVAPPR_Q_THRESHOLD             170
#define DC_BDIV_QR_THRESHOLD                43
#define DC_BDIV_Q_THRESHOLD                110

#define INV_MULMOD_BNM1_THRESHOLD           30
#define INV_NEWTON_THRESHOLD               177
#define INV_APPR_THRESHOLD                 171

#define BINV_NEWTON_THRESHOLD              194
#define REDC_1_TO_REDC_N_THRESHOLD          50

#define MU_DIV_QR_THRESHOLD               1142
#define MU_DIVAPPR_Q_THRESHOLD            1142
#define MUPI_DIV_QR_THRESHOLD               90
#define MU_BDIV_QR_THRESHOLD               942
#define MU_BDIV_Q_THRESHOLD               1017

#define MATRIX22_STRASSEN_THRESHOLD         13
#define HGCD_THRESHOLD                      92
#define GCD_DC_THRESHOLD                   283
#define GCDEXT_DC_THRESHOLD                221
#define JACOBI_BASE_METHOD                   2

#define GET_STR_DC_THRESHOLD                18
#define GET_STR_PRECOMPUTE_THRESHOLD        31
#define SET_STR_DC_THRESHOLD               490
#define SET_STR_PRECOMPUTE_THRESHOLD       994
