/* PowerPC-32 gmp-mparam.h -- Compiler/machine parameter header file.

Copyright 1991, 1993, 1994, 1999, 2000, 2001, 2002, 2003, 2004, 2008, 2009,
2010 Free Software Foundation, Inc.

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


/* This file is supposed to be used for 604, 604e, 744x/745x/747x (G4+), i.e.,
   32-bit PowerPC processors with reasonably fast integer multiply insns.  The
   values below are chosen to be best for the latter processors, since 604 is
   largely irrelevant today.

   In mpn/powerpc32/750/gmp-mparam.h there are values for 75x (G3) and for
   7400/7410 (G4), both which have much slower multiply instructions.  */

/* 1417 MHz PPC 7447A */

#define DIVREM_1_NORM_THRESHOLD              0  /* always */
#define DIVREM_1_UNNORM_THRESHOLD            0  /* always */
#define MOD_1_1P_METHOD                      1
#define MOD_1_NORM_THRESHOLD                 0  /* always */
#define MOD_1_UNNORM_THRESHOLD               0  /* always */
#define MOD_1N_TO_MOD_1_1_THRESHOLD          8
#define MOD_1U_TO_MOD_1_1_THRESHOLD          6
#define MOD_1_1_TO_MOD_1_2_THRESHOLD         8
#define MOD_1_2_TO_MOD_1_4_THRESHOLD        49
#define PREINV_MOD_1_TO_MOD_1_THRESHOLD     18
#define USE_PREINV_DIVREM_1                  1
#define DIV_QR_2_PI2_THRESHOLD           MP_SIZE_T_MAX  /* never */
#define DIVEXACT_1_THRESHOLD                 0  /* always */
#define BMOD_1_TO_MOD_1_THRESHOLD           66

#define MUL_TOOM22_THRESHOLD                14
#define MUL_TOOM33_THRESHOLD                73
#define MUL_TOOM44_THRESHOLD               106
#define MUL_TOOM6H_THRESHOLD               157
#define MUL_TOOM8H_THRESHOLD               236

#define MUL_TOOM32_TO_TOOM43_THRESHOLD      73
#define MUL_TOOM32_TO_TOOM53_THRESHOLD      72
#define MUL_TOOM42_TO_TOOM53_THRESHOLD      73
#define MUL_TOOM42_TO_TOOM63_THRESHOLD      72
#define MUL_TOOM43_TO_TOOM54_THRESHOLD      82

#define SQR_BASECASE_THRESHOLD               4
#define SQR_TOOM2_THRESHOLD                 26
#define SQR_TOOM3_THRESHOLD                 77
#define SQR_TOOM4_THRESHOLD                136
#define SQR_TOOM6_THRESHOLD                189
#define SQR_TOOM8_THRESHOLD                284

#define MULMID_TOOM42_THRESHOLD             32

#define MULMOD_BNM1_THRESHOLD                9
#define SQRMOD_BNM1_THRESHOLD               14

#define MUL_FFT_MODF_THRESHOLD             284  /* k = 5 */
#define MUL_FFT_TABLE3                                      \
  { {    284, 5}, {     17, 6}, {      9, 5}, {     19, 6}, \
    {     13, 7}, {      7, 6}, {     17, 7}, {      9, 6}, \
    {     20, 7}, {     11, 6}, {     23, 7}, {     13, 8}, \
    {      7, 7}, {     19, 8}, {     11, 7}, {     25, 9}, \
    {      7, 8}, {     15, 7}, {     33, 8}, {     19, 7}, \
    {     39, 8}, {     23, 7}, {     47, 9}, {     15, 8}, \
    {     39, 9}, {     23, 8}, {     47,10}, {     15, 9}, \
    {     31, 8}, {     67, 9}, {     39, 8}, {     79, 9}, \
    {     47, 8}, {     95,10}, {     31, 9}, {     71, 8}, \
    {    143, 9}, {     79,10}, {     47, 9}, {     95,11}, \
    {     31,10}, {     63, 9}, {    127, 8}, {    255, 9}, \
    {    135, 8}, {    271, 9}, {    143,10}, {     79, 9}, \
    {    159, 8}, {    319, 9}, {    175,10}, {     95, 9}, \
    {    191, 8}, {    383, 9}, {    207, 8}, {    415,11}, \
    {     63,10}, {    127, 9}, {    255, 8}, {    511, 9}, \
    {    271,10}, {    143, 9}, {    287, 8}, {    575,10}, \
    {    159, 9}, {    319,10}, {    175,11}, {     95,10}, \
    {    191, 9}, {    383,10}, {    207, 9}, {    415, 8}, \
    {    831,12}, {     63,11}, {    127,10}, {    255, 9}, \
    {    511,10}, {    271, 9}, {    543, 8}, {   1087,10}, \
    {    287, 9}, {    575,11}, {    159,10}, {    319, 9}, \
    {    639,10}, {    351, 9}, {    703,11}, {    191,10}, \
    {    415, 9}, {    831,11}, {    223,10}, {    447, 9}, \
    {    895,10}, {    479, 9}, {    959,12}, {    127,11}, \
    {    255,10}, {    543, 9}, {   1087,11}, {    287,10}, \
    {    607,11}, {    319,10}, {    639,11}, {    351,10}, \
    {    703, 9}, {   1407,12}, {    191,11}, {    383,10}, \
    {    767,11}, {    415,10}, {    831,11}, {    447,10}, \
    {    895,11}, {    479,10}, {    959,13}, {    127,12}, \
    {    255,11}, {    543,10}, {   1087,11}, {    607,12}, \
    {    319,11}, {    639,10}, {   1279,11}, {    703,10}, \
    {   1407,12}, {    383,11}, {    831,12}, {    447,11}, \
    {    959,10}, {   1919,13}, {    255,12}, {    511,11}, \
    {   1087,12}, {    575,11}, {   1215,10}, {   2431,12}, \
    {    639,11}, {   1279,12}, {    703,11}, {   1407,13}, \
    {    383,12}, {    959,11}, {   1919,14}, {    255,13}, \
    {    511,12}, {   1215,11}, {   2431,13}, {    639,12}, \
    {   1471,13}, {    767,12}, {   1599,13}, {    895,12}, \
    {   1919,14}, {    511,13}, {   1023,12}, {   2111,13}, \
    {   1151,12}, {   2431,13}, {   1407,14}, {    767,13}, \
    {   1535,12}, {   3071,13}, {   1919,12}, {   3839,15}, \
    {  32768,16} }
#define MUL_FFT_TABLE3_SIZE 165
#define MUL_FFT_THRESHOLD                 3392

#define SQR_FFT_MODF_THRESHOLD             236  /* k = 5 */
#define SQR_FFT_TABLE3                                      \
  { {    248, 5}, {     17, 6}, {      9, 5}, {     19, 6}, \
    {     17, 7}, {      9, 6}, {     20, 7}, {     11, 6}, \
    {     23, 7}, {     13, 8}, {      7, 7}, {     19, 8}, \
    {     11, 7}, {     25, 9}, {      7, 8}, {     15, 7}, \
    {     33, 8}, {     19, 7}, {     39, 8}, {     23, 7}, \
    {     47, 8}, {     27, 9}, {     15, 8}, {     39, 9}, \
    {     23, 8}, {     47,10}, {     15, 9}, {     31, 8}, \
    {     63, 9}, {     39, 8}, {     79, 9}, {     47,10}, \
    {     31, 9}, {     63, 8}, {    127, 9}, {     71, 8}, \
    {    143, 9}, {     79,10}, {     47, 9}, {     95,11}, \
    {     31,10}, {     63, 9}, {    127, 8}, {    255, 7}, \
    {    511, 9}, {    143,10}, {     79, 9}, {    159, 8}, \
    {    319, 9}, {    175, 8}, {    351,10}, {     95, 9}, \
    {    191, 8}, {    383, 9}, {    207, 8}, {    415,11}, \
    {     63,10}, {    127, 9}, {    255, 8}, {    511,10}, \
    {    143, 9}, {    287, 8}, {    575,10}, {    159, 9}, \
    {    319,10}, {    175, 9}, {    351,11}, {     95,10}, \
    {    191, 9}, {    383,10}, {    207, 9}, {    415, 8}, \
    {    831,12}, {     63,11}, {    127,10}, {    255, 9}, \
    {    511,10}, {    271, 9}, {    543,10}, {    287, 9}, \
    {    575,11}, {    159,10}, {    319, 9}, {    639,10}, \
    {    351, 9}, {    703, 8}, {   1407, 9}, {    735,11}, \
    {    191,10}, {    415, 9}, {    831,11}, {    223,10}, \
    {    447, 9}, {    895,10}, {    479,12}, {    127,11}, \
    {    255,10}, {    543,11}, {    287,10}, {    607,11}, \
    {    319,10}, {    639,11}, {    351,10}, {    703, 9}, \
    {   1407,12}, {    191,11}, {    383,13}, {    127,12}, \
    {    255,11}, {    543,10}, {   1087,11}, {    575,10}, \
    {   1151,12}, {    319,11}, {    703,10}, {   1407,12}, \
    {    383,11}, {    831,12}, {    447,11}, {    959,13}, \
    {    255,12}, {    511,11}, {   1087,12}, {    575,11}, \
    {   1215,12}, {    639,11}, {   1279,12}, {    703,11}, \
    {   1407,13}, {    383,12}, {    959,14}, {    255,13}, \
    {    511,12}, {   1215,11}, {   2431,13}, {    639,12}, \
    {   1471,13}, {    767,12}, {   1599,13}, {    895,12}, \
    {   1919,14}, {    511,13}, {   1023,12}, {   2111,13}, \
    {   1151,12}, {   2431,13}, {   1407,12}, {   2815,14}, \
    {    767,13}, {   1535,12}, {   3199,13}, {   1919,15}, \
    {  32768,16} }
#define SQR_FFT_TABLE3_SIZE 153
#define SQR_FFT_THRESHOLD                 2368

#define MULLO_BASECASE_THRESHOLD             0  /* always */
#define MULLO_DC_THRESHOLD                  45
#define MULLO_MUL_N_THRESHOLD             6633

#define DC_DIV_QR_THRESHOLD                 43
#define DC_DIVAPPR_Q_THRESHOLD             153
#define DC_BDIV_QR_THRESHOLD                54
#define DC_BDIV_Q_THRESHOLD                124

#define INV_MULMOD_BNM1_THRESHOLD           42
#define INV_NEWTON_THRESHOLD               179
#define INV_APPR_THRESHOLD                 157

#define BINV_NEWTON_THRESHOLD              204
#define REDC_1_TO_REDC_N_THRESHOLD          54

#define MU_DIV_QR_THRESHOLD                998
#define MU_DIVAPPR_Q_THRESHOLD            1037
#define MUPI_DIV_QR_THRESHOLD               84
#define MU_BDIV_QR_THRESHOLD               748
#define MU_BDIV_Q_THRESHOLD                942

#define POWM_SEC_TABLE  4,23,164,616,1812

#define MATRIX22_STRASSEN_THRESHOLD         11
#define HGCD_THRESHOLD                     118
#define HGCD_APPR_THRESHOLD                167
#define HGCD_REDUCE_THRESHOLD             1679
#define GCD_DC_THRESHOLD                   339
#define GCDEXT_DC_THRESHOLD                273
#define JACOBI_BASE_METHOD                   4

#define GET_STR_DC_THRESHOLD                12
#define GET_STR_PRECOMPUTE_THRESHOLD        27
#define SET_STR_DC_THRESHOLD               781
#define SET_STR_PRECOMPUTE_THRESHOLD      1505

#define FAC_DSC_THRESHOLD                  141
#define FAC_ODD_THRESHOLD                   34
