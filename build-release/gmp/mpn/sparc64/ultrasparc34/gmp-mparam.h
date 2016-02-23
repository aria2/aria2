/* ultrasparc3/4 gmp-mparam.h -- Compiler/machine parameter header file.

Copyright 1991, 1993, 1994, 1999, 2000, 2001, 2002, 2004, 2006, 2008, 2009,
2010 Free Software Foundation, Inc.

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

/* 1593 MHz ultrasparc3 running Solaris 10 (swift.nada.kth.se) */

#define DIVREM_1_NORM_THRESHOLD              0  /* always */
#define DIVREM_1_UNNORM_THRESHOLD            0  /* always */
#define MOD_1_1P_METHOD                      2
#define MOD_1_NORM_THRESHOLD                 0  /* always */
#define MOD_1_UNNORM_THRESHOLD               0  /* always */
#define MOD_1N_TO_MOD_1_1_THRESHOLD         10
#define MOD_1U_TO_MOD_1_1_THRESHOLD          5
#define MOD_1_1_TO_MOD_1_2_THRESHOLD        20
#define MOD_1_2_TO_MOD_1_4_THRESHOLD         0  /* never mpn_mod_1s_2p */
#define PREINV_MOD_1_TO_MOD_1_THRESHOLD     29
#define USE_PREINV_DIVREM_1                  1
#define DIV_QR_2_PI2_THRESHOLD           MP_SIZE_T_MAX  /* never */
#define DIVEXACT_1_THRESHOLD                 0  /* always */
#define BMOD_1_TO_MOD_1_THRESHOLD        MP_SIZE_T_MAX  /* never */

#define MUL_TOOM22_THRESHOLD                30
#define MUL_TOOM33_THRESHOLD                93
#define MUL_TOOM44_THRESHOLD               139
#define MUL_TOOM6H_THRESHOLD               165
#define MUL_TOOM8H_THRESHOLD               278

#define MUL_TOOM32_TO_TOOM43_THRESHOLD      86
#define MUL_TOOM32_TO_TOOM53_THRESHOLD     105
#define MUL_TOOM42_TO_TOOM53_THRESHOLD      85
#define MUL_TOOM42_TO_TOOM63_THRESHOLD      68
#define MUL_TOOM43_TO_TOOM54_THRESHOLD      67

#define SQR_BASECASE_THRESHOLD               9
#define SQR_TOOM2_THRESHOLD                 72
#define SQR_TOOM3_THRESHOLD                 94
#define SQR_TOOM4_THRESHOLD                184
#define SQR_TOOM6_THRESHOLD                  0  /* always */
#define SQR_TOOM8_THRESHOLD                339

#define MULMID_TOOM42_THRESHOLD             40

#define MULMOD_BNM1_THRESHOLD               13
#define SQRMOD_BNM1_THRESHOLD                9

#define MUL_FFT_MODF_THRESHOLD             212  /* k = 5 */
#define MUL_FFT_TABLE3                                      \
  { {    212, 5}, {     13, 6}, {     17, 7}, {      9, 6}, \
    {     19, 7}, {     17, 8}, {      9, 7}, {     20, 8}, \
    {     11, 7}, {     24, 8}, {     13, 9}, {      7, 8}, \
    {     19, 9}, {     11, 8}, {     25,10}, {      7, 9}, \
    {     15, 8}, {     33, 9}, {     19, 8}, {     39, 9}, \
    {     23, 8}, {     47, 9}, {     27,10}, {     15, 9}, \
    {     39,10}, {     23, 9}, {     47,11}, {     15,10}, \
    {     31, 9}, {     67,10}, {     39, 9}, {     79, 8}, \
    {    159, 9}, {     83,10}, {     47, 9}, {     95, 8}, \
    {    191, 7}, {    383, 9}, {     99,10}, {     55,11}, \
    {     31,10}, {     63, 9}, {    127, 8}, {    255,10}, \
    {     71, 9}, {    143, 8}, {    287,10}, {     79, 9}, \
    {    159, 8}, {    319,11}, {     47,10}, {     95, 9}, \
    {    191, 8}, {    383,10}, {    103, 9}, {    207, 8}, \
    {    415,10}, {    111,12}, {     31,11}, {     63,10}, \
    {    127, 9}, {    255,10}, {    143, 9}, {    287,11}, \
    {     79,10}, {    159, 9}, {    319, 8}, {    639,10}, \
    {    175, 9}, {    351,11}, {     95,10}, {    191, 9}, \
    {    383,10}, {    207, 9}, {    415,11}, {    111,10}, \
    {    223, 9}, {    447,12}, {     63,11}, {    127,10}, \
    {    255,11}, {    143,10}, {    287, 9}, {    575,11}, \
    {    159,10}, {    319,11}, {    175,10}, {    351,12}, \
    {     95,11}, {    191,10}, {    383,11}, {    207,10}, \
    {    415,11}, {    223,10}, {    447,13}, {     63,12}, \
    {    127,11}, {    287,10}, {    575,12}, {    159,11}, \
    {    351,10}, {    703,12}, {    191,11}, {    415,12}, \
    {    223,11}, {    479,10}, {    959,13}, {    127,12}, \
    {    287,11}, {    575,12}, {    351,13}, {    191,12}, \
    {    479,14}, {    127,13}, {    255,12}, {    575,13}, \
    {    319,12}, {    639,11}, {   1279,12}, {    703,13}, \
    {    383,12}, {    831,13}, {    447,12}, {    895,14}, \
    {    255,13}, {    511,12}, {   1087,13}, {    575,12}, \
    {   1215,13}, {    639,12}, {   1279,13}, {    703,14}, \
    {    383,13}, {    831,12}, {   1663,13}, {    895,15}, \
    {    255,14}, {    511,13}, {   1151,14}, {    639,13}, \
    {   1407,14}, {    767,13}, {   1663,14}, {    895,13}, \
    {   1791,15}, {    511,14}, {   1023,13}, {   2047,14}, \
    {   1151,13}, {   2303,14}, {   1407,15}, {    767,14}, \
    {   1791,16}, {    511,15}, {   1023,14}, {   2303,15}, \
    {   1279,14}, {   2815,15}, {   1535,14}, {   3199,15}, \
    {   1791,14}, {   3583,16}, {  65536,17}, { 131072,18}, \
    { 262144,19}, { 524288,20}, {1048576,21}, {2097152,22}, \
    {4194304,23}, {8388608,24} }
#define MUL_FFT_TABLE3_SIZE 170
#define MUL_FFT_THRESHOLD                 1984

#define SQR_FFT_MODF_THRESHOLD             236  /* k = 5 */
#define SQR_FFT_TABLE3                                      \
  { {    244, 5}, {      8, 4}, {     17, 5}, {     17, 6}, \
    {     17, 7}, {      9, 6}, {     19, 7}, {     17, 8}, \
    {      9, 7}, {     20, 8}, {     11, 7}, {     23, 8}, \
    {     21, 9}, {     11, 8}, {     25, 9}, {     15, 8}, \
    {     31, 9}, {     19, 8}, {     39, 9}, {     27,10}, \
    {     15, 9}, {     39,10}, {     23, 9}, {     47,11}, \
    {     15,10}, {     31, 9}, {     67,10}, {     39, 9}, \
    {     79,10}, {     55,11}, {     31,10}, {     63, 9}, \
    {    127, 8}, {    255,10}, {     71, 9}, {    143, 8}, \
    {    287,10}, {     79, 9}, {    159,11}, {     47,10}, \
    {     95, 9}, {    191, 8}, {    383,12}, {     31,11}, \
    {     63,10}, {    127, 9}, {    255, 8}, {    511,10}, \
    {    135, 9}, {    271,10}, {    143, 9}, {    287,11}, \
    {     79,10}, {    159, 9}, {    319, 8}, {    639,10}, \
    {    175, 9}, {    351, 8}, {    703,11}, {     95,10}, \
    {    191, 9}, {    383, 8}, {    767,10}, {    207, 9}, \
    {    415, 8}, {    831,10}, {    223, 9}, {    447,12}, \
    {     63,11}, {    127,10}, {    271, 9}, {    543,11}, \
    {    143,10}, {    287, 9}, {    575, 8}, {   1151, 9}, \
    {    607,11}, {    159,10}, {    319, 9}, {    639,11}, \
    {    175,10}, {    351, 9}, {    703, 8}, {   1407,12}, \
    {     95,11}, {    191,10}, {    383,11}, {    207,10}, \
    {    415, 9}, {    831,11}, {    223,10}, {    447,13}, \
    {     63,12}, {    127,11}, {    271,10}, {    543,11}, \
    {    287,10}, {    575, 9}, {   1151,10}, {    607,12}, \
    {    159,11}, {    319,10}, {    639,11}, {    351,10}, \
    {    703,12}, {    191,11}, {    415,10}, {    831,12}, \
    {    223,11}, {    479,13}, {    127,12}, {    255,11}, \
    {    543,12}, {    287,11}, {    607,12}, {    319,11}, \
    {    639,12}, {    351,11}, {    703,13}, {    191,12}, \
    {    415,11}, {    831,12}, {    479,11}, {    959,14}, \
    {    127,13}, {    255,12}, {    543,11}, {   1087,12}, \
    {    575,13}, {    319,12}, {    639,11}, {   1279,12}, \
    {    703,13}, {    383,12}, {    831,13}, {    447,12}, \
    {    895,14}, {    255,13}, {    511,12}, {   1023,13}, \
    {    575,12}, {   1151,13}, {    703,14}, {    383,13}, \
    {    831,12}, {   1663,13}, {    959,15}, {    255,14}, \
    {    511,13}, {   1087,12}, {   2175,13}, {   1151,14}, \
    {    639,13}, {   1407,14}, {    767,13}, {   1663,14}, \
    {    895,13}, {   1791,15}, {    511,14}, {   1023,13}, \
    {   2047,14}, {   1151,13}, {   2431,14}, {   1407,15}, \
    {    767,14}, {   1791,16}, {    511,15}, {   1023,14}, \
    {   2303,15}, {   1279,14}, {   2815,15}, {   1535,14}, \
    {   3199,15}, {   1791,16}, {  65536,17}, { 131072,18}, \
    { 262144,19}, { 524288,20}, {1048576,21}, {2097152,22}, \
    {4194304,23}, {8388608,24} }
#define SQR_FFT_TABLE3_SIZE 182
#define SQR_FFT_THRESHOLD                 1728

#define MULLO_BASECASE_THRESHOLD            12
#define MULLO_DC_THRESHOLD                   0  /* never mpn_mullo_basecase */
#define MULLO_MUL_N_THRESHOLD             3791

#define DC_DIV_QR_THRESHOLD                 16
#define DC_DIVAPPR_Q_THRESHOLD              66
#define DC_BDIV_QR_THRESHOLD                27
#define DC_BDIV_Q_THRESHOLD                 86

#define INV_MULMOD_BNM1_THRESHOLD           58
#define INV_NEWTON_THRESHOLD                16
#define INV_APPR_THRESHOLD                  17

#define BINV_NEWTON_THRESHOLD              110
#define REDC_1_TO_REDC_2_THRESHOLD           0  /* always */
#define REDC_2_TO_REDC_N_THRESHOLD         115

#define MU_DIV_QR_THRESHOLD                618
#define MU_DIVAPPR_Q_THRESHOLD             551
#define MUPI_DIV_QR_THRESHOLD                0  /* always */
#define MU_BDIV_QR_THRESHOLD               562
#define MU_BDIV_Q_THRESHOLD                748

#define POWM_SEC_TABLE  4,23,130,961,1926

#define MATRIX22_STRASSEN_THRESHOLD         12
#define HGCD_THRESHOLD                      39
#define HGCD_APPR_THRESHOLD                 50
#define HGCD_REDUCE_THRESHOLD             1012
#define GCD_DC_THRESHOLD                   134
#define GCDEXT_DC_THRESHOLD                132
#define JACOBI_BASE_METHOD                   4

#define GET_STR_DC_THRESHOLD                19
#define GET_STR_PRECOMPUTE_THRESHOLD        28
#define SET_STR_DC_THRESHOLD               300
#define SET_STR_PRECOMPUTE_THRESHOLD      1043

#define FAC_DSC_THRESHOLD                  462
#define FAC_ODD_THRESHOLD                    0  /* always */
