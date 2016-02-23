/* PowerPC-32 gmp-mparam.h -- Compiler/machine parameter header file.

Copyright 1991, 1993, 1994, 1999, 2000, 2001, 2002, 2003, 2004, 2008, 2009,
2010, 2011 Free Software Foundation, Inc.

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

/* 1800 MHz PowerPC-970 */

#define DIVREM_1_NORM_THRESHOLD              0  /* always */
#define DIVREM_1_UNNORM_THRESHOLD            0  /* always */
#define MOD_1_1P_METHOD                      1
#define MOD_1_NORM_THRESHOLD                 0  /* always */
#define MOD_1_UNNORM_THRESHOLD               0  /* always */
#define MOD_1N_TO_MOD_1_1_THRESHOLD          7
#define MOD_1U_TO_MOD_1_1_THRESHOLD          6
#define MOD_1_1_TO_MOD_1_2_THRESHOLD         9
#define MOD_1_2_TO_MOD_1_4_THRESHOLD        42
#define PREINV_MOD_1_TO_MOD_1_THRESHOLD     14
#define USE_PREINV_DIVREM_1                  1
#define DIVEXACT_1_THRESHOLD                 0  /* always */
#define BMOD_1_TO_MOD_1_THRESHOLD           46

#define MUL_TOOM22_THRESHOLD                20
#define MUL_TOOM33_THRESHOLD                73
#define MUL_TOOM44_THRESHOLD               121
#define MUL_TOOM6H_THRESHOLD               222
#define MUL_TOOM8H_THRESHOLD               363

#define MUL_TOOM32_TO_TOOM43_THRESHOLD      84
#define MUL_TOOM32_TO_TOOM53_THRESHOLD     107
#define MUL_TOOM42_TO_TOOM53_THRESHOLD      81
#define MUL_TOOM42_TO_TOOM63_THRESHOLD      88

#define SQR_BASECASE_THRESHOLD               0  /* always */
#define SQR_TOOM2_THRESHOLD                 30
#define SQR_TOOM3_THRESHOLD                 74
#define SQR_TOOM4_THRESHOLD                160
#define SQR_TOOM6_THRESHOLD                222
#define SQR_TOOM8_THRESHOLD                357

#define MULMOD_BNM1_THRESHOLD               16
#define SQRMOD_BNM1_THRESHOLD               18

#define MUL_FFT_MODF_THRESHOLD             444  /* k = 5 */
#define MUL_FFT_TABLE3                                      \
  { {    444, 5}, {     15, 6}, {      8, 5}, {     17, 6}, \
    {      9, 5}, {     19, 6}, {     10, 5}, {     21, 6}, \
    {     21, 7}, {     11, 6}, {     24, 7}, {     13, 6}, \
    {     27, 7}, {     15, 6}, {     31, 7}, {     21, 8}, \
    {     11, 7}, {     27, 8}, {     15, 7}, {     33, 8}, \
    {     19, 7}, {     39, 8}, {     23, 7}, {     47, 8}, \
    {     27, 9}, {     15, 8}, {     39, 9}, {     23, 8}, \
    {     51,10}, {     15, 9}, {     31, 8}, {     67, 9}, \
    {     39, 8}, {     79, 9}, {     47, 8}, {     95, 9}, \
    {     55,10}, {     31, 9}, {     79,10}, {     47, 9}, \
    {     95,11}, {     31,10}, {     63, 9}, {    135,10}, \
    {     79, 9}, {    159, 8}, {    319,10}, {     95, 8}, \
    {    383,10}, {    111,11}, {     63,10}, {    127, 9}, \
    {    255, 8}, {    511,10}, {    143, 9}, {    287, 8}, \
    {    575, 9}, {    303,10}, {    159, 9}, {    319,11}, \
    {     95, 9}, {    383,12}, {     63,11}, {    127,10}, \
    {    255, 9}, {    511,10}, {    271, 9}, {    543, 8}, \
    {   1087,10}, {    287, 9}, {    575,10}, {    303,11}, \
    {    159,10}, {    319, 9}, {    639,10}, {    335, 9}, \
    {    671,10}, {    351, 9}, {    703, 8}, {   1407,10}, \
    {    383, 9}, {    767,10}, {    415, 9}, {    831,11}, \
    {    223,10}, {    447,12}, {   4096,13}, {   8192,14}, \
    {  16384,15}, {  32768,16} }
#define MUL_FFT_TABLE3_SIZE 90
#define MUL_FFT_THRESHOLD                 4736

#define SQR_FFT_MODF_THRESHOLD             308  /* k = 5 */
#define SQR_FFT_TABLE3                                      \
  { {    308, 5}, {     15, 6}, {      8, 5}, {     19, 6}, \
    {     10, 5}, {     21, 6}, {     21, 7}, {     11, 6}, \
    {     24, 7}, {     13, 6}, {     27, 7}, {     15, 6}, \
    {     31, 7}, {     21, 8}, {     11, 7}, {     27, 8}, \
    {     15, 7}, {     33, 8}, {     19, 7}, {     39, 8}, \
    {     27, 9}, {     15, 8}, {     39, 9}, {     23, 8}, \
    {     47,10}, {     15, 9}, {     31, 8}, {     63, 9}, \
    {     39, 8}, {     79, 9}, {     47,10}, {     31, 9}, \
    {     79,10}, {     47, 9}, {     95,11}, {     31,10}, \
    {     63, 9}, {    127, 8}, {    255, 9}, {    135,10}, \
    {     79, 9}, {    159, 8}, {    319, 9}, {    175,10}, \
    {     95, 9}, {    191, 8}, {    383, 9}, {    207,11}, \
    {     63,10}, {    127, 9}, {    255, 8}, {    511, 9}, \
    {    271,10}, {    143, 9}, {    287, 8}, {    575,10}, \
    {    159, 9}, {    319,10}, {    175,11}, {     95,10}, \
    {    191, 9}, {    383,10}, {    207,12}, {     63,11}, \
    {    127,10}, {    255, 9}, {    511,10}, {    271, 9}, \
    {    543, 8}, {   1087,10}, {    287, 9}, {    575,11}, \
    {    159,10}, {    319, 9}, {    639,10}, {    351, 9}, \
    {    703,11}, {    191,10}, {    383, 9}, {    767,10}, \
    {    415, 9}, {    831,11}, {    223,10}, {    447,12}, \
    {   4096,13}, {   8192,14}, {  16384,15}, {  32768,16} }
#define SQR_FFT_TABLE3_SIZE 88
#define SQR_FFT_THRESHOLD                 3520

#define MULLO_BASECASE_THRESHOLD             0  /* always */
#define MULLO_DC_THRESHOLD                  62
#define MULLO_MUL_N_THRESHOLD             8907

#define DC_DIV_QR_THRESHOLD                 53
#define DC_DIVAPPR_Q_THRESHOLD             216
#define DC_BDIV_QR_THRESHOLD                67
#define DC_BDIV_Q_THRESHOLD                180

#define INV_MULMOD_BNM1_THRESHOLD           58
#define INV_NEWTON_THRESHOLD               226
#define INV_APPR_THRESHOLD                 228

#define BINV_NEWTON_THRESHOLD              252
#define REDC_1_TO_REDC_N_THRESHOLD          67

#define MU_DIV_QR_THRESHOLD               1187
#define MU_DIVAPPR_Q_THRESHOLD            1308
#define MUPI_DIV_QR_THRESHOLD              114
#define MU_BDIV_QR_THRESHOLD              1017
#define MU_BDIV_Q_THRESHOLD               1187

#define MATRIX22_STRASSEN_THRESHOLD         15
#define HGCD_THRESHOLD                      97
#define GCD_DC_THRESHOLD                   386
#define GCDEXT_DC_THRESHOLD                298
#define JACOBI_BASE_METHOD                   4

#define GET_STR_DC_THRESHOLD                11
#define GET_STR_PRECOMPUTE_THRESHOLD        24
#define SET_STR_DC_THRESHOLD               318
#define SET_STR_PRECOMPUTE_THRESHOLD       929
