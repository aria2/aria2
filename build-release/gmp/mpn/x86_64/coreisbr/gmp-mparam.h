/* Sandy Bridge gmp-mparam.h -- Compiler/machine parameter header file.

Copyright 1991, 1993, 1994, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007,
2008, 2009, 2010, 2011, 2012 Free Software Foundation, Inc.

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

/* 3300 MHz Core i5 Sandy Bridge */

#define MOD_1_NORM_THRESHOLD                 0  /* always */
#define MOD_1_UNNORM_THRESHOLD               0  /* always */
#define MOD_1N_TO_MOD_1_1_THRESHOLD          3
#define MOD_1U_TO_MOD_1_1_THRESHOLD          3
#define MOD_1_1_TO_MOD_1_2_THRESHOLD         9
#define MOD_1_2_TO_MOD_1_4_THRESHOLD        20
#define PREINV_MOD_1_TO_MOD_1_THRESHOLD     10
#define USE_PREINV_DIVREM_1                  1  /* native */
#define DIV_QR_2_PI2_THRESHOLD           MP_SIZE_T_MAX  /* never */
#define DIVEXACT_1_THRESHOLD                 0  /* always (native) */
#define BMOD_1_TO_MOD_1_THRESHOLD           30

#define MUL_TOOM22_THRESHOLD                18
#define MUL_TOOM33_THRESHOLD                57
#define MUL_TOOM44_THRESHOLD               154
#define MUL_TOOM6H_THRESHOLD               226
#define MUL_TOOM8H_THRESHOLD               333

#define MUL_TOOM32_TO_TOOM43_THRESHOLD      97
#define MUL_TOOM32_TO_TOOM53_THRESHOLD     108
#define MUL_TOOM42_TO_TOOM53_THRESHOLD     105
#define MUL_TOOM42_TO_TOOM63_THRESHOLD     114
#define MUL_TOOM43_TO_TOOM54_THRESHOLD     138

#define SQR_BASECASE_THRESHOLD               0  /* always (native) */
#define SQR_TOOM2_THRESHOLD                 26
#define SQR_TOOM3_THRESHOLD                 81
#define SQR_TOOM4_THRESHOLD                250
#define SQR_TOOM6_THRESHOLD                345
#define SQR_TOOM8_THRESHOLD                381

#define MULMID_TOOM42_THRESHOLD             24

#define MULMOD_BNM1_THRESHOLD               14
#define SQRMOD_BNM1_THRESHOLD               14

#define POWM_SEC_TABLE  4,35,516,1036,1222

#define MUL_FFT_MODF_THRESHOLD             380  /* k = 5 */
#define MUL_FFT_TABLE3                                      \
  { {    380, 5}, {     17, 6}, {      9, 5}, {     19, 6}, \
    {     11, 5}, {     23, 6}, {     21, 7}, {     11, 6}, \
    {     23, 7}, {     21, 8}, {     11, 7}, {     24, 8}, \
    {     13, 7}, {     27, 8}, {     15, 7}, {     31, 8}, \
    {     17, 7}, {     35, 8}, {     19, 7}, {     39, 8}, \
    {     21, 9}, {     11, 8}, {     27, 9}, {     15, 8}, \
    {     35, 9}, {     19, 8}, {     41, 9}, {     23, 8}, \
    {     49, 9}, {     27,10}, {     15, 9}, {     39,10}, \
    {     23, 9}, {     51,11}, {     15,10}, {     31, 9}, \
    {     67,10}, {     39, 9}, {     83,10}, {     47, 9}, \
    {     95,10}, {     55,11}, {     31,10}, {     79,11}, \
    {     47,10}, {     95,12}, {     31,11}, {     63,10}, \
    {    135,11}, {     79,10}, {    159, 9}, {    319,10}, \
    {    167,11}, {     95,10}, {    191, 9}, {    383,12}, \
    {     63,11}, {    127,10}, {    255, 9}, {    511,10}, \
    {    271,11}, {    143,10}, {    287, 9}, {    575,10}, \
    {    303,11}, {    159,10}, {    319,12}, {     95,11}, \
    {    191,10}, {    383,11}, {    207,10}, {    415,13}, \
    {     63,12}, {    127,11}, {    255,10}, {    511,11}, \
    {    271,10}, {    543,11}, {    287,10}, {    575,11}, \
    {    303,10}, {    607,12}, {    159,11}, {    319,10}, \
    {    639,11}, {    351,10}, {    703, 9}, {   1407,11}, \
    {    367,12}, {    191,11}, {    383,10}, {    767,11}, \
    {    415,10}, {    831,12}, {    223,11}, {    447,10}, \
    {    895,13}, {    127,12}, {    255,11}, {    543,10}, \
    {   1087,12}, {    287,11}, {    575,10}, {   1151,11}, \
    {    607,12}, {    319,11}, {    639,12}, {    351,11}, \
    {    703,10}, {   1407,11}, {    735,13}, {    191,12}, \
    {    383,11}, {    767,12}, {    415,11}, {    831,10}, \
    {   1663,12}, {    447,11}, {    895,14}, {    127,13}, \
    {    255,12}, {    511,11}, {   1023,12}, {    543,11}, \
    {   1087,12}, {    575,11}, {   1151,12}, {    607,11}, \
    {   1215,13}, {    319,12}, {    639,11}, {   1279,12}, \
    {    703,11}, {   1407,13}, {    383,12}, {    767,11}, \
    {   1535,12}, {    831,11}, {   1663,13}, {    447,12}, \
    {    959,11}, {   1919,14}, {    255,13}, {    511,12}, \
    {   1087,13}, {    575,12}, {   1215,11}, {   2431,13}, \
    {    639,12}, {   1279,13}, {    703,12}, {   1407,14}, \
    {    383,13}, {    831,12}, {   1663,13}, {    959,12}, \
    {   1919,14}, {    511,13}, {   1087,12}, {   2175,13}, \
    {   1215,12}, {   2431,14}, {    639,13}, {   1343,12}, \
    {   2687,13}, {   1407,12}, {   2815,13}, {   1471,14}, \
    {    767,13}, {   1663,14}, {    895,13}, {   1919,15}, \
    {    511,14}, {   1023,13}, {   2175,14}, {   1151,13}, \
    {   2431,12}, {   4863,14}, {   1279,13}, {   2687,14}, \
    {   1407,13}, {   2815,15}, {    767,14}, {   1663,13}, \
    {   3455,14}, {   1919,13}, {   3839,16}, {    511,15}, \
    {   1023,14}, {   2431,13}, {   4863,15}, {   1279,14}, \
    {   2943,13}, {   5887,15}, {  32768,16}, {  65536,17}, \
    { 131072,18}, { 262144,19}, { 524288,20}, {1048576,21}, \
    {2097152,22}, {4194304,23}, {8388608,24} }
#define MUL_FFT_TABLE3_SIZE 203
#define MUL_FFT_THRESHOLD                 4736

#define SQR_FFT_MODF_THRESHOLD             304  /* k = 5 */
#define SQR_FFT_TABLE3                                      \
  { {    304, 5}, {     17, 6}, {      9, 5}, {     19, 6}, \
    {     21, 7}, {     11, 6}, {     23, 7}, {     21, 8}, \
    {     11, 7}, {     24, 8}, {     13, 7}, {     27, 8}, \
    {     15, 7}, {     31, 8}, {     21, 9}, {     11, 8}, \
    {     27, 9}, {     15, 8}, {     33, 9}, {     19, 8}, \
    {     41, 9}, {     23, 8}, {     47, 9}, {     27,10}, \
    {     15, 9}, {     39,10}, {     23, 9}, {     47,11}, \
    {     15,10}, {     31, 9}, {     63,10}, {     39, 9}, \
    {     79,10}, {     47,11}, {     31,10}, {     79,11}, \
    {     47,12}, {     31,11}, {     63,10}, {    127, 9}, \
    {    255, 8}, {    511,10}, {    135,11}, {     79,10}, \
    {    159, 9}, {    319,11}, {     95,10}, {    191, 9}, \
    {    383,12}, {     63,11}, {    127,10}, {    255, 9}, \
    {    511,10}, {    271, 9}, {    543,11}, {    143,10}, \
    {    287, 9}, {    575,11}, {    159,10}, {    319, 9}, \
    {    639,12}, {     95,11}, {    191,10}, {    383, 9}, \
    {    767,11}, {    207,13}, {     63,12}, {    127,11}, \
    {    255,10}, {    511,11}, {    271,10}, {    543,11}, \
    {    287,10}, {    575,11}, {    303,12}, {    159,11}, \
    {    319,10}, {    639,11}, {    351,10}, {    703,12}, \
    {    191,11}, {    383,10}, {    767,11}, {    415,10}, \
    {    831,12}, {    223,11}, {    447,10}, {    895,11}, \
    {    479,10}, {    959,13}, {    127,12}, {    255,11}, \
    {    511,10}, {   1023,11}, {    543,12}, {    287,11}, \
    {    575,10}, {   1151,11}, {    607,12}, {    319,11}, \
    {    639,10}, {   1279,12}, {    351,11}, {    703,13}, \
    {    191,12}, {    383,11}, {    767,12}, {    415,11}, \
    {    831,12}, {    447,11}, {    895,12}, {    479,11}, \
    {    959,10}, {   1919,14}, {    127,13}, {    255,12}, \
    {    511,11}, {   1023,12}, {    543,11}, {   1087,12}, \
    {    575,11}, {   1151,12}, {    607,13}, {    319,12}, \
    {    639,11}, {   1279,12}, {    703,11}, {   1407,13}, \
    {    383,12}, {    767,11}, {   1535,12}, {    831,13}, \
    {    447,12}, {    959,11}, {   1919,14}, {    255,13}, \
    {    511,12}, {   1087,13}, {    575,12}, {   1215,11}, \
    {   2431,13}, {    639,12}, {   1279,13}, {    703,12}, \
    {   1407,14}, {    383,13}, {    767,12}, {   1535,13}, \
    {    831,12}, {   1663,13}, {    959,12}, {   1919,15}, \
    {    255,14}, {    511,13}, {   1087,12}, {   2175,13}, \
    {   1215,12}, {   2431,14}, {    639,13}, {   1343,12}, \
    {   2687,13}, {   1407,12}, {   2815,13}, {   1471,14}, \
    {    767,13}, {   1663,14}, {    895,13}, {   1919,15}, \
    {    511,14}, {   1023,13}, {   2175,14}, {   1151,13}, \
    {   2431,12}, {   4863,14}, {   1279,13}, {   2687,14}, \
    {   1407,13}, {   2815,15}, {    767,14}, {   1663,13}, \
    {   3455,14}, {   1919,16}, {    511,15}, {   1023,14}, \
    {   2431,13}, {   4863,15}, {   1279,14}, {   2943,13}, \
    {   5887,15}, {  32768,16}, {  65536,17}, { 131072,18}, \
    { 262144,19}, { 524288,20}, {1048576,21}, {2097152,22}, \
    {4194304,23}, {8388608,24} }
#define SQR_FFT_TABLE3_SIZE 198
#define SQR_FFT_THRESHOLD                 2752

#define MULLO_BASECASE_THRESHOLD             0  /* always */
#define MULLO_DC_THRESHOLD                  51
#define MULLO_MUL_N_THRESHOLD             8648

#define DC_DIV_QR_THRESHOLD                 63
#define DC_DIVAPPR_Q_THRESHOLD             196
#define DC_BDIV_QR_THRESHOLD                59
#define DC_BDIV_Q_THRESHOLD                134

#define INV_MULMOD_BNM1_THRESHOLD           46
#define INV_NEWTON_THRESHOLD               202
#define INV_APPR_THRESHOLD                 190

#define BINV_NEWTON_THRESHOLD              224
#define REDC_1_TO_REDC_2_THRESHOLD          16
#define REDC_2_TO_REDC_N_THRESHOLD          55

#define MU_DIV_QR_THRESHOLD               1442
#define MU_DIVAPPR_Q_THRESHOLD            1528
#define MUPI_DIV_QR_THRESHOLD               85
#define MU_BDIV_QR_THRESHOLD              1187
#define MU_BDIV_Q_THRESHOLD               1387

#define MATRIX22_STRASSEN_THRESHOLD         15
#define HGCD_THRESHOLD                     113
#define HGCD_APPR_THRESHOLD                 84
#define HGCD_REDUCE_THRESHOLD             2681
#define GCD_DC_THRESHOLD                   555
#define GCDEXT_DC_THRESHOLD                396
#define JACOBI_BASE_METHOD                   4

#define GET_STR_DC_THRESHOLD                12
#define GET_STR_PRECOMPUTE_THRESHOLD        20
#define SET_STR_DC_THRESHOLD              1204
#define SET_STR_PRECOMPUTE_THRESHOLD      2251

#define FAC_DSC_THRESHOLD                  800
#define FAC_ODD_THRESHOLD                   28
