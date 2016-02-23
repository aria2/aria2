/* gmp-mparam.h -- Compiler/machine parameter header file.

Copyright 1991, 1993, 1994, 1999, 2000, 2001, 2002, 2003, 2009, 2010, 2012 Free
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

/* 1700MHz Cortex-A15 */

#define MOD_1_NORM_THRESHOLD                 0  /* always */
#define MOD_1_UNNORM_THRESHOLD               0  /* always */
#define MOD_1N_TO_MOD_1_1_THRESHOLD          3
#define MOD_1U_TO_MOD_1_1_THRESHOLD          3
#define MOD_1_1_TO_MOD_1_2_THRESHOLD         9
#define MOD_1_2_TO_MOD_1_4_THRESHOLD     MP_SIZE_T_MAX
#define PREINV_MOD_1_TO_MOD_1_THRESHOLD      8
#define USE_PREINV_DIVREM_1                  1  /* native */
#define DIV_QR_2_PI2_THRESHOLD           MP_SIZE_T_MAX  /* never */
#define DIVEXACT_1_THRESHOLD                 0  /* always */
#define BMOD_1_TO_MOD_1_THRESHOLD           15

#define MUL_TOOM22_THRESHOLD                31
#define MUL_TOOM33_THRESHOLD               109
#define MUL_TOOM44_THRESHOLD               288
#define MUL_TOOM6H_THRESHOLD               632
#define MUL_TOOM8H_THRESHOLD                 0  /* always */

#define MUL_TOOM32_TO_TOOM43_THRESHOLD     113
#define MUL_TOOM32_TO_TOOM53_THRESHOLD     199
#define MUL_TOOM42_TO_TOOM53_THRESHOLD     189
#define MUL_TOOM42_TO_TOOM63_THRESHOLD     211
#define MUL_TOOM43_TO_TOOM54_THRESHOLD     287

#define SQR_BASECASE_THRESHOLD               0  /* always (native) */
#define SQR_TOOM2_THRESHOLD                 51
#define SQR_TOOM3_THRESHOLD                169
#define SQR_TOOM4_THRESHOLD                662
#define SQR_TOOM6_THRESHOLD                951
#define SQR_TOOM8_THRESHOLD               1005

#define MULMID_TOOM42_THRESHOLD             44

#define MULMOD_BNM1_THRESHOLD               17
#define SQRMOD_BNM1_THRESHOLD               30

#define MUL_FFT_MODF_THRESHOLD             525  /* k = 5 */
#define MUL_FFT_TABLE3                                      \
  { {    505, 5}, {     27, 6}, {     28, 7}, {     15, 6}, \
    {     33, 7}, {     17, 6}, {     35, 7}, {     19, 6}, \
    {     41, 7}, {     21, 8}, {     11, 7}, {     23, 6}, \
    {     47, 7}, {     27, 8}, {     15, 7}, {     35, 8}, \
    {     19, 7}, {     41, 8}, {     23, 7}, {     51, 8}, \
    {     27, 9}, {     15, 8}, {     31, 7}, {     63, 8}, \
    {     43, 9}, {     23, 8}, {     55, 9}, {     31, 8}, \
    {     71, 9}, {     39, 8}, {     79, 9}, {     47, 8}, \
    {     99, 9}, {     55,10}, {     31, 9}, {     79,10}, \
    {     47, 9}, {    103,11}, {     31,10}, {     63, 9}, \
    {    135,10}, {     79, 9}, {    159,10}, {     95, 9}, \
    {    191,10}, {    111,11}, {     63,10}, {    127, 9}, \
    {    255,10}, {    143, 9}, {    287,10}, {    159,11}, \
    {     95,10}, {    191, 9}, {    383,12}, {     63,11}, \
    {    127,10}, {    255, 9}, {    511,10}, {    271, 9}, \
    {    543,10}, {    287,11}, {    159,10}, {    319, 9}, \
    {    639,10}, {    351,11}, {    191,10}, {    383, 9}, \
    {    767,10}, {    415,12}, {    127,11}, {    255,10}, \
    {    543,11}, {    287,10}, {    607,11}, {    319,10}, \
    {    671,11}, {    351,12}, {    191,11}, {    383,10}, \
    {    799,11}, {    415,13}, {    127,12}, {    255,11}, \
    {    543,10}, {   1087,11}, {    607,10}, {   1215,12}, \
    {    319,11}, {    735,12}, {    383,11}, {    799,10}, \
    {   1599,11}, {    831,12}, {    447,11}, {    959,13}, \
    {    255,12}, {    511,11}, {   1087,12}, {    575,11}, \
    {   1215,12}, {    703,13}, {    383,12}, {    959,14}, \
    {    255,13}, {    511,12}, {   1215,13}, {   8192,14}, \
    {  16384,15}, {  32768,16} }
#define MUL_FFT_TABLE3_SIZE 110
#define MUL_FFT_THRESHOLD                 5760

#define SQR_FFT_MODF_THRESHOLD             535  /* k = 5 */
#define SQR_FFT_TABLE3                                      \
  { {    535, 5}, {     27, 6}, {     16, 5}, {     33, 6}, \
    {     29, 7}, {     15, 6}, {     33, 7}, {     17, 6}, \
    {     35, 7}, {     19, 6}, {     41, 7}, {     21, 6}, \
    {     43, 8}, {     11, 6}, {     45, 7}, {     23, 6}, \
    {     47, 7}, {     25, 6}, {     51, 7}, {     27, 6}, \
    {     55, 7}, {     29, 8}, {     15, 7}, {     35, 8}, \
    {     19, 7}, {     43, 8}, {     23, 7}, {     51, 8}, \
    {     27, 9}, {     15, 8}, {     31, 7}, {     63, 8}, \
    {     35, 7}, {     71, 8}, {     43, 9}, {     23, 8}, \
    {     55, 9}, {     31, 8}, {     71, 9}, {     39, 8}, \
    {     83, 9}, {     47, 8}, {     95, 9}, {     55,10}, \
    {     31, 9}, {     79,10}, {     47, 9}, {    103,11}, \
    {     31,10}, {     63, 9}, {    135,10}, {     79, 9}, \
    {    159,10}, {     95, 9}, {    191,10}, {    111,11}, \
    {     63,10}, {    159,11}, {     95,10}, {    191,12}, \
    {     63,11}, {    127,10}, {    255, 9}, {    511,10}, \
    {    271, 9}, {    543,11}, {    159,10}, {    319, 9}, \
    {    639,10}, {    335,11}, {    191,10}, {    383, 9}, \
    {    767,10}, {    415,12}, {    127,11}, {    255,10}, \
    {    511,11}, {    287,10}, {    607,11}, {    319,10}, \
    {    639,12}, {    191,11}, {    383,10}, {    767,11}, \
    {    415,13}, {    127,12}, {    255,11}, {    543,10}, \
    {   1087,11}, {    607,10}, {   1215,12}, {    319,11}, \
    {    735,12}, {    383,11}, {    831,12}, {    447,11}, \
    {    959,13}, {    255,12}, {    511,11}, {   1087,12}, \
    {    575,11}, {   1215,12}, {    703,13}, {    383,12}, \
    {    959,14}, {    255,13}, {    511,12}, {   1215,13}, \
    {   8192,14}, {  16384,15}, {  32768,16} }
#define SQR_FFT_TABLE3_SIZE 111
#define SQR_FFT_THRESHOLD                 4928

#define MULLO_BASECASE_THRESHOLD             0  /* always */
#define MULLO_DC_THRESHOLD                  27
#define MULLO_MUL_N_THRESHOLD             8907

#define DC_DIV_QR_THRESHOLD                 31
#define DC_DIVAPPR_Q_THRESHOLD              45
#define DC_BDIV_QR_THRESHOLD                29
#define DC_BDIV_Q_THRESHOLD                 50

#define INV_MULMOD_BNM1_THRESHOLD           66
#define INV_NEWTON_THRESHOLD               171
#define INV_APPR_THRESHOLD                  65

#define BINV_NEWTON_THRESHOLD              300
#define REDC_1_TO_REDC_2_THRESHOLD          12
#define REDC_2_TO_REDC_N_THRESHOLD          99

#define MU_DIV_QR_THRESHOLD               1895
#define MU_DIVAPPR_Q_THRESHOLD            1895
#define MUPI_DIV_QR_THRESHOLD               54
#define MU_BDIV_QR_THRESHOLD              1470
#define MU_BDIV_Q_THRESHOLD               1895

#define POWM_SEC_TABLE  6,44,548,1604

#define MATRIX22_STRASSEN_THRESHOLD         22
#define HGCD_THRESHOLD                      40
#define HGCD_APPR_THRESHOLD                 50
#define HGCD_REDUCE_THRESHOLD             3389
#define GCD_DC_THRESHOLD                   278
#define GCDEXT_DC_THRESHOLD                180
#define JACOBI_BASE_METHOD                   4

#define GET_STR_DC_THRESHOLD                18
#define GET_STR_PRECOMPUTE_THRESHOLD        34
#define SET_STR_DC_THRESHOLD               198
#define SET_STR_PRECOMPUTE_THRESHOLD       541

#define FAC_DSC_THRESHOLD                  303
#define FAC_ODD_THRESHOLD                   28
