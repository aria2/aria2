/* POWER4/PowerPC970 gmp-mparam.h -- Compiler/machine parameter header file.

Copyright 2008, 2009, 2010 Free Software Foundation, Inc.

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
#define MOD_1N_TO_MOD_1_1_THRESHOLD          6
#define MOD_1U_TO_MOD_1_1_THRESHOLD          5
#define MOD_1_1_TO_MOD_1_2_THRESHOLD         9
#define MOD_1_2_TO_MOD_1_4_THRESHOLD        20
#define PREINV_MOD_1_TO_MOD_1_THRESHOLD     16
#define USE_PREINV_DIVREM_1                  0
#define DIV_QR_2_PI2_THRESHOLD           MP_SIZE_T_MAX  /* never */
#define DIVEXACT_1_THRESHOLD                 0  /* always (native) */
#define BMOD_1_TO_MOD_1_THRESHOLD           37

#define MUL_TOOM22_THRESHOLD                18
#define MUL_TOOM33_THRESHOLD                53
#define MUL_TOOM44_THRESHOLD               106
#define MUL_TOOM6H_THRESHOLD               180
#define MUL_TOOM8H_THRESHOLD               309

#define MUL_TOOM32_TO_TOOM43_THRESHOLD      61
#define MUL_TOOM32_TO_TOOM53_THRESHOLD      91
#define MUL_TOOM42_TO_TOOM53_THRESHOLD      92
#define MUL_TOOM42_TO_TOOM63_THRESHOLD      51
#define MUL_TOOM43_TO_TOOM54_THRESHOLD      79

#define SQR_BASECASE_THRESHOLD               0  /* always (native) */
#define SQR_TOOM2_THRESHOLD                 28
#define SQR_TOOM3_THRESHOLD                 73
#define SQR_TOOM4_THRESHOLD                136
#define SQR_TOOM6_THRESHOLD                194
#define SQR_TOOM8_THRESHOLD                272

#define MULMID_TOOM42_THRESHOLD             32

#define MULMOD_BNM1_THRESHOLD               11
#define SQRMOD_BNM1_THRESHOLD               16

#define MUL_FFT_MODF_THRESHOLD             372  /* k = 5 */
#define MUL_FFT_TABLE3                                      \
  { {    372, 5}, {     13, 6}, {      7, 5}, {     15, 6}, \
    {      8, 5}, {     17, 6}, {      9, 5}, {     19, 6}, \
    {     10, 5}, {     21, 6}, {     19, 7}, {     10, 6}, \
    {     21, 7}, {     11, 6}, {     23, 7}, {     21, 8}, \
    {     11, 7}, {     25, 8}, {     13, 7}, {     31, 8}, \
    {     17, 7}, {     35, 8}, {     21, 9}, {     11, 8}, \
    {     27, 9}, {     15, 8}, {     35, 9}, {     19, 8}, \
    {     41, 9}, {     23, 8}, {     49, 9}, {     27,10}, \
    {     15, 9}, {     39,10}, {     23, 9}, {     55,11}, \
    {     15,10}, {     31, 9}, {     71,10}, {     39, 9}, \
    {     83,10}, {     47, 9}, {     99,10}, {     55,11}, \
    {     31,10}, {     63, 9}, {    127,10}, {     79,11}, \
    {     47,10}, {     95, 9}, {    191,10}, {    103, 9}, \
    {    207,12}, {     31,11}, {     63,10}, {    127, 9}, \
    {    255,10}, {    135, 9}, {    271,11}, {     79,10}, \
    {    159, 9}, {    319,10}, {    167,11}, {     95,10}, \
    {    191, 9}, {    383, 8}, {    767,10}, {    207, 9}, \
    {    415,11}, {    111,10}, {    223,12}, {     63,11}, \
    {    127,10}, {    255, 9}, {    511,10}, {    271, 9}, \
    {    543,11}, {    143,10}, {    287, 9}, {    575, 8}, \
    {   1151,10}, {    303, 9}, {    607,10}, {    319, 9}, \
    {    639,10}, {    335,12}, {     95,11}, {    191,10}, \
    {    383, 9}, {    767,11}, {    207,10}, {    415, 9}, \
    {    831,11}, {    223,10}, {    447,13}, {   8192,14}, \
    {  16384,15}, {  32768,16}, {  65536,17}, { 131072,18}, \
    { 262144,19}, { 524288,20}, {1048576,21}, {2097152,22}, \
    {4194304,23}, {8388608,24} }
#define MUL_FFT_TABLE3_SIZE 106
#define MUL_FFT_THRESHOLD                 3264

#define SQR_FFT_MODF_THRESHOLD             284  /* k = 5 */
#define SQR_FFT_TABLE3                                      \
  { {    280, 5}, {     13, 6}, {      7, 5}, {     15, 6}, \
    {      8, 5}, {     17, 6}, {     19, 7}, {     10, 6}, \
    {     21, 7}, {     21, 8}, {     11, 7}, {     24, 8}, \
    {     13, 7}, {     29, 8}, {     15, 7}, {     31, 8}, \
    {     17, 7}, {     35, 8}, {     21, 9}, {     11, 8}, \
    {     27, 9}, {     15, 8}, {     33, 9}, {     19, 8}, \
    {     41, 9}, {     23, 8}, {     47, 9}, {     27,10}, \
    {     15, 9}, {     39,10}, {     23, 9}, {     51,11}, \
    {     15,10}, {     31, 9}, {     67,10}, {     39, 9}, \
    {     83,10}, {     47, 9}, {     95,10}, {     55,11}, \
    {     31,10}, {     71, 9}, {    143,10}, {     79,11}, \
    {     47,10}, {     95, 9}, {    191, 8}, {    383,10}, \
    {    103,12}, {     31,11}, {     63,10}, {    127, 9}, \
    {    255, 8}, {    511,10}, {    135, 9}, {    271,10}, \
    {    143, 9}, {    287,11}, {     79,10}, {    159, 9}, \
    {    319, 8}, {    639,10}, {    175, 9}, {    351,11}, \
    {     95,10}, {    191, 9}, {    383, 8}, {    767,10}, \
    {    207, 9}, {    415,12}, {     63,11}, {    127,10}, \
    {    255, 9}, {    511,10}, {    271,11}, {    143,10}, \
    {    287, 9}, {    575,11}, {    159,10}, {    319, 9}, \
    {    639,11}, {    175,10}, {    351,12}, {     95,11}, \
    {    191,10}, {    383, 9}, {    767,11}, {    207,10}, \
    {    415, 9}, {    831,11}, {    223,10}, {    447,13}, \
    {   8192,14}, {  16384,15}, {  32768,16}, {  65536,17}, \
    { 131072,18}, { 262144,19}, { 524288,20}, {1048576,21}, \
    {2097152,22}, {4194304,23}, {8388608,24} }
#define SQR_FFT_TABLE3_SIZE 103
#define SQR_FFT_THRESHOLD                 2752

#define MULLO_BASECASE_THRESHOLD             3
#define MULLO_DC_THRESHOLD                  40
#define MULLO_MUL_N_THRESHOLD             6440

#define DC_DIV_QR_THRESHOLD                 43
#define DC_DIVAPPR_Q_THRESHOLD             166
#define DC_BDIV_QR_THRESHOLD                47
#define DC_BDIV_Q_THRESHOLD                112

#define INV_MULMOD_BNM1_THRESHOLD           50
#define INV_NEWTON_THRESHOLD               181
#define INV_APPR_THRESHOLD                 165

#define BINV_NEWTON_THRESHOLD              214
#define REDC_1_TO_REDC_N_THRESHOLD          55

#define MU_DIV_QR_THRESHOLD                998
#define MU_DIVAPPR_Q_THRESHOLD            1017
#define MUPI_DIV_QR_THRESHOLD               84
#define MU_BDIV_QR_THRESHOLD               855
#define MU_BDIV_Q_THRESHOLD               1017

#define POWM_SEC_TABLE  4,32,327,1100,2826

#define MATRIX22_STRASSEN_THRESHOLD         12
#define HGCD_THRESHOLD                     109
#define HGCD_APPR_THRESHOLD                107
#define HGCD_REDUCE_THRESHOLD             2121
#define GCD_DC_THRESHOLD                   348
#define GCDEXT_DC_THRESHOLD                246
#define JACOBI_BASE_METHOD                   4

#define GET_STR_DC_THRESHOLD                11
#define GET_STR_PRECOMPUTE_THRESHOLD        23
#define SET_STR_DC_THRESHOLD               650
#define SET_STR_PRECOMPUTE_THRESHOLD      1713

#define FAC_DSC_THRESHOLD                  562
#define FAC_ODD_THRESHOLD                   23
