/* AMD Bobcat gmp-mparam.h -- Compiler/machine parameter header file.

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

/* 1600 MHz AMD Bobcat E-350 */

#define MOD_1_NORM_THRESHOLD                 0  /* always */
#define MOD_1_UNNORM_THRESHOLD               0  /* always */
#define MOD_1N_TO_MOD_1_1_THRESHOLD          3
#define MOD_1U_TO_MOD_1_1_THRESHOLD          2
#define MOD_1_1_TO_MOD_1_2_THRESHOLD        39
#define MOD_1_2_TO_MOD_1_4_THRESHOLD         0  /* never mpn_mod_1s_2p */
#define PREINV_MOD_1_TO_MOD_1_THRESHOLD      7
#define USE_PREINV_DIVREM_1                  1  /* native */
#define DIV_QR_2_PI2_THRESHOLD           MP_SIZE_T_MAX  /* never */
#define DIVEXACT_1_THRESHOLD                 0  /* always (native) */
#define BMOD_1_TO_MOD_1_THRESHOLD           17

#define MUL_TOOM22_THRESHOLD                27
#define MUL_TOOM33_THRESHOLD                32
#define MUL_TOOM44_THRESHOLD               272
#define MUL_TOOM6H_THRESHOLD               357
#define MUL_TOOM8H_THRESHOLD                 0  /* always */

#define MUL_TOOM32_TO_TOOM43_THRESHOLD     115
#define MUL_TOOM32_TO_TOOM53_THRESHOLD     172
#define MUL_TOOM42_TO_TOOM53_THRESHOLD     119
#define MUL_TOOM42_TO_TOOM63_THRESHOLD     128
#define MUL_TOOM43_TO_TOOM54_THRESHOLD     160

#define SQR_BASECASE_THRESHOLD               0  /* always (native) */
#define SQR_TOOM2_THRESHOLD                 28
#define SQR_TOOM3_THRESHOLD                 93
#define SQR_TOOM4_THRESHOLD                372
#define SQR_TOOM6_THRESHOLD                  0  /* always */
#define SQR_TOOM8_THRESHOLD                430

#define MULMID_TOOM42_THRESHOLD             24

#define MULMOD_BNM1_THRESHOLD               11
#define SQRMOD_BNM1_THRESHOLD               13

#define MUL_FFT_MODF_THRESHOLD             460  /* k = 5 */
#define MUL_FFT_TABLE3                                      \
  { {    372, 5}, {     17, 6}, {      9, 5}, {     19, 6}, \
    {     10, 5}, {     21, 6}, {     11, 5}, {     23, 6}, \
    {     12, 5}, {     25, 6}, {     25, 7}, {     13, 6}, \
    {     27, 7}, {     15, 6}, {     31, 7}, {     21, 8}, \
    {     11, 7}, {     25, 8}, {     13, 7}, {     28, 8}, \
    {     15, 7}, {     31, 8}, {     17, 7}, {     35, 8}, \
    {     29, 9}, {     15, 8}, {     35, 9}, {     19, 8}, \
    {     41, 9}, {     23, 8}, {     47, 9}, {     27,10}, \
    {     15, 9}, {     39,10}, {     23, 9}, {     55,11}, \
    {     15,10}, {     31, 9}, {     67,10}, {     39, 9}, \
    {     83,10}, {     47, 9}, {     95,10}, {     55,11}, \
    {     31,10}, {     79,11}, {     47,10}, {     95, 8}, \
    {    383,10}, {    111,12}, {     31,11}, {     63,10}, \
    {    143,11}, {     79, 9}, {    319,10}, {    167,11}, \
    {     95,10}, {    191, 9}, {    383,10}, {    207,12}, \
    {     63,11}, {    127,10}, {    255, 9}, {    511,10}, \
    {    271,11}, {    143,10}, {    287, 9}, {    575,10}, \
    {    303,11}, {    159,10}, {    319,12}, {     95,11}, \
    {    191,10}, {    383,11}, {    207,13}, {   8192,14}, \
    {  16384,15}, {  32768,16}, {  65536,17}, { 131072,18}, \
    { 262144,19}, { 524288,20}, {1048576,21}, {2097152,22}, \
    {4194304,23}, {8388608,24} }
#define MUL_FFT_TABLE3_SIZE 86
#define MUL_FFT_THRESHOLD                 5760

#define SQR_FFT_MODF_THRESHOLD             376  /* k = 5 */
#define SQR_FFT_TABLE3                                      \
  { {    340, 5}, {     19, 6}, {     10, 5}, {     21, 6}, \
    {     21, 7}, {     11, 6}, {     23, 7}, {     13, 6}, \
    {     27, 7}, {     21, 8}, {     11, 7}, {     25, 8}, \
    {     13, 7}, {     27, 8}, {     15, 7}, {     31, 8}, \
    {     17, 7}, {     35, 8}, {     19, 7}, {     39, 8}, \
    {     21, 9}, {     11, 8}, {     27, 9}, {     15, 8}, \
    {     35, 9}, {     19, 8}, {     41, 9}, {     23, 8}, \
    {     47, 9}, {     27,10}, {     15, 9}, {     39,10}, \
    {     23, 9}, {     51,11}, {     15,10}, {     31, 9}, \
    {     67,10}, {     39, 9}, {     83,10}, {     47, 9}, \
    {     95,10}, {     55,11}, {     31,10}, {     79,11}, \
    {     47,10}, {     95,12}, {     31,11}, {     63,10}, \
    {    127,11}, {     95,10}, {    191, 9}, {    383,12}, \
    {     63,11}, {    127,10}, {    255, 9}, {    511,10}, \
    {    271, 9}, {    543,10}, {    287, 9}, {    575,10}, \
    {    319,12}, {     95,11}, {    191,10}, {    383,11}, \
    {    207,10}, {    415,13}, {   8192,14}, {  16384,15}, \
    {  32768,16}, {  65536,17}, { 131072,18}, { 262144,19}, \
    { 524288,20}, {1048576,21}, {2097152,22}, {4194304,23}, \
    {8388608,24} }
#define SQR_FFT_TABLE3_SIZE 77
#define SQR_FFT_THRESHOLD                 3648

#define MULLO_BASECASE_THRESHOLD             0  /* always */
#define MULLO_DC_THRESHOLD                  52
#define MULLO_MUL_N_THRESHOLD            11278

#define DC_DIV_QR_THRESHOLD                 71
#define DC_DIVAPPR_Q_THRESHOLD             202
#define DC_BDIV_QR_THRESHOLD                76
#define DC_BDIV_Q_THRESHOLD                151

#define INV_MULMOD_BNM1_THRESHOLD           51
#define INV_NEWTON_THRESHOLD               248
#define INV_APPR_THRESHOLD                 204

#define BINV_NEWTON_THRESHOLD              252
#define REDC_1_TO_REDC_2_THRESHOLD          46
#define REDC_2_TO_REDC_N_THRESHOLD           0  /* always */

#define MU_DIV_QR_THRESHOLD               1470
#define MU_DIVAPPR_Q_THRESHOLD            1589
#define MUPI_DIV_QR_THRESHOLD              122
#define MU_BDIV_QR_THRESHOLD              1334
#define MU_BDIV_Q_THRESHOLD               1442

#define POWM_SEC_TABLE  1,41,322,840,1421

#define MATRIX22_STRASSEN_THRESHOLD         15
#define HGCD_THRESHOLD                      86
#define HGCD_APPR_THRESHOLD                 50
#define HGCD_REDUCE_THRESHOLD             3014
#define GCD_DC_THRESHOLD                   483
#define GCDEXT_DC_THRESHOLD                303
#define JACOBI_BASE_METHOD                   4

#define GET_STR_DC_THRESHOLD                16
#define GET_STR_PRECOMPUTE_THRESHOLD        31
#define SET_STR_DC_THRESHOLD               266
#define SET_STR_PRECOMPUTE_THRESHOLD      1424

#define FAC_DSC_THRESHOLD                  906
#define FAC_ODD_THRESHOLD                   46
