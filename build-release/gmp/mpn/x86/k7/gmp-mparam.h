/* AMD K7 gmp-mparam.h -- Compiler/machine parameter header file.

Copyright 1991, 1993, 1994, 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009,
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


#define MOD_1_NORM_THRESHOLD                 0  /* always */
#define MOD_1_UNNORM_THRESHOLD               3
#define MOD_1N_TO_MOD_1_1_THRESHOLD          7
#define MOD_1U_TO_MOD_1_1_THRESHOLD          3
#define MOD_1_1_TO_MOD_1_2_THRESHOLD        24
#define MOD_1_2_TO_MOD_1_4_THRESHOLD         0  /* never mpn_mod_1s_2p */
#define PREINV_MOD_1_TO_MOD_1_THRESHOLD     10
#define USE_PREINV_DIVREM_1                  1  /* native */
#define DIV_QR_2_PI2_THRESHOLD           MP_SIZE_T_MAX  /* never */
#define DIVEXACT_1_THRESHOLD                 0  /* always (native) */
#define BMOD_1_TO_MOD_1_THRESHOLD           24

#define MUL_TOOM22_THRESHOLD                28
#define MUL_TOOM33_THRESHOLD                85
#define MUL_TOOM44_THRESHOLD               142
#define MUL_TOOM6H_THRESHOLD               258
#define MUL_TOOM8H_THRESHOLD               309

#define MUL_TOOM32_TO_TOOM43_THRESHOLD      85
#define MUL_TOOM32_TO_TOOM53_THRESHOLD      99
#define MUL_TOOM42_TO_TOOM53_THRESHOLD      97
#define MUL_TOOM42_TO_TOOM63_THRESHOLD     102
#define MUL_TOOM43_TO_TOOM54_THRESHOLD     144

#define SQR_BASECASE_THRESHOLD               0  /* always (native) */
#define SQR_TOOM2_THRESHOLD                 50
#define SQR_TOOM3_THRESHOLD                 83
#define SQR_TOOM4_THRESHOLD                216
#define SQR_TOOM6_THRESHOLD                318
#define SQR_TOOM8_THRESHOLD                430

#define MULMID_TOOM42_THRESHOLD             56

#define MULMOD_BNM1_THRESHOLD               17
#define SQRMOD_BNM1_THRESHOLD               19

#define MUL_FFT_MODF_THRESHOLD             888  /* k = 6 */
#define MUL_FFT_TABLE3                                      \
  { {    888, 6}, {     25, 7}, {     13, 6}, {     27, 7}, \
    {     15, 6}, {     32, 7}, {     17, 6}, {     35, 7}, \
    {     19, 6}, {     39, 7}, {     23, 6}, {     47, 7}, \
    {     27, 8}, {     15, 7}, {     31, 6}, {     63, 7}, \
    {     35, 8}, {     19, 7}, {     39, 8}, {     23, 7}, \
    {     47, 8}, {     31, 7}, {     63, 8}, {     39, 7}, \
    {     79, 9}, {     23, 8}, {     47, 7}, {     95, 8}, \
    {     51, 9}, {     31, 8}, {     71, 9}, {     39, 8}, \
    {     79, 9}, {     47, 8}, {     95, 9}, {     55,10}, \
    {     31, 9}, {     63, 8}, {    127, 9}, {     71, 8}, \
    {    143, 9}, {     79,10}, {     47,11}, {     31,10}, \
    {     63, 9}, {    127,10}, {     79, 9}, {    167,10}, \
    {     95, 9}, {    207,10}, {    111,11}, {     63,10}, \
    {    127, 9}, {    255,10}, {    159, 9}, {    319,11}, \
    {     95,10}, {    191,12}, {     63,11}, {    127,10}, \
    {    271, 9}, {    543,10}, {    287,11}, {    159,10}, \
    {    319, 9}, {    671,11}, {    191,10}, {    383, 9}, \
    {    767,11}, {    223,12}, {    127,11}, {    255,10}, \
    {    511, 9}, {   1023,10}, {    543, 9}, {   1087,11}, \
    {    287,10}, {    575, 9}, {   1151,10}, {    607, 9}, \
    {   1215, 8}, {   2431,11}, {    319,10}, {    639, 9}, \
    {   1279,10}, {    671, 9}, {   1343,12}, {    191,11}, \
    {    383,10}, {    767, 9}, {   1535,10}, {    799, 9}, \
    {   1599,11}, {    415,10}, {    831, 9}, {   1663,13}, \
    {    127,12}, {    255,11}, {    511,10}, {   1023,11}, \
    {    543,10}, {   1087,11}, {    575,10}, {   1151,11}, \
    {    607,10}, {   1215,12}, {    319,11}, {    639,10}, \
    {   1279,11}, {    671,10}, {   1407, 9}, {   2815,11}, \
    {    735,10}, {   1471, 9}, {   2943,12}, {    383,11}, \
    {    767,10}, {   1535,11}, {    799,10}, {   1599,11}, \
    {    831,10}, {   1663,11}, {    863,10}, {   1727,11}, \
    {    895,10}, {   1791,11}, {    959,13}, {    255,12}, \
    {    511,11}, {   1023,10}, {   2047,11}, {   1087,12}, \
    {    575,11}, {   1151,10}, {   2303,11}, {   1215,10}, \
    {   2431,12}, {    639,11}, {   1407,10}, {   2815,11}, \
    {   1471,10}, {   2943,13}, {    383,12}, {    767,11}, \
    {   1599,12}, {    831,11}, {   1663,10}, {   3327,11}, \
    {   1727,12}, {    895,11}, {   1791,10}, {   3583,12}, \
    {    959,11}, {   1919,14}, {    255,13}, {    511,12}, \
    {   1023,11}, {   2047,12}, {   1087,11}, {   2239,12}, \
    {   1151,11}, {   2303,12}, {   1215,11}, {   2431,13}, \
    {   8192,14}, {  16384,15}, {  32768,16} }
#define MUL_FFT_TABLE3_SIZE 167
#define MUL_FFT_THRESHOLD                 7552

#define SQR_FFT_MODF_THRESHOLD             666  /* k = 6 */
#define SQR_FFT_TABLE3                                      \
  { {    786, 6}, {     25, 7}, {     13, 6}, {     27, 7}, \
    {     15, 6}, {     31, 7}, {     17, 6}, {     35, 7}, \
    {     19, 6}, {     39, 7}, {     23, 6}, {     47, 7}, \
    {     27, 8}, {     15, 7}, {     31, 6}, {     63, 7}, \
    {     35, 8}, {     19, 7}, {     39, 8}, {     23, 7}, \
    {     47, 8}, {     31, 7}, {     63, 8}, {     39, 9}, \
    {     23, 8}, {     47, 7}, {     95, 8}, {     51, 9}, \
    {     31, 8}, {     67, 9}, {     39, 8}, {     79, 9}, \
    {     47, 8}, {     95,10}, {     31, 9}, {     63, 8}, \
    {    127, 9}, {     79,10}, {     47, 9}, {     95, 8}, \
    {    191,11}, {     31,10}, {     63, 9}, {    135,10}, \
    {     79, 9}, {    167,10}, {     95, 9}, {    191,10}, \
    {    111,11}, {     63,10}, {    143, 9}, {    287, 8}, \
    {    607,10}, {    159, 9}, {    319,10}, {    175,11}, \
    {     95,10}, {    191, 9}, {    383,10}, {    207,12}, \
    {     63,11}, {    127,10}, {    255, 9}, {    511,10}, \
    {    287,11}, {    159,10}, {    319, 9}, {    639, 8}, \
    {   1279, 9}, {    671, 8}, {   1343,11}, {    191,10}, \
    {    383, 9}, {    767, 8}, {   1535, 9}, {    799, 8}, \
    {   1599,10}, {    415,11}, {    223,12}, {    127,11}, \
    {    255,10}, {    511, 9}, {   1023,10}, {    543, 9}, \
    {   1087,11}, {    287,10}, {    575, 9}, {   1151,10}, \
    {    607, 9}, {   1215, 8}, {   2431,11}, {    319,10}, \
    {    639, 9}, {   1279,10}, {    671, 9}, {   1343,12}, \
    {    191,11}, {    383,10}, {    767, 9}, {   1535,10}, \
    {    799, 9}, {   1599,11}, {    415,10}, {    863,13}, \
    {    127,12}, {    255,11}, {    511,10}, {   1023,11}, \
    {    543,10}, {   1087,11}, {    575,10}, {   1151, 9}, \
    {   2303,11}, {    607,10}, {   1215, 9}, {   2431,12}, \
    {    319,11}, {    639,10}, {   1279,11}, {    671,10}, \
    {   1407, 9}, {   2815,11}, {    735,10}, {   1471, 9}, \
    {   2943,11}, {    767,10}, {   1535,11}, {    799,10}, \
    {   1599,11}, {    831,10}, {   1663,11}, {    863,10}, \
    {   1727,11}, {    895,10}, {   1791,11}, {    959,10}, \
    {   1919,13}, {    255,12}, {    511,11}, {   1023,10}, \
    {   2047,11}, {   1087,10}, {   2175,12}, {    575,11}, \
    {   1151,10}, {   2303,11}, {   1215,10}, {   2431,12}, \
    {    639,11}, {   1407,10}, {   2815,11}, {   1471,10}, \
    {   2943,12}, {    767,11}, {   1599,12}, {    831,11}, \
    {   1663,10}, {   3327,12}, {    895,11}, {   1791,12}, \
    {    959,11}, {   1919,10}, {   3839,11}, {   1983,14}, \
    {    255,13}, {    511,12}, {   1023,11}, {   2047,12}, \
    {   1087,11}, {   2239,12}, {   1151,11}, {   2303,12}, \
    {   1215,11}, {   2431,13}, {   8192,14}, {  16384,15}, \
    {  32768,16} }
#define SQR_FFT_TABLE3_SIZE 177
#define SQR_FFT_THRESHOLD                 7040

#define MULLO_BASECASE_THRESHOLD            11
#define MULLO_DC_THRESHOLD                  35
#define MULLO_MUL_N_THRESHOLD            13463

#define DC_DIV_QR_THRESHOLD                 41
#define DC_DIVAPPR_Q_THRESHOLD             214
#define DC_BDIV_QR_THRESHOLD                41
#define DC_BDIV_Q_THRESHOLD                148

#define INV_MULMOD_BNM1_THRESHOLD           77
#define INV_NEWTON_THRESHOLD               204
#define INV_APPR_THRESHOLD                 204

#define BINV_NEWTON_THRESHOLD              230
#define REDC_1_TO_REDC_N_THRESHOLD          59

#define MU_DIV_QR_THRESHOLD               1752
#define MU_DIVAPPR_Q_THRESHOLD            1528
#define MUPI_DIV_QR_THRESHOLD               82
#define MU_BDIV_QR_THRESHOLD              1360
#define MU_BDIV_Q_THRESHOLD               1470

#define POWM_SEC_TABLE  2,17,176,905,2246

#define MATRIX22_STRASSEN_THRESHOLD         16
#define HGCD_THRESHOLD                     125
#define HGCD_APPR_THRESHOLD                143
#define HGCD_REDUCE_THRESHOLD             4633
#define GCD_DC_THRESHOLD                   460
#define GCDEXT_DC_THRESHOLD                330
#define JACOBI_BASE_METHOD                   4

#define GET_STR_DC_THRESHOLD                15
#define GET_STR_PRECOMPUTE_THRESHOLD        35
#define SET_STR_DC_THRESHOLD               272
#define SET_STR_PRECOMPUTE_THRESHOLD      1183

#define FAC_DSC_THRESHOLD                  336
#define FAC_ODD_THRESHOLD                   29
