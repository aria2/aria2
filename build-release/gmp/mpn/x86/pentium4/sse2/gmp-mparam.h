/* Intel Pentium-4 gmp-mparam.h -- Compiler/machine parameter header file.

Copyright 1991, 1993, 1994, 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2008,
2009, 2010 Free Software Foundation, Inc.

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


#define MOD_1_NORM_THRESHOLD                24
#define MOD_1_UNNORM_THRESHOLD           MP_SIZE_T_MAX  /* never */
#define MOD_1N_TO_MOD_1_1_THRESHOLD          6
#define MOD_1U_TO_MOD_1_1_THRESHOLD          5
#define MOD_1_1_TO_MOD_1_2_THRESHOLD        13
#define MOD_1_2_TO_MOD_1_4_THRESHOLD         0  /* never mpn_mod_1s_2p */
#define PREINV_MOD_1_TO_MOD_1_THRESHOLD      2
#define USE_PREINV_DIVREM_1                  1  /* native */
#define DIV_QR_2_PI2_THRESHOLD           MP_SIZE_T_MAX  /* never */
#define DIVEXACT_1_THRESHOLD                 0  /* always (native) */
#define BMOD_1_TO_MOD_1_THRESHOLD           20

#define MUL_TOOM22_THRESHOLD                29
#define MUL_TOOM33_THRESHOLD               107
#define MUL_TOOM44_THRESHOLD               276
#define MUL_TOOM6H_THRESHOLD               422
#define MUL_TOOM8H_THRESHOLD               587

#define MUL_TOOM32_TO_TOOM43_THRESHOLD     117
#define MUL_TOOM32_TO_TOOM53_THRESHOLD     207
#define MUL_TOOM42_TO_TOOM53_THRESHOLD     193
#define MUL_TOOM42_TO_TOOM63_THRESHOLD     184
#define MUL_TOOM43_TO_TOOM54_THRESHOLD     164

#define SQR_BASECASE_THRESHOLD               0  /* always (native) */
#define SQR_TOOM2_THRESHOLD                 48
#define SQR_TOOM3_THRESHOLD                173
#define SQR_TOOM4_THRESHOLD                264
#define SQR_TOOM6_THRESHOLD                354
#define SQR_TOOM8_THRESHOLD                915

#define MULMID_TOOM42_THRESHOLD             66

#define MULMOD_BNM1_THRESHOLD               19
#define SQRMOD_BNM1_THRESHOLD               19

#define MUL_FFT_MODF_THRESHOLD            1103  /* k = 5 */
#define MUL_FFT_TABLE3                                      \
  { {    904, 6}, {     15, 5}, {     32, 6}, {     17, 5}, \
    {     35, 6}, {     19, 5}, {     39, 6}, {     29, 7}, \
    {     15, 6}, {     33, 7}, {     17, 6}, {     35, 7}, \
    {     19, 6}, {     41, 7}, {     21, 6}, {     43, 7}, \
    {     23, 6}, {     47, 7}, {     27, 6}, {     55, 7}, \
    {     31, 6}, {     63, 7}, {     43, 8}, {     23, 7}, \
    {     51, 8}, {     27, 7}, {     55, 8}, {     31, 7}, \
    {     63, 8}, {     39, 7}, {     79, 8}, {     43, 9}, \
    {     23, 8}, {     55, 9}, {     31, 8}, {     71, 9}, \
    {     39, 8}, {     79, 9}, {     47, 8}, {     95, 9}, \
    {     55,10}, {     31, 9}, {     63, 8}, {    127, 9}, \
    {     79,10}, {     47, 9}, {    111,11}, {     31,10}, \
    {     63, 9}, {    143,10}, {     79, 9}, {    167,10}, \
    {     95, 9}, {    191,10}, {    111,11}, {     63,10}, \
    {    127, 9}, {    255,10}, {    159, 9}, {    319,10}, \
    {    175,11}, {     95,10}, {    207,12}, {     63,11}, \
    {    127,10}, {    287,11}, {    159,10}, {    319, 9}, \
    {    639,10}, {    351,11}, {    191,10}, {    383,11}, \
    {    223,12}, {   4096,13}, {   8192,14}, {  16384,15}, \
    {  32768,16} }
#define MUL_FFT_TABLE3_SIZE 77
#define MUL_FFT_THRESHOLD                 7808

#define SQR_FFT_MODF_THRESHOLD             824  /* k = 5 */
#define SQR_FFT_TABLE3                                      \
  { {    793, 5}, {     28, 6}, {     15, 5}, {     33, 6}, \
    {     17, 5}, {     35, 6}, {     28, 7}, {     15, 6}, \
    {     33, 7}, {     17, 6}, {     35, 7}, {     19, 6}, \
    {     41, 7}, {     23, 6}, {     47, 7}, {     27, 6}, \
    {     55, 8}, {     15, 7}, {     31, 6}, {     63, 7}, \
    {     37, 8}, {     19, 7}, {     43, 8}, {     23, 7}, \
    {     51, 8}, {     31, 7}, {     63, 8}, {     39, 7}, \
    {     79, 8}, {     43, 9}, {     23, 8}, {     55, 9}, \
    {     31, 8}, {     71, 9}, {     39, 8}, {     79, 9}, \
    {     47, 8}, {     95, 9}, {     55,10}, {     31, 9}, \
    {     79,10}, {     47, 9}, {     95,11}, {     31,10}, \
    {     63, 9}, {    135,10}, {     79, 9}, {    159,10}, \
    {     95, 9}, {    191,10}, {    111,11}, {     63,10}, \
    {    127, 9}, {    255,10}, {    159,11}, {     95,10}, \
    {    191,12}, {     63,11}, {    127,10}, {    255, 9}, \
    {    511,10}, {    271,11}, {    159,10}, {    319, 9}, \
    {    639,11}, {    191,10}, {    399, 9}, {    799,12}, \
    {   4096,13}, {   8192,14}, {  16384,15}, {  32768,16} }
#define SQR_FFT_TABLE3_SIZE 72
#define SQR_FFT_THRESHOLD                 7296

#define MULLO_BASECASE_THRESHOLD            13
#define MULLO_DC_THRESHOLD                  48
#define MULLO_MUL_N_THRESHOLD            14709

#define DC_DIV_QR_THRESHOLD                 38
#define DC_DIVAPPR_Q_THRESHOLD              77
#define DC_BDIV_QR_THRESHOLD                54
#define DC_BDIV_Q_THRESHOLD                 97

#define INV_MULMOD_BNM1_THRESHOLD           57
#define INV_NEWTON_THRESHOLD               202
#define INV_APPR_THRESHOLD                 116

#define BINV_NEWTON_THRESHOLD              327
#define REDC_1_TO_REDC_N_THRESHOLD          34

#define MU_DIV_QR_THRESHOLD               2350
#define MU_DIVAPPR_Q_THRESHOLD            2172
#define MUPI_DIV_QR_THRESHOLD               66
#define MU_BDIV_QR_THRESHOLD              1787
#define MU_BDIV_Q_THRESHOLD               2350

#define POWM_SEC_TABLE  2,35,164,1068,2500

#define MATRIX22_STRASSEN_THRESHOLD         30
#define HGCD_THRESHOLD                      85
#define HGCD_APPR_THRESHOLD                 95
#define HGCD_REDUCE_THRESHOLD             5010
#define GCD_DC_THRESHOLD                   393
#define GCDEXT_DC_THRESHOLD                253
#define JACOBI_BASE_METHOD                   4

#define GET_STR_DC_THRESHOLD                11
#define GET_STR_PRECOMPUTE_THRESHOLD        24
#define SET_STR_DC_THRESHOLD               119
#define SET_STR_PRECOMPUTE_THRESHOLD      1084

#define FAC_DSC_THRESHOLD                  342
#define FAC_ODD_THRESHOLD                   27
