/* VIA Nano gmp-mparam.h -- Compiler/machine parameter header file.

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

#define SHLD_SLOW 1
#define SHRD_SLOW 1

/* 1600 MHz Nano 2xxx */

#define MOD_1_NORM_THRESHOLD                 0  /* always */
#define MOD_1_UNNORM_THRESHOLD               0  /* always */
#define MOD_1N_TO_MOD_1_1_THRESHOLD          4
#define MOD_1U_TO_MOD_1_1_THRESHOLD          3
#define MOD_1_1_TO_MOD_1_2_THRESHOLD        18
#define MOD_1_2_TO_MOD_1_4_THRESHOLD        20
#define PREINV_MOD_1_TO_MOD_1_THRESHOLD      8
#define USE_PREINV_DIVREM_1                  1  /* native */
#define DIV_QR_2_PI2_THRESHOLD           MP_SIZE_T_MAX  /* never */
#define DIVEXACT_1_THRESHOLD                 0  /* always (native) */
#define BMOD_1_TO_MOD_1_THRESHOLD           22

#define MUL_TOOM22_THRESHOLD                27
#define MUL_TOOM33_THRESHOLD                33
#define MUL_TOOM44_THRESHOLD               290
#define MUL_TOOM6H_THRESHOLD               718
#define MUL_TOOM8H_THRESHOLD               915

#define MUL_TOOM32_TO_TOOM43_THRESHOLD      67
#define MUL_TOOM32_TO_TOOM53_THRESHOLD     184
#define MUL_TOOM42_TO_TOOM53_THRESHOLD     193
#define MUL_TOOM42_TO_TOOM63_THRESHOLD     193
#define MUL_TOOM43_TO_TOOM54_THRESHOLD     287

#define SQR_BASECASE_THRESHOLD               0  /* always (native) */
#define SQR_TOOM2_THRESHOLD                 34
#define SQR_TOOM3_THRESHOLD                 93
#define SQR_TOOM4_THRESHOLD                587
#define SQR_TOOM6_THRESHOLD               1095
#define SQR_TOOM8_THRESHOLD                  0  /* always */

#define MULMID_TOOM42_THRESHOLD             28

#define MULMOD_BNM1_THRESHOLD               13
#define SQRMOD_BNM1_THRESHOLD               17

#define MUL_FFT_MODF_THRESHOLD             376  /* k = 5 */
#define MUL_FFT_TABLE3                                      \
  { {    376, 5}, {     21, 6}, {     11, 5}, {     23, 6}, \
    {     12, 5}, {     25, 6}, {     13, 5}, {     27, 6}, \
    {     15, 5}, {     31, 6}, {     21, 7}, {     11, 6}, \
    {     24, 7}, {     13, 6}, {     27, 7}, {     15, 6}, \
    {     31, 7}, {     19, 6}, {     39, 7}, {     21, 8}, \
    {     11, 7}, {     25, 8}, {     13, 7}, {     27, 8}, \
    {     15, 7}, {     32, 8}, {     17, 7}, {     35, 8}, \
    {     19, 7}, {     40, 8}, {     23, 7}, {     47, 8}, \
    {     27, 9}, {     15, 8}, {     35, 9}, {     19, 8}, \
    {     39, 4}, {    767, 5}, {    399, 6}, {    201, 5}, \
    {    415, 6}, {    208, 7}, {    105, 6}, {    214, 7}, \
    {    127, 8}, {     71, 9}, {     39, 8}, {     87, 9}, \
    {     47, 8}, {     97, 9}, {     55,11}, {     15,10}, \
    {     31, 9}, {     67, 8}, {    135, 9}, {     75,10}, \
    {     39, 9}, {     87,10}, {     47, 9}, {     99,10}, \
    {     55,11}, {     31,10}, {     63, 9}, {    127,10}, \
    {     87,11}, {     47,10}, {    103,12}, {     31,11}, \
    {     63,10}, {    143,11}, {     79,10}, {    167,11}, \
    {     95,10}, {    199,11}, {    111,12}, {     63,11}, \
    {    127, 9}, {    511,11}, {    143,10}, {    287,11}, \
    {    159, 9}, {    639,11}, {    175,12}, {     95,11}, \
    {    191,10}, {    383, 9}, {    767,11}, {    207,10}, \
    {    415, 9}, {    831,13}, {     63,12}, {    127,11}, \
    {    255,10}, {    511, 9}, {   1023,11}, {    271,10}, \
    {    543, 9}, {   1087,11}, {    287,10}, {    575,12}, \
    {    159,11}, {    319,10}, {    639, 9}, {   1279,11}, \
    {    335,10}, {    671, 9}, {   1343,10}, {    703, 9}, \
    {   1407,12}, {    191,11}, {    383,10}, {    767, 9}, \
    {   1535,11}, {    415,10}, {    831, 9}, {   1663,12}, \
    {    223,11}, {    447,13}, {    127,12}, {    255,11}, \
    {    511,10}, {   1023,11}, {    543,10}, {   1087,12}, \
    {    287,11}, {    575,10}, {   1151,11}, {    607,10}, \
    {   1215,12}, {    319,11}, {    671,10}, {   1343,12}, \
    {    351,11}, {    703,10}, {   1407,13}, {    191,12}, \
    {    383,11}, {    767,10}, {   1599,12}, {    415,11}, \
    {    831,10}, {   1663,12}, {    447,11}, {    895,14}, \
    {    127,13}, {    255,12}, {    511,11}, {   1023,12}, \
    {    543,11}, {   1087,10}, {   2175,12}, {    575,11}, \
    {   1151,12}, {    607,11}, {   1215,13}, {    319,12}, \
    {    639,11}, {   1343,12}, {    703,11}, {   1407,12}, \
    {    735,11}, {   1471,13}, {    383,12}, {    767,11}, \
    {   1535,12}, {    831,11}, {   1663,13}, {    447,12}, \
    {    895,11}, {   1791,12}, {    959,11}, {   1919,13}, \
    {    511,12}, {   1023,11}, {   2047,12}, {   1087,11}, \
    {   2175,13}, {    575,12}, {   1215,13}, {    639,12}, \
    {   1343,13}, {    703,12}, {   1471,13}, {    767,12}, \
    {   1535,13}, {    831,12}, {   1663,13}, {    895,12}, \
    {   1791,13}, {    959,12}, {   1919,14}, {    511,13}, \
    {   1023,12}, {   2047,13}, {   1087,12}, {   2175,13}, \
    {   1215,14}, {    639,13}, {   1471,14}, {    767,13}, \
    {   1727,14}, {    895,13}, {   1791,12}, {   3583,13}, \
    {   8192,14}, {  16384,15}, {  32768,16}, {  65536,17}, \
    { 131072,18}, { 262144,19}, { 524288,20}, {1048576,21}, \
    {2097152,22}, {4194304,23}, {8388608,24} }
#define MUL_FFT_TABLE3_SIZE 215
#define MUL_FFT_THRESHOLD                 3200

#define SQR_FFT_MODF_THRESHOLD             400  /* k = 5 */
#define SQR_FFT_TABLE3                                      \
  { {    400, 5}, {     21, 6}, {     11, 5}, {     23, 6}, \
    {     12, 5}, {     25, 6}, {     21, 7}, {     11, 6}, \
    {     25, 7}, {     13, 6}, {     27, 7}, {     25, 8}, \
    {     13, 7}, {     28, 8}, {     15, 7}, {     32, 8}, \
    {     17, 7}, {     35, 8}, {     19, 7}, {     39, 8}, \
    {     27, 9}, {     15, 8}, {     35, 9}, {     19, 8}, \
    {     41, 9}, {     23, 8}, {     47, 9}, {     27,10}, \
    {     15, 6}, {    255, 4}, {   1151, 5}, {    607, 7}, \
    {    167, 8}, {     99, 9}, {     55,10}, {     31, 9}, \
    {     75,10}, {     39, 9}, {     87,10}, {     47, 9}, \
    {    103,10}, {     55, 9}, {    111,11}, {     31,10}, \
    {     63, 9}, {    131,10}, {     71, 9}, {    143,10}, \
    {     79,11}, {     47,10}, {    103,12}, {     31,11}, \
    {     63,10}, {    135, 9}, {    271,10}, {    143,11}, \
    {     79,10}, {    159, 9}, {    319,10}, {    167,11}, \
    {     95,10}, {    191, 9}, {    383, 8}, {    767,10}, \
    {    199,11}, {    111,12}, {     63,11}, {    127,10}, \
    {    255, 9}, {    511, 8}, {   1023,11}, {    143, 9}, \
    {    575, 8}, {   1151,11}, {    159,10}, {    319, 9}, \
    {    639, 8}, {   1279,12}, {     95,11}, {    191,10}, \
    {    383, 9}, {    767, 8}, {   1535, 9}, {    799,11}, \
    {    207,10}, {    415, 9}, {    831,13}, {     63,12}, \
    {    127,11}, {    255,10}, {    511, 9}, {   1023,11}, \
    {    271,10}, {    543, 9}, {   1087,10}, {    575, 9}, \
    {   1151,11}, {    303,12}, {    159,10}, {    639, 9}, \
    {   1279,11}, {    335,10}, {    671, 9}, {   1343,11}, \
    {    351,10}, {    703, 9}, {   1471,12}, {    191,11}, \
    {    383,10}, {    767, 9}, {   1535,11}, {    399,10}, \
    {    799,11}, {    415,10}, {    831, 9}, {   1663,12}, \
    {    223,11}, {    447,10}, {    895,13}, {    127,12}, \
    {    255,11}, {    511,10}, {   1023,11}, {    543,10}, \
    {   1087,12}, {    287,11}, {    575,10}, {   1215,11}, \
    {    639,10}, {   1279,11}, {    671,10}, {   1343,11}, \
    {    703,10}, {   1407,11}, {    735,10}, {   1471,13}, \
    {    191,12}, {    383,11}, {    767,10}, {   1535,11}, \
    {    799,10}, {   1599,12}, {    415,11}, {    831,10}, \
    {   1663,11}, {    863,12}, {    447,11}, {    895,10}, \
    {   1791,11}, {    959,14}, {    127,12}, {    511,11}, \
    {   1023,12}, {    543,11}, {   1087,12}, {    575,11}, \
    {   1215,12}, {    639,11}, {   1279,12}, {    671,11}, \
    {   1343,12}, {    703,11}, {   1471,13}, {    383,12}, \
    {    767,11}, {   1535,12}, {    799,11}, {   1599,12}, \
    {    831,11}, {   1663,12}, {    863,13}, {    447,12}, \
    {    895,11}, {   1791,12}, {    959,13}, {    511,12}, \
    {   1023,11}, {   2047,12}, {   1087,13}, {    575,12}, \
    {   1215,13}, {    639,12}, {   1343,13}, {    703,12}, \
    {   1471,13}, {    767,12}, {   1599,13}, {    831,12}, \
    {   1727,13}, {    895,12}, {   1791,13}, {    959,12}, \
    {   1919,14}, {    511,13}, {   1023,12}, {   2047,13}, \
    {   1215,14}, {    639,13}, {   1471,14}, {    767,13}, \
    {   1727,14}, {    895,13}, {   1791,12}, {   3583,13}, \
    {   8192,14}, {  16384,15}, {  32768,16}, {  65536,17}, \
    { 131072,18}, { 262144,19}, { 524288,20}, {1048576,21}, \
    {2097152,22}, {4194304,23}, {8388608,24} }
#define SQR_FFT_TABLE3_SIZE 215
#define SQR_FFT_THRESHOLD                 2880

#define MULLO_BASECASE_THRESHOLD             0  /* always */
#define MULLO_DC_THRESHOLD                  79
#define MULLO_MUL_N_THRESHOLD             6253

#define DC_DIV_QR_THRESHOLD                 54
#define DC_DIVAPPR_Q_THRESHOLD             153
#define DC_BDIV_QR_THRESHOLD                51
#define DC_BDIV_Q_THRESHOLD                 52

#define INV_MULMOD_BNM1_THRESHOLD           52
#define INV_NEWTON_THRESHOLD               150
#define INV_APPR_THRESHOLD                 151

#define BINV_NEWTON_THRESHOLD              232
#define REDC_1_TO_REDC_2_THRESHOLD          13
#define REDC_2_TO_REDC_N_THRESHOLD          55

#define MU_DIV_QR_THRESHOLD               1499
#define MU_DIVAPPR_Q_THRESHOLD            1620
#define MUPI_DIV_QR_THRESHOLD               75
#define MU_BDIV_QR_THRESHOLD              1142
#define MU_BDIV_Q_THRESHOLD               1499

#define POWM_SEC_TABLE  4,29,387,1421

#define MATRIX22_STRASSEN_THRESHOLD         17
#define HGCD_THRESHOLD                     112
#define HGCD_APPR_THRESHOLD                185
#define HGCD_REDUCE_THRESHOLD             3134
#define GCD_DC_THRESHOLD                   492
#define GCDEXT_DC_THRESHOLD                465
#define JACOBI_BASE_METHOD                   4

#define GET_STR_DC_THRESHOLD                11
#define GET_STR_PRECOMPUTE_THRESHOLD        25
#define SET_STR_DC_THRESHOLD               414
#define SET_STR_PRECOMPUTE_THRESHOLD      1945

#define FAC_DSC_THRESHOLD                 1517
#define FAC_ODD_THRESHOLD                   44
