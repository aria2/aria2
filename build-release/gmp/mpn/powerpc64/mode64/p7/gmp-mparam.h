/* POWER7 gmp-mparam.h -- Compiler/machine parameter header file.

Copyright 1991, 1993, 1994, 1999, 2000, 2001, 2002, 2003, 2009, 2010, 2011
Free Software Foundation, Inc.

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

/* 3550 MHz POWER7 (gcc110.fsffrance.org) */

#define MOD_1_NORM_THRESHOLD                 0  /* always */
#define MOD_1_UNNORM_THRESHOLD               0  /* always */
#define MOD_1N_TO_MOD_1_1_THRESHOLD          8
#define MOD_1U_TO_MOD_1_1_THRESHOLD          5
#define MOD_1_1_TO_MOD_1_2_THRESHOLD        12
#define MOD_1_2_TO_MOD_1_4_THRESHOLD        33
#define PREINV_MOD_1_TO_MOD_1_THRESHOLD     17
#define USE_PREINV_DIVREM_1                  0
#define DIV_QR_2_PI2_THRESHOLD           MP_SIZE_T_MAX  /* never */
#define DIVEXACT_1_THRESHOLD                 0  /* always (native) */
#define BMOD_1_TO_MOD_1_THRESHOLD           38

#define MUL_TOOM22_THRESHOLD                22
#define MUL_TOOM33_THRESHOLD                73
#define MUL_TOOM44_THRESHOLD               154
#define MUL_TOOM6H_THRESHOLD               270
#define MUL_TOOM8H_THRESHOLD               369

#define MUL_TOOM32_TO_TOOM43_THRESHOLD      81
#define MUL_TOOM32_TO_TOOM53_THRESHOLD     122
#define MUL_TOOM42_TO_TOOM53_THRESHOLD     105
#define MUL_TOOM42_TO_TOOM63_THRESHOLD     105
#define MUL_TOOM43_TO_TOOM54_THRESHOLD     112

#define SQR_BASECASE_THRESHOLD               0  /* always (native) */
#define SQR_TOOM2_THRESHOLD                 30
#define SQR_TOOM3_THRESHOLD                109
#define SQR_TOOM4_THRESHOLD                178
#define SQR_TOOM6_THRESHOLD                303
#define SQR_TOOM8_THRESHOLD                357

#define MULMID_TOOM42_THRESHOLD             62

#define MULMOD_BNM1_THRESHOLD               16
#define SQRMOD_BNM1_THRESHOLD               18

#define MUL_FFT_MODF_THRESHOLD             444  /* k = 5 */
#define MUL_FFT_TABLE3                                      \
  { {    436, 5}, {     19, 6}, {     10, 5}, {     21, 6}, \
    {     21, 7}, {     11, 6}, {     23, 7}, {     12, 6}, \
    {     25, 7}, {     13, 6}, {     27, 7}, {     15, 6}, \
    {     31, 7}, {     21, 8}, {     11, 7}, {     25, 8}, \
    {     13, 7}, {     28, 8}, {     15, 7}, {     32, 8}, \
    {     17, 7}, {     35, 8}, {     19, 7}, {     39, 8}, \
    {     21, 9}, {     11, 8}, {     29, 9}, {     15, 8}, \
    {     35, 9}, {     19, 8}, {     41, 9}, {     23, 8}, \
    {     47, 9}, {     27,10}, {     15, 9}, {     31, 8}, \
    {     63, 9}, {     43,10}, {     23, 9}, {     51,11}, \
    {     15,10}, {     31, 9}, {     67,10}, {     39, 9}, \
    {     79,10}, {     47, 9}, {     95,10}, {     55,11}, \
    {     31,10}, {     79,11}, {     47,10}, {     95,12}, \
    {     31,11}, {     63,10}, {    135,11}, {     79,10}, \
    {    159,11}, {     95,10}, {    191,11}, {    111,12}, \
    {     63,11}, {    127,10}, {    255,11}, {    143,10}, \
    {    287, 9}, {    575,10}, {    303,11}, {    159,12}, \
    {     95,11}, {    191,10}, {    383,13}, {     63,12}, \
    {    127,11}, {    255,10}, {    511,11}, {    271,10}, \
    {    543, 9}, {   1087,11}, {    287,10}, {    575,11}, \
    {    303,12}, {    159,11}, {    319,10}, {    639,11}, \
    {    335,10}, {    671,11}, {    351,10}, {    703,12}, \
    {    191,11}, {    383,10}, {    767,11}, {    415,10}, \
    {    831,12}, {    223,11}, {    447,13}, {   8192,14}, \
    {  16384,15}, {  32768,16}, {  65536,17}, { 131072,18}, \
    { 262144,19}, { 524288,20}, {1048576,21}, {2097152,22}, \
    {4194304,23}, {8388608,24} }
#define MUL_FFT_TABLE3_SIZE 106
#define MUL_FFT_THRESHOLD                 5248

#define SQR_FFT_MODF_THRESHOLD             380  /* k = 5 */
#define SQR_FFT_TABLE3                                      \
  { {    308, 5}, {     17, 6}, {      9, 5}, {     19, 6}, \
    {     21, 7}, {     11, 6}, {     23, 7}, {     21, 8}, \
    {     11, 7}, {     24, 8}, {     13, 7}, {     27, 8}, \
    {     15, 7}, {     31, 8}, {     21, 9}, {     11, 8}, \
    {     27, 9}, {     15, 8}, {     33, 9}, {     19, 8}, \
    {     41, 9}, {     23, 8}, {     47, 9}, {     27,10}, \
    {     15, 9}, {     39,10}, {     23, 9}, {     47,11}, \
    {     15,10}, {     31, 9}, {     67,10}, {     39, 9}, \
    {     79,10}, {     47,11}, {     31,10}, {     79,11}, \
    {     47,10}, {     95,12}, {     31,11}, {     63,10}, \
    {    127, 9}, {    255,10}, {    135,11}, {     79,10}, \
    {    159, 9}, {    319,11}, {     95,10}, {    191, 9}, \
    {    383,12}, {     63,11}, {    127,10}, {    255, 9}, \
    {    511,10}, {    271, 9}, {    543,11}, {    143,10}, \
    {    287, 9}, {    575,11}, {    159,10}, {    319, 9}, \
    {    639,11}, {    175,12}, {     95,11}, {    191,10}, \
    {    383, 9}, {    767,11}, {    207,13}, {     63,12}, \
    {    127,11}, {    255,10}, {    511,11}, {    271,10}, \
    {    543,11}, {    287,10}, {    575,11}, {    303,12}, \
    {    159,11}, {    319,10}, {    639, 9}, {   1279,10}, \
    {    671,11}, {    351,10}, {    703,12}, {    191,11}, \
    {    383,10}, {    767,11}, {    415,10}, {    831,12}, \
    {    223,11}, {    447,10}, {    895,11}, {    479,13}, \
    {   8192,14}, {  16384,15}, {  32768,16}, {  65536,17}, \
    { 131072,18}, { 262144,19}, { 524288,20}, {1048576,21}, \
    {2097152,22}, {4194304,23}, {8388608,24} }
#define SQR_FFT_TABLE3_SIZE 103
#define SQR_FFT_THRESHOLD                 3712

#define MULLO_BASECASE_THRESHOLD             5
#define MULLO_DC_THRESHOLD                  33
#define MULLO_MUL_N_THRESHOLD            10323

#define DC_DIV_QR_THRESHOLD                 57
#define DC_DIVAPPR_Q_THRESHOLD             185
#define DC_BDIV_QR_THRESHOLD                63
#define DC_BDIV_Q_THRESHOLD                158

#define INV_MULMOD_BNM1_THRESHOLD           58
#define INV_NEWTON_THRESHOLD               212
#define INV_APPR_THRESHOLD                 187

#define BINV_NEWTON_THRESHOLD              276
#define REDC_1_TO_REDC_N_THRESHOLD          63

#define MU_DIV_QR_THRESHOLD               1442
#define MU_DIVAPPR_Q_THRESHOLD            1442
#define MUPI_DIV_QR_THRESHOLD               91
#define MU_BDIV_QR_THRESHOLD              1142
#define MU_BDIV_Q_THRESHOLD               1442

#define POWM_SEC_TABLE  3,38,270,1487

#define MATRIX22_STRASSEN_THRESHOLD         14
#define HGCD_THRESHOLD                     138
#define HGCD_APPR_THRESHOLD                157
#define HGCD_REDUCE_THRESHOLD             2578
#define GCD_DC_THRESHOLD                   573
#define GCDEXT_DC_THRESHOLD                440
#define JACOBI_BASE_METHOD                   4

#define GET_STR_DC_THRESHOLD                14
#define GET_STR_PRECOMPUTE_THRESHOLD        32
#define SET_STR_DC_THRESHOLD              1517
#define SET_STR_PRECOMPUTE_THRESHOLD      3007

#define FAC_DSC_THRESHOLD                  680
#define FAC_ODD_THRESHOLD                   24
