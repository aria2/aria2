/* Core 2 gmp-mparam.h -- Compiler/machine parameter header file.

Copyright 1991, 1993, 1994, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007,
2008, 2009, 2010, 2012 Free Software Foundation, Inc.

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

#define GMP_LIMB_BITS 64
#define BYTES_PER_MP_LIMB 8

/* 2133 MHz Core 2 (65nm) */

#define MOD_1_NORM_THRESHOLD                 0  /* always */
#define MOD_1_UNNORM_THRESHOLD               0  /* always */
#define MOD_1N_TO_MOD_1_1_THRESHOLD          3
#define MOD_1U_TO_MOD_1_1_THRESHOLD          2
#define MOD_1_1_TO_MOD_1_2_THRESHOLD        11
#define MOD_1_2_TO_MOD_1_4_THRESHOLD        15
#define PREINV_MOD_1_TO_MOD_1_THRESHOLD      6
#define USE_PREINV_DIVREM_1                  1  /* native */
#define DIV_QR_2_PI2_THRESHOLD           MP_SIZE_T_MAX  /* never */
#define DIVEXACT_1_THRESHOLD                 0  /* always (native) */
#define BMOD_1_TO_MOD_1_THRESHOLD           23

#define MUL_TOOM22_THRESHOLD                23
#define MUL_TOOM33_THRESHOLD                65
#define MUL_TOOM44_THRESHOLD               106
#define MUL_TOOM6H_THRESHOLD               224
#define MUL_TOOM8H_THRESHOLD                 0  /* always */

#define MUL_TOOM32_TO_TOOM43_THRESHOLD      69
#define MUL_TOOM32_TO_TOOM53_THRESHOLD      76
#define MUL_TOOM42_TO_TOOM53_THRESHOLD      73
#define MUL_TOOM42_TO_TOOM63_THRESHOLD      72
#define MUL_TOOM43_TO_TOOM54_THRESHOLD     100

#define SQR_BASECASE_THRESHOLD               0  /* always (native) */
#define SQR_TOOM2_THRESHOLD                 28
#define SQR_TOOM3_THRESHOLD                101
#define SQR_TOOM4_THRESHOLD                148
#define SQR_TOOM6_THRESHOLD                206
#define SQR_TOOM8_THRESHOLD                296

#define MULMID_TOOM42_THRESHOLD             24

#define MULMOD_BNM1_THRESHOLD               18
#define SQRMOD_BNM1_THRESHOLD               17

#define MUL_FFT_MODF_THRESHOLD             380  /* k = 5 */
#define MUL_FFT_TABLE3                                      \
  { {    380, 5}, {     15, 6}, {      8, 5}, {     17, 6}, \
    {      9, 5}, {     19, 6}, {     19, 7}, {     10, 6}, \
    {     21, 7}, {     11, 6}, {     23, 7}, {     13, 6}, \
    {     27, 7}, {     21, 8}, {     11, 7}, {     23, 8}, \
    {     13, 7}, {     27, 8}, {     15, 7}, {     31, 8}, \
    {     17, 7}, {     35, 8}, {     19, 7}, {     39, 8}, \
    {     21, 9}, {     11, 8}, {     27, 9}, {     15, 8}, \
    {     35, 9}, {     19, 8}, {     41, 9}, {     23, 8}, \
    {     47, 9}, {     27,10}, {     15, 9}, {     43,10}, \
    {     23, 9}, {     55,11}, {     15,10}, {     31, 9}, \
    {     67,10}, {     39, 9}, {     79,10}, {     55,11}, \
    {     31, 9}, {    127,10}, {     71, 8}, {    287,10}, \
    {     79,11}, {     47,12}, {     31,11}, {     63, 9}, \
    {    255,10}, {    135, 9}, {    271,11}, {     79, 9}, \
    {    319,10}, {    175,11}, {     95,10}, {    191, 9}, \
    {    383,11}, {    111,12}, {     63,11}, {    127,10}, \
    {    271, 9}, {    543,11}, {    143,10}, {    287,11}, \
    {    159,10}, {    319, 9}, {    639,11}, {    175,10}, \
    {    351,12}, {     95,11}, {    191,10}, {    383,11}, \
    {    207,10}, {    415,13}, {     63,12}, {    127,11}, \
    {    271,10}, {    543,11}, {    287,12}, {    159,11}, \
    {    319,10}, {    671,11}, {    351,12}, {    191,11}, \
    {    415,12}, {    223,11}, {    447,10}, {    895,11}, \
    {    479,13}, {    127,12}, {    287,11}, {    607,12}, \
    {    319,11}, {    671,12}, {    351,13}, {    191,12}, \
    {    415,11}, {    831,10}, {   1663,12}, {    479,14}, \
    {    127,13}, {    255,12}, {    543,11}, {   1087,12}, \
    {    607,13}, {    319,12}, {    703,13}, {    383,12}, \
    {    767,10}, {   3071,12}, {    831,13}, {    447,12}, \
    {    959,14}, {    255,13}, {    511,12}, {   1023,13}, \
    {    575,12}, {   1151,11}, {   2303,13}, {    639,12}, \
    {   1343,13}, {    703,14}, {    383,13}, {    831,12}, \
    {   1727,13}, {    959,15}, {    255,14}, {    511,13}, \
    {   1087,12}, {   2175,13}, {   1215,14}, {    639,13}, \
    {   1343,12}, {   2687,13}, {   1407,12}, {   2815,14}, \
    {    767,13}, {   1663,14}, {    895,13}, {   1919,15}, \
    {    511,14}, {   1023,13}, {   2175,14}, {   1151,13}, \
    {   2431,12}, {   4863,13}, {   2495,14}, {   1279,13}, \
    {   2687,14}, {   1407,15}, {    767,14}, {   1663,13}, \
    {   3327,12}, {   6655,13}, {   3455,12}, {   6911,14}, \
    {  16384,15}, {  32768,16}, {  65536,17}, { 131072,18}, \
    { 262144,19}, { 524288,20}, {1048576,21}, {2097152,22}, \
    {4194304,23}, {8388608,24} }
#define MUL_FFT_TABLE3_SIZE 170
#define MUL_FFT_THRESHOLD                 4736

#define SQR_FFT_MODF_THRESHOLD             308  /* k = 5 */
#define SQR_FFT_TABLE3                                      \
  { {    308, 5}, {     17, 6}, {      9, 5}, {     19, 6}, \
    {     21, 7}, {     11, 6}, {     23, 7}, {     12, 6}, \
    {     25, 7}, {     21, 8}, {     11, 7}, {     24, 8}, \
    {     13, 7}, {     27, 8}, {     15, 7}, {     31, 8}, \
    {     21, 9}, {     11, 8}, {     27, 9}, {     15, 8}, \
    {     35, 9}, {     19, 8}, {     41, 9}, {     23, 8}, \
    {     47, 9}, {     27,10}, {     15, 9}, {     39,10}, \
    {     23, 9}, {     59,11}, {     15,10}, {     31, 8}, \
    {    125, 9}, {     67,10}, {     39, 9}, {     79,10}, \
    {     47, 9}, {    103,10}, {     79, 9}, {    159,10}, \
    {     87, 9}, {    175, 8}, {    351,11}, {     47,10}, \
    {     95,11}, {     63,10}, {    127, 8}, {    511, 9}, \
    {    271, 8}, {    543,11}, {     79,10}, {    175,11}, \
    {     95,10}, {    191, 9}, {    415,12}, {     63,11}, \
    {    127,10}, {    255,11}, {    143,10}, {    287, 9}, \
    {    575,11}, {    159,10}, {    319,11}, {    175,10}, \
    {    351,12}, {     95,11}, {    191,10}, {    383,11}, \
    {    207,10}, {    415,13}, {     63,12}, {    127,11}, \
    {    255,10}, {    511, 9}, {   1023,10}, {    543,11}, \
    {    287,10}, {    575,11}, {    303,12}, {    159,11}, \
    {    319,10}, {    639,11}, {    351,12}, {    191,11}, \
    {    383,10}, {    767,11}, {    415,10}, {    831,12}, \
    {    223,11}, {    479,13}, {    127,12}, {    255,11}, \
    {    543,12}, {    287,11}, {    575,12}, {    319,11}, \
    {    639,12}, {    351,13}, {    191,12}, {    383,11}, \
    {    767,12}, {    415,11}, {    831,12}, {    447,11}, \
    {    895,12}, {    479,14}, {    127,13}, {    255,12}, \
    {    543,11}, {   1087,12}, {    607,13}, {    319,12}, \
    {    639,11}, {   1279,12}, {    703,13}, {    383,12}, \
    {    831,13}, {    447,12}, {    959,14}, {    255,13}, \
    {    511,12}, {   1087,13}, {    575,12}, {   1215,13}, \
    {    639,12}, {   1279,13}, {    703,14}, {    383,13}, \
    {    767,12}, {   1535,13}, {    831,12}, {   1663,13}, \
    {    959,15}, {    255,14}, {    511,13}, {   1087,12}, \
    {   2175,13}, {   1215,14}, {    639,13}, {   1343,12}, \
    {   2687,13}, {   1407,12}, {   2815,14}, {    767,13}, \
    {   1535,12}, {   3071,13}, {   1663,14}, {    895,13}, \
    {   1791,15}, {    511,14}, {   1023,13}, {   2175,14}, \
    {   1151,13}, {   2303,12}, {   4607,13}, {   2431,12}, \
    {   4863,14}, {   1279,13}, {   2687,14}, {   1407,13}, \
    {   2815,15}, {    767,14}, {   1535,13}, {   3071,14}, \
    {   1663,13}, {   3327,12}, {   6655,13}, {   3455,12}, \
    {   6911,14}, {   1791,16}, {  65536,17}, { 131072,18}, \
    { 262144,19}, { 524288,20}, {1048576,21}, {2097152,22}, \
    {4194304,23}, {8388608,24} }
#define SQR_FFT_TABLE3_SIZE 178
#define SQR_FFT_THRESHOLD                 3520

#define MULLO_BASECASE_THRESHOLD             0  /* always */
#define MULLO_DC_THRESHOLD                  55
#define MULLO_MUL_N_THRESHOLD             9174

#define DC_DIV_QR_THRESHOLD                 54
#define DC_DIVAPPR_Q_THRESHOLD             179
#define DC_BDIV_QR_THRESHOLD                53
#define DC_BDIV_Q_THRESHOLD                125

#define INV_MULMOD_BNM1_THRESHOLD           62
#define INV_NEWTON_THRESHOLD               173
#define INV_APPR_THRESHOLD                 172

#define BINV_NEWTON_THRESHOLD              230
#define REDC_1_TO_REDC_2_THRESHOLD          10
#define REDC_2_TO_REDC_N_THRESHOLD          63

#define MU_DIV_QR_THRESHOLD               1334
#define MU_DIVAPPR_Q_THRESHOLD            1210
#define MUPI_DIV_QR_THRESHOLD               79
#define MU_BDIV_QR_THRESHOLD              1057
#define MU_BDIV_Q_THRESHOLD               1187

#define POWM_SEC_TABLE  2,65,322,780


#define MATRIX22_STRASSEN_THRESHOLD         17
#define HGCD_THRESHOLD                     135
#define HGCD_APPR_THRESHOLD                178
#define HGCD_REDUCE_THRESHOLD             2121
#define GCD_DC_THRESHOLD                   416
#define GCDEXT_DC_THRESHOLD                361
#define JACOBI_BASE_METHOD                   4

#define GET_STR_DC_THRESHOLD                12
#define GET_STR_PRECOMPUTE_THRESHOLD        21
#define SET_STR_DC_THRESHOLD               552
#define SET_STR_PRECOMPUTE_THRESHOLD      1815

#define FAC_DSC_THRESHOLD                  608
#define FAC_ODD_THRESHOLD                   28
