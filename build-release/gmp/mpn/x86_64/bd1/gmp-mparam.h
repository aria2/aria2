/* AMD Bulldozer-1 gmp-mparam.h -- Compiler/machine parameter header file.

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

#define MOD_1_NORM_THRESHOLD                 0  /* always */
#define MOD_1_UNNORM_THRESHOLD               0  /* always */
#define MOD_1N_TO_MOD_1_1_THRESHOLD          5
#define MOD_1U_TO_MOD_1_1_THRESHOLD          3
#define MOD_1_1_TO_MOD_1_2_THRESHOLD        24
#define MOD_1_2_TO_MOD_1_4_THRESHOLD        34
#define PREINV_MOD_1_TO_MOD_1_THRESHOLD     11
#define USE_PREINV_DIVREM_1                  1  /* native */
#define DIV_QR_2_PI2_THRESHOLD           MP_SIZE_T_MAX  /* never */
#define DIVEXACT_1_THRESHOLD                 0  /* always (native) */
#define BMOD_1_TO_MOD_1_THRESHOLD           20

#define MUL_TOOM22_THRESHOLD                16
#define MUL_TOOM33_THRESHOLD                57
#define MUL_TOOM44_THRESHOLD               154
#define MUL_TOOM6H_THRESHOLD               250
#define MUL_TOOM8H_THRESHOLD               309

#define MUL_TOOM32_TO_TOOM43_THRESHOLD      97
#define MUL_TOOM32_TO_TOOM53_THRESHOLD     108
#define MUL_TOOM42_TO_TOOM53_THRESHOLD     105
#define MUL_TOOM42_TO_TOOM63_THRESHOLD     109
#define MUL_TOOM43_TO_TOOM54_THRESHOLD     143

#define SQR_BASECASE_THRESHOLD               0  /* always (native) */
#define SQR_TOOM2_THRESHOLD                 24
#define SQR_TOOM3_THRESHOLD                139
#define SQR_TOOM4_THRESHOLD                218
#define SQR_TOOM6_THRESHOLD                318
#define SQR_TOOM8_THRESHOLD                434

#define MULMID_TOOM42_THRESHOLD             22

#define MULMOD_BNM1_THRESHOLD               11
#define SQRMOD_BNM1_THRESHOLD               13

#define MUL_FFT_MODF_THRESHOLD             396  /* k = 5 */
#define MUL_FFT_TABLE3                                      \
  { {    380, 5}, {     21, 6}, {     11, 5}, {     23, 6}, \
    {     23, 7}, {     12, 6}, {     25, 7}, {     13, 6}, \
    {     27, 7}, {     15, 6}, {     31, 7}, {     25, 8}, \
    {     13, 7}, {     27, 8}, {     15, 7}, {     32, 8}, \
    {     17, 7}, {     35, 8}, {     21, 9}, {     11, 8}, \
    {     27, 9}, {     15, 8}, {     35, 9}, {     19, 8}, \
    {     41, 9}, {     23, 8}, {     47, 9}, {     27,10}, \
    {     15, 9}, {     31, 8}, {     63, 9}, {     35, 8}, \
    {     71, 9}, {     39,10}, {     23, 9}, {     51,11}, \
    {     15,10}, {     31, 9}, {     71,10}, {     39, 9}, \
    {     87,10}, {     47, 9}, {     99,10}, {     55,11}, \
    {     31,10}, {     87,11}, {     47,10}, {    103,12}, \
    {     31,11}, {     63,10}, {    135,11}, {     79,10}, \
    {    167,11}, {     95,12}, {     63,11}, {    127,10}, \
    {    255,11}, {    143,10}, {    287, 9}, {    575,10}, \
    {    303,11}, {    159,12}, {     95,11}, {    191,13}, \
    {   8192,14}, {  16384,15}, {  32768,16}, {  65536,17}, \
    { 131072,18}, { 262144,19}, { 524288,20}, {1048576,21}, \
    {2097152,22}, {4194304,23}, {8388608,24} }
#define MUL_FFT_TABLE3_SIZE 75
#define MUL_FFT_THRESHOLD                 4736

#define SQR_FFT_MODF_THRESHOLD             340  /* k = 5 */
#define SQR_FFT_TABLE3                                      \
  { {    332, 5}, {     19, 6}, {     10, 5}, {     21, 6}, \
    {     11, 5}, {     23, 6}, {     25, 7}, {     25, 8}, \
    {     13, 7}, {     27, 8}, {     15, 7}, {     31, 8}, \
    {     21, 9}, {     11, 8}, {     27, 9}, {     15, 8}, \
    {     33, 9}, {     19, 8}, {     41, 9}, {     23, 8}, \
    {     47, 9}, {     27,10}, {     15, 9}, {     39,10}, \
    {     23, 9}, {     47,11}, {     15,10}, {     31, 9}, \
    {     67,10}, {     39, 9}, {     79,10}, {     47, 9}, \
    {     95,10}, {     55,11}, {     31,10}, {     79,11}, \
    {     47,10}, {     95,12}, {     31,11}, {     63,10}, \
    {    127, 9}, {    255,10}, {    135,11}, {     79,10}, \
    {    159,11}, {     95,10}, {    191,12}, {     63,11}, \
    {    127,10}, {    255, 9}, {    511,10}, {    271,11}, \
    {    143,10}, {    303,11}, {    159,10}, {    319,12}, \
    {     95,11}, {    191,10}, {    383,11}, {    207,13}, \
    {   8192,14}, {  16384,15}, {  32768,16}, {  65536,17}, \
    { 131072,18}, { 262144,19}, { 524288,20}, {1048576,21}, \
    {2097152,22}, {4194304,23}, {8388608,24} }
#define SQR_FFT_TABLE3_SIZE 71
#define SQR_FFT_THRESHOLD                 3264

#define MULLO_BASECASE_THRESHOLD             0  /* always */
#define MULLO_DC_THRESHOLD                  37
#define MULLO_MUL_N_THRESHOLD             8648

#define DC_DIV_QR_THRESHOLD                 57
#define DC_DIVAPPR_Q_THRESHOLD             204
#define DC_BDIV_QR_THRESHOLD                48
#define DC_BDIV_Q_THRESHOLD                107

#define INV_MULMOD_BNM1_THRESHOLD           30
#define INV_NEWTON_THRESHOLD               228
#define INV_APPR_THRESHOLD                 214

#define BINV_NEWTON_THRESHOLD              248
#define REDC_1_TO_REDC_2_THRESHOLD          51
#define REDC_2_TO_REDC_N_THRESHOLD           0  /* always */

#define MU_DIV_QR_THRESHOLD               1334
#define MU_DIVAPPR_Q_THRESHOLD            1387
#define MUPI_DIV_QR_THRESHOLD              108
#define MU_BDIV_QR_THRESHOLD              1142
#define MU_BDIV_Q_THRESHOLD               1308

#define POWM_SEC_TABLE  2,44,411,580,2246

#define MATRIX22_STRASSEN_THRESHOLD         17
#define HGCD_THRESHOLD                     117
#define HGCD_APPR_THRESHOLD                 50
#define HGCD_REDUCE_THRESHOLD             2681
#define GCD_DC_THRESHOLD                   487
#define GCDEXT_DC_THRESHOLD                318
#define JACOBI_BASE_METHOD                   4

#define GET_STR_DC_THRESHOLD                13
#define GET_STR_PRECOMPUTE_THRESHOLD        20
#define SET_STR_DC_THRESHOLD               418
#define SET_STR_PRECOMPUTE_THRESHOLD      1340

#define FAC_DSC_THRESHOLD                  462
#define FAC_ODD_THRESHOLD                    0  /* always */
