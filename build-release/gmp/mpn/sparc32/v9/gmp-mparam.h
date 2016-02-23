/* SPARC v9 32-bit gmp-mparam.h -- Compiler/machine parameter header file.

Copyright 1991, 1993, 1994, 1999, 2000, 2001, 2002, 2004, 2009, 2010, 2011
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

#define GMP_LIMB_BITS 32
#define BYTES_PER_MP_LIMB 4

#define DIVREM_1_NORM_THRESHOLD              3
#define DIVREM_1_UNNORM_THRESHOLD            4
#define MOD_1_1P_METHOD                      2
#define MOD_1_NORM_THRESHOLD                 3
#define MOD_1_UNNORM_THRESHOLD               4
#define MOD_1N_TO_MOD_1_1_THRESHOLD         11
#define MOD_1U_TO_MOD_1_1_THRESHOLD         11
#define MOD_1_1_TO_MOD_1_2_THRESHOLD         0  /* never mpn_mod_1_1p */
#define MOD_1_2_TO_MOD_1_4_THRESHOLD        22
#define PREINV_MOD_1_TO_MOD_1_THRESHOLD     61
#define USE_PREINV_DIVREM_1                  1
#define DIV_QR_2_PI2_THRESHOLD           MP_SIZE_T_MAX  /* never */
#define DIVEXACT_1_THRESHOLD                 0  /* always */
#define BMOD_1_TO_MOD_1_THRESHOLD        MP_SIZE_T_MAX  /* never */

#define MUL_TOOM22_THRESHOLD                27
#define MUL_TOOM33_THRESHOLD               112
#define MUL_TOOM44_THRESHOLD               124
#define MUL_TOOM6H_THRESHOLD               160
#define MUL_TOOM8H_THRESHOLD               242

#define MUL_TOOM32_TO_TOOM43_THRESHOLD      69
#define MUL_TOOM32_TO_TOOM53_THRESHOLD      93
#define MUL_TOOM42_TO_TOOM53_THRESHOLD      71
#define MUL_TOOM42_TO_TOOM63_THRESHOLD      53
#define MUL_TOOM43_TO_TOOM54_THRESHOLD      70

#define SQR_BASECASE_THRESHOLD               5
#define SQR_TOOM2_THRESHOLD                 64
#define SQR_TOOM3_THRESHOLD                 85
#define SQR_TOOM4_THRESHOLD                158
#define SQR_TOOM6_THRESHOLD                185
#define SQR_TOOM8_THRESHOLD                224

#define MULMID_TOOM42_THRESHOLD             64

#define MULMOD_BNM1_THRESHOLD               11
#define SQRMOD_BNM1_THRESHOLD               16

#define MUL_FFT_MODF_THRESHOLD             212  /* k = 5 */
#define MUL_FFT_TABLE3                                      \
  { {    212, 5}, {     11, 6}, {      7, 5}, {     17, 6}, \
    {      9, 5}, {     20, 6}, {     13, 7}, {      7, 6}, \
    {     16, 7}, {      9, 6}, {     20, 7}, {     13, 8}, \
    {      7, 7}, {     19, 8}, {     11, 7}, {     25, 9}, \
    {      7, 8}, {     15, 7}, {     31, 8}, {     19, 7}, \
    {     39, 8}, {     27, 9}, {     15, 8}, {     39, 9}, \
    {     23,10}, {     15, 9}, {     31, 8}, {     67, 9}, \
    {     39, 8}, {     79, 7}, {    159, 8}, {     83, 7}, \
    {    175, 8}, {     91, 9}, {     47, 8}, {     95,10}, \
    {     31, 9}, {     63, 8}, {    127, 9}, {     71, 8}, \
    {    143, 9}, {     79, 8}, {    159, 9}, {     87,10}, \
    {     47, 9}, {     95,11}, {     31,10}, {     63, 9}, \
    {    127, 8}, {    255, 9}, {    143,10}, {     79, 9}, \
    {    175,10}, {     95, 9}, {    191, 8}, {    415,10}, \
    {    111,11}, {     63,10}, {    127, 9}, {    271,10}, \
    {    143, 9}, {    287, 8}, {    575,10}, {    175,11}, \
    {     95,10}, {    191, 9}, {    415, 8}, {    831,10}, \
    {    223,12}, {     63,11}, {    127,10}, {    287, 9}, \
    {    575,11}, {    159,10}, {    319, 9}, {    639, 8}, \
    {   1407,11}, {    191,10}, {    415, 9}, {    831,11}, \
    {    223,10}, {    447,12}, {   4096,13}, {   8192,14}, \
    {  16384,15}, {  32768,16} }
#define MUL_FFT_TABLE3_SIZE 86
#define MUL_FFT_THRESHOLD                 2688

#define SQR_FFT_MODF_THRESHOLD             180  /* k = 5 */
#define SQR_FFT_TABLE3                                      \
  { {    180, 5}, {      6, 4}, {     13, 5}, {     13, 6}, \
    {      7, 5}, {     15, 6}, {     13, 7}, {      7, 6}, \
    {     17, 7}, {      9, 6}, {     20, 7}, {     11, 6}, \
    {     23, 7}, {     13, 8}, {      7, 7}, {     19, 8}, \
    {     11, 7}, {     25, 9}, {      7, 8}, {     15, 7}, \
    {     31, 8}, {     23, 9}, {     15, 8}, {     39, 9}, \
    {     23,10}, {     15, 9}, {     31, 8}, {     63, 7}, \
    {    127, 9}, {     47,10}, {     31, 9}, {     63, 8}, \
    {    127, 9}, {     71, 8}, {    143, 7}, {    287, 6}, \
    {    575,10}, {     47, 9}, {     95,11}, {     31,10}, \
    {     63, 9}, {    127, 8}, {    255, 9}, {    143,10}, \
    {     79, 9}, {    159, 8}, {    319, 9}, {    175, 8}, \
    {    351, 7}, {    703,10}, {     95, 9}, {    191, 8}, \
    {    383, 9}, {    207,10}, {    111,11}, {     63,10}, \
    {    127, 9}, {    255,10}, {    143, 9}, {    287, 8}, \
    {    575,10}, {    159, 9}, {    319,10}, {    175, 9}, \
    {    351, 8}, {    703,11}, {     95,10}, {    191, 9}, \
    {    383,10}, {    207, 9}, {    415, 8}, {    831,12}, \
    {     63,11}, {    127,10}, {    255, 9}, {    543,10}, \
    {    287, 9}, {    575,11}, {    159,10}, {    319, 9}, \
    {    639,10}, {    351, 9}, {    703, 8}, {   1407,11}, \
    {    191,10}, {    415, 9}, {    831,11}, {    223,10}, \
    {    447, 9}, {    895,12}, {   4096,13}, {   8192,14}, \
    {  16384,15}, {  32768,16} }
#define SQR_FFT_TABLE3_SIZE 94
#define SQR_FFT_THRESHOLD                 1856

#define MULLO_BASECASE_THRESHOLD             0  /* always */
#define MULLO_DC_THRESHOLD                 145
#define MULLO_MUL_N_THRESHOLD             5333

#define DC_DIV_QR_THRESHOLD                 78
#define DC_DIVAPPR_Q_THRESHOLD             414
#define DC_BDIV_QR_THRESHOLD                75
#define DC_BDIV_Q_THRESHOLD                360

#define INV_MULMOD_BNM1_THRESHOLD           52
#define INV_NEWTON_THRESHOLD               351
#define INV_APPR_THRESHOLD                 354

#define BINV_NEWTON_THRESHOLD              234
#define REDC_1_TO_REDC_N_THRESHOLD          60

#define MU_DIV_QR_THRESHOLD                855
#define MU_DIVAPPR_Q_THRESHOLD            1099
#define MUPI_DIV_QR_THRESHOLD              112
#define MU_BDIV_QR_THRESHOLD               839
#define MU_BDIV_Q_THRESHOLD                979

#define POWM_SEC_TABLE  4,23,127,453,1679,2870

#define MATRIX22_STRASSEN_THRESHOLD          9
#define HGCD_THRESHOLD                      87
#define HGCD_APPR_THRESHOLD                126
#define HGCD_REDUCE_THRESHOLD             1679
#define GCD_DC_THRESHOLD                   283
#define GCDEXT_DC_THRESHOLD                189
#define JACOBI_BASE_METHOD                   4

#define GET_STR_DC_THRESHOLD                14
#define GET_STR_PRECOMPUTE_THRESHOLD        28
#define SET_STR_DC_THRESHOLD               262
#define SET_STR_PRECOMPUTE_THRESHOLD       548

#define FAC_DSC_THRESHOLD                  156
#define FAC_ODD_THRESHOLD                   28
