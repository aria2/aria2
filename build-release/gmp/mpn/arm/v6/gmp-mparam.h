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

/* 700MHz ARM11 (raspberry pi) */

#define DIVREM_1_NORM_THRESHOLD              0  /* preinv always */
#define DIVREM_1_UNNORM_THRESHOLD            0  /* always */
#define MOD_1_1P_METHOD                      2
#define MOD_1_NORM_THRESHOLD                 0  /* always */
#define MOD_1_UNNORM_THRESHOLD               0  /* always */
#define MOD_1N_TO_MOD_1_1_THRESHOLD          3
#define MOD_1U_TO_MOD_1_1_THRESHOLD          7
#define MOD_1_1_TO_MOD_1_2_THRESHOLD         0  /* never mpn_mod_1_1p */
#define MOD_1_2_TO_MOD_1_4_THRESHOLD     MP_SIZE_T_MAX
#define PREINV_MOD_1_TO_MOD_1_THRESHOLD     29
#define USE_PREINV_DIVREM_1                  1  /* preinv always */
#define DIV_QR_2_PI2_THRESHOLD           MP_SIZE_T_MAX  /* never */
#define DIVEXACT_1_THRESHOLD                 0  /* always */
#define BMOD_1_TO_MOD_1_THRESHOLD           33

#define MUL_TOOM22_THRESHOLD                36
#define MUL_TOOM33_THRESHOLD               117
#define MUL_TOOM44_THRESHOLD               462
#define MUL_TOOM6H_THRESHOLD                 0  /* always */
#define MUL_TOOM8H_THRESHOLD               620

#define MUL_TOOM32_TO_TOOM43_THRESHOLD     130
#define MUL_TOOM32_TO_TOOM53_THRESHOLD     573
#define MUL_TOOM42_TO_TOOM53_THRESHOLD     209
#define MUL_TOOM42_TO_TOOM63_THRESHOLD     209
#define MUL_TOOM43_TO_TOOM54_THRESHOLD     305

#define SQR_BASECASE_THRESHOLD               0  /* always (native) */
#define SQR_TOOM2_THRESHOLD                 50
#define SQR_TOOM3_THRESHOLD                181
#define SQR_TOOM4_THRESHOLD                686
#define SQR_TOOM6_THRESHOLD                  0  /* always */
#define SQR_TOOM8_THRESHOLD                915

#define MULMID_TOOM42_THRESHOLD             72

#define MULMOD_BNM1_THRESHOLD               25
#define SQRMOD_BNM1_THRESHOLD               30

#define MUL_FFT_MODF_THRESHOLD             476  /* k = 5 */
#define MUL_FFT_TABLE3                                      \
  { {    476, 5}, {     21, 6}, {     11, 5}, {     25, 6}, \
    {     13, 5}, {     27, 6}, {     25, 7}, {     13, 6}, \
    {     28, 7}, {     15, 6}, {     32, 7}, {     17, 6}, \
    {     35, 7}, {     19, 6}, {     39, 7}, {     23, 6}, \
    {     47, 7}, {     29, 8}, {     15, 7}, {     35, 8}, \
    {     19, 7}, {     43, 8}, {     23, 7}, {     51, 8}, \
    {     27, 7}, {     55, 8}, {     31, 7}, {     63, 8}, \
    {     43, 9}, {     23, 8}, {     55, 9}, {     31, 8}, \
    {     71, 9}, {     39, 8}, {     83, 9}, {     47, 8}, \
    {     95, 9}, {     55,10}, {     31, 9}, {     79,10}, \
    {     47, 9}, {    103,11}, {     31,10}, {     63, 9}, \
    {    135,10}, {     79, 9}, {    159,10}, {     95, 9}, \
    {    191,10}, {    111,11}, {     63,10}, {    127, 9}, \
    {    255,10}, {    143, 9}, {    287,10}, {    159,11}, \
    {     95,10}, {    191, 9}, {    383,12}, {   4096,13}, \
    {   8192,14}, {  16384,15}, {  32768,16} }
#define MUL_FFT_TABLE3_SIZE 63
#define MUL_FFT_THRESHOLD                 4736

#define SQR_FFT_MODF_THRESHOLD             464  /* k = 5 */
#define SQR_FFT_TABLE3                                      \
  { {    464, 5}, {     25, 6}, {     13, 5}, {     27, 6}, \
    {     29, 7}, {     15, 6}, {     33, 7}, {     17, 6}, \
    {     36, 7}, {     19, 6}, {     39, 7}, {     23, 6}, \
    {     47, 7}, {     29, 8}, {     15, 7}, {     35, 8}, \
    {     19, 7}, {     43, 8}, {     23, 7}, {     49, 8}, \
    {     27, 7}, {     55, 8}, {     31, 7}, {     63, 8}, \
    {     35, 7}, {     71, 8}, {     43, 9}, {     23, 8}, \
    {     55, 9}, {     31, 8}, {     71, 9}, {     39, 8}, \
    {     83, 9}, {     47, 8}, {     95, 9}, {     55,10}, \
    {     31, 9}, {     79,10}, {     47, 9}, {    103,11}, \
    {     31,10}, {     63, 9}, {    135,10}, {     79, 9}, \
    {    159,10}, {     95, 9}, {    191,10}, {    111,11}, \
    {     63,10}, {    127, 9}, {    255,10}, {    143, 9}, \
    {    287,10}, {    159,11}, {     95,10}, {    191, 9}, \
    {    383,12}, {   4096,13}, {   8192,14}, {  16384,15}, \
    {  32768,16} }
#define SQR_FFT_TABLE3_SIZE 61
#define SQR_FFT_THRESHOLD                 3776

#define MULLO_BASECASE_THRESHOLD             0  /* always */
#define MULLO_DC_THRESHOLD                  67
#define MULLO_MUL_N_THRESHOLD             8907

#define DC_DIV_QR_THRESHOLD                 40
#define DC_DIVAPPR_Q_THRESHOLD             156
#define DC_BDIV_QR_THRESHOLD                71
#define DC_BDIV_Q_THRESHOLD                208

#define INV_MULMOD_BNM1_THRESHOLD           70
#define INV_NEWTON_THRESHOLD               151
#define INV_APPR_THRESHOLD                 150

#define BINV_NEWTON_THRESHOLD              375
#define REDC_1_TO_REDC_2_THRESHOLD           5
#define REDC_2_TO_REDC_N_THRESHOLD         134

#define MU_DIV_QR_THRESHOLD               2130
#define MU_DIVAPPR_Q_THRESHOLD            2130
#define MUPI_DIV_QR_THRESHOLD               80
#define MU_BDIV_QR_THRESHOLD              1787
#define MU_BDIV_Q_THRESHOLD               2130

#define POWM_SEC_TABLE  7,32,460,1705

#define MATRIX22_STRASSEN_THRESHOLD         19
#define HGCD_THRESHOLD                      85
#define HGCD_APPR_THRESHOLD                119
#define HGCD_REDUCE_THRESHOLD             3389
#define GCD_DC_THRESHOLD                   333
#define GCDEXT_DC_THRESHOLD                309
#define JACOBI_BASE_METHOD                   1

#define GET_STR_DC_THRESHOLD                21
#define GET_STR_PRECOMPUTE_THRESHOLD        41
#define SET_STR_DC_THRESHOLD               527
#define SET_STR_PRECOMPUTE_THRESHOLD      1323

#define FAC_DSC_THRESHOLD                  414
#define FAC_ODD_THRESHOLD                  154
