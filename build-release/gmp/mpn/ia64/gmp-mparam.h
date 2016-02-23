/* gmp-mparam.h -- Compiler/machine parameter header file.

Copyright 2000, 2001, 2002, 2003, 2004, 2005, 2009, 2010, 2011 Free Software
Foundation, Inc.

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

/* 900MHz Itanium2 (titanic.gmplib.org) */

#define MOD_1_1P_METHOD                      2
#define MOD_1_NORM_THRESHOLD                 0  /* always */
#define MOD_1_UNNORM_THRESHOLD               0  /* always */
#define MOD_1N_TO_MOD_1_1_THRESHOLD          3
#define MOD_1U_TO_MOD_1_1_THRESHOLD          5
#define MOD_1_1_TO_MOD_1_2_THRESHOLD        26
#define MOD_1_2_TO_MOD_1_4_THRESHOLD         0  /* never mpn_mod_1s_2p */
#define PREINV_MOD_1_TO_MOD_1_THRESHOLD     10
#define USE_PREINV_DIVREM_1                  1  /* native */
#define DIV_QR_2_PI2_THRESHOLD              12
#define DIVEXACT_1_THRESHOLD                 0  /* always (native) */
#define BMOD_1_TO_MOD_1_THRESHOLD        MP_SIZE_T_MAX  /* never */

#define MUL_TOOM22_THRESHOLD                36
#define MUL_TOOM33_THRESHOLD               129
#define MUL_TOOM44_THRESHOLD               214
#define MUL_TOOM6H_THRESHOLD               318
#define MUL_TOOM8H_THRESHOLD               430

#define MUL_TOOM32_TO_TOOM43_THRESHOLD     121
#define MUL_TOOM32_TO_TOOM53_THRESHOLD     138
#define MUL_TOOM42_TO_TOOM53_THRESHOLD     121
#define MUL_TOOM42_TO_TOOM63_THRESHOLD     145
#define MUL_TOOM43_TO_TOOM54_THRESHOLD     203

#define SQR_BASECASE_THRESHOLD              11
#define SQR_TOOM2_THRESHOLD                 84
#define SQR_TOOM3_THRESHOLD                131
#define SQR_TOOM4_THRESHOLD                494
#define SQR_TOOM6_THRESHOLD                  0  /* always */
#define SQR_TOOM8_THRESHOLD                  0  /* always */

#define MULMID_TOOM42_THRESHOLD             98

#define MULMOD_BNM1_THRESHOLD               21
#define SQRMOD_BNM1_THRESHOLD               25

#define MUL_FFT_MODF_THRESHOLD             468  /* k = 5 */
#define MUL_FFT_TABLE3                                      \
  { {    476, 5}, {     27, 6}, {     14, 5}, {     29, 6}, \
    {     33, 7}, {     17, 6}, {     37, 7}, {     19, 6}, \
    {     39, 7}, {     21, 6}, {     43, 7}, {     33, 8}, \
    {     17, 7}, {     37, 8}, {     19, 7}, {     39, 8}, \
    {     21, 7}, {     43, 8}, {     37, 9}, {     19, 8}, \
    {     43, 9}, {     23, 8}, {     51, 9}, {     27, 8}, \
    {     57, 9}, {     31, 8}, {     63, 9}, {     43,10}, \
    {     23, 9}, {     59,10}, {     31, 9}, {     71,10}, \
    {     39, 9}, {     83,10}, {     47, 9}, {     99,10}, \
    {     55,11}, {     31,10}, {     87,11}, {     47,10}, \
    {    111,12}, {     31,11}, {     63,10}, {    143,11}, \
    {     79,10}, {    167,11}, {     95,10}, {    191,11}, \
    {    111,12}, {     63,11}, {    143,10}, {    287, 9}, \
    {    575,10}, {    303,11}, {    159,10}, {    319,12}, \
    {     95,11}, {    191,10}, {    399,11}, {    207,10}, \
    {    431,13}, {     63,12}, {    127,11}, {    271,10}, \
    {    543,11}, {    287,10}, {    575,11}, {    303,12}, \
    {    159,11}, {    335,10}, {    671,11}, {    367,12}, \
    {    191,11}, {    399,10}, {    799,11}, {    431,12}, \
    {    223,11}, {    447,13}, {    127,12}, {    255,11}, \
    {    543,12}, {    287,11}, {    607,12}, {    319,11}, \
    {    671,12}, {    351,11}, {    703,13}, {    191,12}, \
    {    415,11}, {    863,12}, {    447,14}, {    127,13}, \
    {    255,12}, {    607,13}, {    319,12}, {    735,13}, \
    {    383,12}, {    799,11}, {   1599,12}, {    863,13}, \
    {    447,12}, {    927,11}, {   1855,14}, {    255,13}, \
    {    511,12}, {   1055,13}, {    575,12}, {   1215,13}, \
    {    639,12}, {   1279,13}, {    703,14}, {    383,13}, \
    {    767,12}, {   1535,13}, {    831,12}, {   1663,13}, \
    {    895,12}, {   1791,15}, {    255,14}, {    511,13}, \
    {   1087,12}, {   2175,13}, {   1215,14}, {    639,13}, \
    {   1343,12}, {   2687,13}, {   1471,14}, {    767,13}, \
    {   1599,12}, {   3199,13}, {   1663,14}, {    895,13}, \
    {   1855,15}, {    511,14}, {   1023,13}, {   2175,14}, \
    {   1151,13}, {   2431,14}, {   1279,13}, {   2687,14}, \
    {   1407,15}, {    767,14}, {   1535,13}, {   3199,14}, \
    {   1663,13}, {   3455,14}, {   1791,16}, {  65536,17}, \
    { 131072,18}, { 262144,19}, { 524288,20}, {1048576,21}, \
    {2097152,22}, {4194304,23}, {8388608,24} }
#define MUL_FFT_TABLE3_SIZE 155
#define MUL_FFT_THRESHOLD                 6272

#define SQR_FFT_MODF_THRESHOLD             440  /* k = 5 */
#define SQR_FFT_TABLE3                                      \
  { {    436, 5}, {     14, 4}, {     29, 5}, {     31, 6}, \
    {     35, 7}, {     18, 6}, {     37, 7}, {     37, 8}, \
    {     19, 7}, {     40, 8}, {     37, 9}, {     19, 8}, \
    {     43, 9}, {     23, 8}, {     49, 9}, {     27, 8}, \
    {     57, 9}, {     43,10}, {     23, 9}, {     55,10}, \
    {     31, 9}, {     71,10}, {     39, 9}, {     83,10}, \
    {     47, 9}, {     99,10}, {     55,11}, {     31,10}, \
    {     87,11}, {     47,10}, {    111,12}, {     31,11}, \
    {     63,10}, {    135,11}, {     79,10}, {    167,11}, \
    {     95,10}, {    191,11}, {    111,12}, {     63,11}, \
    {    127,10}, {    255,11}, {    143,10}, {    287, 9}, \
    {    575,10}, {    303,11}, {    159,10}, {    319,12}, \
    {     95,11}, {    191,10}, {    399,11}, {    207,10}, \
    {    431,13}, {     63,12}, {    127,11}, {    271,10}, \
    {    543,11}, {    303,12}, {    159,11}, {    335,10}, \
    {    671,11}, {    367,10}, {    735,12}, {    191,11}, \
    {    399,10}, {    799,11}, {    431,12}, {    223,11}, \
    {    463,13}, {    127,12}, {    255,11}, {    543,12}, \
    {    287,11}, {    607,12}, {    319,11}, {    671,12}, \
    {    351,11}, {    735,13}, {    191,12}, {    383,11}, \
    {    799,12}, {    415,11}, {    863,12}, {    447,11}, \
    {    895,14}, {    127,13}, {    255,12}, {    543,11}, \
    {   1087,12}, {    607,13}, {    319,12}, {    735,13}, \
    {    383,12}, {    863,13}, {    447,12}, {    959,14}, \
    {    255,13}, {    511,12}, {   1087,13}, {    575,12}, \
    {   1183,13}, {    639,12}, {   1279,13}, {    703,12}, \
    {   1407,14}, {    383,13}, {    767,12}, {   1535,13}, \
    {    831,12}, {   1663,13}, {    895,12}, {   1791,13}, \
    {    959,15}, {    255,14}, {    511,13}, {   1087,12}, \
    {   2175,13}, {   1215,14}, {    639,13}, {   1343,12}, \
    {   2687,13}, {   1471,14}, {    767,13}, {   1663,14}, \
    {    895,13}, {   1919,15}, {    511,14}, {   1023,13}, \
    {   2175,14}, {   1151,13}, {   2431,14}, {   1279,13}, \
    {   2687,14}, {   1407,15}, {    767,14}, {   1535,13}, \
    {   3199,14}, {   1663,13}, {   3455,14}, {   1791,13}, \
    {   8192,14}, {  16384,15}, {  32768,16}, {  65536,17}, \
    { 131072,18}, { 262144,19}, { 524288,20}, {1048576,21}, \
    {2097152,22}, {4194304,23}, {8388608,24} }
#define SQR_FFT_TABLE3_SIZE 151
#define SQR_FFT_THRESHOLD                 4032

#define MULLO_BASECASE_THRESHOLD             0  /* always */
#define MULLO_DC_THRESHOLD                  62
#define MULLO_MUL_N_THRESHOLD            12322

#define DC_DIV_QR_THRESHOLD                 55
#define DC_DIVAPPR_Q_THRESHOLD             220
#define DC_BDIV_QR_THRESHOLD                92
#define DC_BDIV_Q_THRESHOLD                252

#define INV_MULMOD_BNM1_THRESHOLD           70
#define INV_NEWTON_THRESHOLD               156
#define INV_APPR_THRESHOLD                 154

#define BINV_NEWTON_THRESHOLD              248
#define REDC_1_TO_REDC_2_THRESHOLD           0  /* always */
#define REDC_2_TO_REDC_N_THRESHOLD         149

#define MU_DIV_QR_THRESHOLD               1142
#define MU_DIVAPPR_Q_THRESHOLD            1142
#define MUPI_DIV_QR_THRESHOLD                0  /* always */
#define MU_BDIV_QR_THRESHOLD              1142
#define MU_BDIV_Q_THRESHOLD               1470

#define POWM_SEC_TABLE  2,29,298,1897

#define MATRIX22_STRASSEN_THRESHOLD         19
#define HGCD_THRESHOLD                     115
#define HGCD_APPR_THRESHOLD                181
#define HGCD_REDUCE_THRESHOLD             3014
#define GCD_DC_THRESHOLD                   555
#define GCDEXT_DC_THRESHOLD                368
#define JACOBI_BASE_METHOD                   4

#define GET_STR_DC_THRESHOLD                13
#define GET_STR_PRECOMPUTE_THRESHOLD        21
#define SET_STR_DC_THRESHOLD              1216
#define SET_STR_PRECOMPUTE_THRESHOLD      3170

#define FAC_DSC_THRESHOLD                  746
#define FAC_ODD_THRESHOLD                    0  /* always */
