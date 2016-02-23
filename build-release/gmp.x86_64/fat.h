/* fat.h - setups for fat binaries.
   Generated by configure - DO NOT EDIT.  */

#ifndef OPERATION_add_n
#undef  mpn_add_n
#define mpn_add_n  (*__gmpn_cpuvec.add_n)
#endif
DECL_add_n (__MPN(add_n_init));

#ifndef OPERATION_addmul_1
#undef  mpn_addmul_1
#define mpn_addmul_1  (*__gmpn_cpuvec.addmul_1)
#endif
DECL_addmul_1 (__MPN(addmul_1_init));

#ifndef OPERATION_bdiv_dbm1c
#undef  mpn_bdiv_dbm1c
#define mpn_bdiv_dbm1c  (*__gmpn_cpuvec.bdiv_dbm1c)
#endif
DECL_bdiv_dbm1c (__MPN(bdiv_dbm1c_init));

#ifndef OPERATION_com
#undef  mpn_com
#define mpn_com  (*__gmpn_cpuvec.com)
#endif
DECL_com (__MPN(com_init));

#ifndef OPERATION_copyd
#undef  mpn_copyd
#define mpn_copyd  (*__gmpn_cpuvec.copyd)
#endif
DECL_copyd (__MPN(copyd_init));

#ifndef OPERATION_copyi
#undef  mpn_copyi
#define mpn_copyi  (*__gmpn_cpuvec.copyi)
#endif
DECL_copyi (__MPN(copyi_init));

#ifndef OPERATION_dive_1
#undef  mpn_divexact_1
#define mpn_divexact_1  (*__gmpn_cpuvec.divexact_1)
#endif
DECL_divexact_1 (__MPN(divexact_1_init));

#ifndef OPERATION_divrem_1
#undef  mpn_divrem_1
#define mpn_divrem_1  (*__gmpn_cpuvec.divrem_1)
#endif
DECL_divrem_1 (__MPN(divrem_1_init));

#ifndef OPERATION_gcd_1
#undef  mpn_gcd_1
#define mpn_gcd_1  (*__gmpn_cpuvec.gcd_1)
#endif
DECL_gcd_1 (__MPN(gcd_1_init));

#ifndef OPERATION_lshift
#undef  mpn_lshift
#define mpn_lshift  (*__gmpn_cpuvec.lshift)
#endif
DECL_lshift (__MPN(lshift_init));

#ifndef OPERATION_lshiftc
#undef  mpn_lshiftc
#define mpn_lshiftc  (*__gmpn_cpuvec.lshiftc)
#endif
DECL_lshiftc (__MPN(lshiftc_init));

#ifndef OPERATION_mod_1
#undef  mpn_mod_1
#define mpn_mod_1  (*__gmpn_cpuvec.mod_1)
#endif
DECL_mod_1 (__MPN(mod_1_init));

#ifndef OPERATION_mod_1_1
#undef  mpn_mod_1_1p
#define mpn_mod_1_1p  (*__gmpn_cpuvec.mod_1_1p)
#endif
DECL_mod_1_1p (__MPN(mod_1_1p_init));

#ifndef OPERATION_mod_1_1_cps
#undef  mpn_mod_1_1p_cps
#define mpn_mod_1_1p_cps  (*__gmpn_cpuvec.mod_1_1p_cps)
#endif
DECL_mod_1_1p_cps (__MPN(mod_1_1p_cps_init));

#ifndef OPERATION_mod_1_2
#undef  mpn_mod_1s_2p
#define mpn_mod_1s_2p  (*__gmpn_cpuvec.mod_1s_2p)
#endif
DECL_mod_1s_2p (__MPN(mod_1s_2p_init));

#ifndef OPERATION_mod_1_2_cps
#undef  mpn_mod_1s_2p_cps
#define mpn_mod_1s_2p_cps  (*__gmpn_cpuvec.mod_1s_2p_cps)
#endif
DECL_mod_1s_2p_cps (__MPN(mod_1s_2p_cps_init));

#ifndef OPERATION_mod_1_4
#undef  mpn_mod_1s_4p
#define mpn_mod_1s_4p  (*__gmpn_cpuvec.mod_1s_4p)
#endif
DECL_mod_1s_4p (__MPN(mod_1s_4p_init));

#ifndef OPERATION_mod_1_4_cps
#undef  mpn_mod_1s_4p_cps
#define mpn_mod_1s_4p_cps  (*__gmpn_cpuvec.mod_1s_4p_cps)
#endif
DECL_mod_1s_4p_cps (__MPN(mod_1s_4p_cps_init));

#ifndef OPERATION_mod_34lsub1
#undef  mpn_mod_34lsub1
#define mpn_mod_34lsub1  (*__gmpn_cpuvec.mod_34lsub1)
#endif
DECL_mod_34lsub1 (__MPN(mod_34lsub1_init));

#ifndef OPERATION_mode1o
#undef  mpn_modexact_1c_odd
#define mpn_modexact_1c_odd  (*__gmpn_cpuvec.modexact_1c_odd)
#endif
DECL_modexact_1c_odd (__MPN(modexact_1c_odd_init));

#ifndef OPERATION_mul_1
#undef  mpn_mul_1
#define mpn_mul_1  (*__gmpn_cpuvec.mul_1)
#endif
DECL_mul_1 (__MPN(mul_1_init));

#ifndef OPERATION_mul_basecase
#undef  mpn_mul_basecase
#define mpn_mul_basecase  (*__gmpn_cpuvec.mul_basecase)
#endif
DECL_mul_basecase (__MPN(mul_basecase_init));

#ifndef OPERATION_mullo_basecase
#undef  mpn_mullo_basecase
#define mpn_mullo_basecase  (*__gmpn_cpuvec.mullo_basecase)
#endif
DECL_mullo_basecase (__MPN(mullo_basecase_init));

#ifndef OPERATION_pre_divrem_1
#undef  mpn_preinv_divrem_1
#define mpn_preinv_divrem_1  (*__gmpn_cpuvec.preinv_divrem_1)
#endif
DECL_preinv_divrem_1 (__MPN(preinv_divrem_1_init));

#ifndef OPERATION_pre_mod_1
#undef  mpn_preinv_mod_1
#define mpn_preinv_mod_1  (*__gmpn_cpuvec.preinv_mod_1)
#endif
DECL_preinv_mod_1 (__MPN(preinv_mod_1_init));

#ifndef OPERATION_redc_1
#undef  mpn_redc_1
#define mpn_redc_1  (*__gmpn_cpuvec.redc_1)
#endif
DECL_redc_1 (__MPN(redc_1_init));

#ifndef OPERATION_redc_2
#undef  mpn_redc_2
#define mpn_redc_2  (*__gmpn_cpuvec.redc_2)
#endif
DECL_redc_2 (__MPN(redc_2_init));

#ifndef OPERATION_rshift
#undef  mpn_rshift
#define mpn_rshift  (*__gmpn_cpuvec.rshift)
#endif
DECL_rshift (__MPN(rshift_init));

#ifndef OPERATION_sqr_basecase
#undef  mpn_sqr_basecase
#define mpn_sqr_basecase  (*__gmpn_cpuvec.sqr_basecase)
#endif
DECL_sqr_basecase (__MPN(sqr_basecase_init));

#ifndef OPERATION_sub_n
#undef  mpn_sub_n
#define mpn_sub_n  (*__gmpn_cpuvec.sub_n)
#endif
DECL_sub_n (__MPN(sub_n_init));

#ifndef OPERATION_submul_1
#undef  mpn_submul_1
#define mpn_submul_1  (*__gmpn_cpuvec.submul_1)
#endif
DECL_submul_1 (__MPN(submul_1_init));

#ifndef OPERATION_addmul_2
#undef  mpn_addmul_2
#define mpn_addmul_2  (*__gmpn_cpuvec.addmul_2)
#endif
DECL_addmul_2 (__MPN(addmul_2_init));

#ifndef OPERATION_addlsh1_n
#undef  mpn_addlsh1_n
#define mpn_addlsh1_n  (*__gmpn_cpuvec.addlsh1_n)
#endif
DECL_addlsh1_n (__MPN(addlsh1_n_init));

#ifndef OPERATION_addlsh2_n
#undef  mpn_addlsh2_n
#define mpn_addlsh2_n  (*__gmpn_cpuvec.addlsh2_n)
#endif
DECL_addlsh2_n (__MPN(addlsh2_n_init));

#ifndef OPERATION_sublsh1_n
#undef  mpn_sublsh1_n
#define mpn_sublsh1_n  (*__gmpn_cpuvec.sublsh1_n)
#endif
DECL_sublsh1_n (__MPN(sublsh1_n_init));

/* variable thresholds */
#undef  MUL_TOOM22_THRESHOLD
#define MUL_TOOM22_THRESHOLD  CPUVEC_THRESHOLD (mul_toom22_threshold)
#undef  MUL_TOOM33_THRESHOLD
#define MUL_TOOM33_THRESHOLD  CPUVEC_THRESHOLD (mul_toom33_threshold)
#undef  SQR_TOOM2_THRESHOLD
#define SQR_TOOM2_THRESHOLD  CPUVEC_THRESHOLD (sqr_toom2_threshold)
#undef  SQR_TOOM3_THRESHOLD
#define SQR_TOOM3_THRESHOLD  CPUVEC_THRESHOLD (sqr_toom3_threshold)
#undef  BMOD_1_TO_MOD_1_THRESHOLD
#define BMOD_1_TO_MOD_1_THRESHOLD  CPUVEC_THRESHOLD (bmod_1_to_mod_1_threshold)

/* Copy all fields into __gmpn_cpuvec.
   memcpy is not used because it might operate byte-wise (depending on its
   implementation), and we need the function pointer writes to be atomic.
   volatile discourages the compiler from trying to optimize this.  */
#define CPUVEC_INSTALL(vec) \
  do { \
    volatile struct cpuvec_t *p = &__gmpn_cpuvec; \
    p->add_n = vec.add_n; \
    p->addmul_1 = vec.addmul_1; \
    p->bdiv_dbm1c = vec.bdiv_dbm1c; \
    p->com = vec.com; \
    p->copyd = vec.copyd; \
    p->copyi = vec.copyi; \
    p->divexact_1 = vec.divexact_1; \
    p->divrem_1 = vec.divrem_1; \
    p->gcd_1 = vec.gcd_1; \
    p->lshift = vec.lshift; \
    p->lshiftc = vec.lshiftc; \
    p->mod_1 = vec.mod_1; \
    p->mod_1_1p = vec.mod_1_1p; \
    p->mod_1_1p_cps = vec.mod_1_1p_cps; \
    p->mod_1s_2p = vec.mod_1s_2p; \
    p->mod_1s_2p_cps = vec.mod_1s_2p_cps; \
    p->mod_1s_4p = vec.mod_1s_4p; \
    p->mod_1s_4p_cps = vec.mod_1s_4p_cps; \
    p->mod_34lsub1 = vec.mod_34lsub1; \
    p->modexact_1c_odd = vec.modexact_1c_odd; \
    p->mul_1 = vec.mul_1; \
    p->mul_basecase = vec.mul_basecase; \
    p->mullo_basecase = vec.mullo_basecase; \
    p->preinv_divrem_1 = vec.preinv_divrem_1; \
    p->preinv_mod_1 = vec.preinv_mod_1; \
    p->redc_1 = vec.redc_1; \
    p->redc_2 = vec.redc_2; \
    p->rshift = vec.rshift; \
    p->sqr_basecase = vec.sqr_basecase; \
    p->sub_n = vec.sub_n; \
    p->submul_1 = vec.submul_1; \
    p->addmul_2 = vec.addmul_2; \
    p->addlsh1_n = vec.addlsh1_n; \
    p->addlsh2_n = vec.addlsh2_n; \
    p->sublsh1_n = vec.sublsh1_n; \
    p->mul_toom22_threshold = vec.mul_toom22_threshold; \
    p->mul_toom33_threshold = vec.mul_toom33_threshold; \
    p->sqr_toom2_threshold = vec.sqr_toom2_threshold; \
    p->sqr_toom3_threshold = vec.sqr_toom3_threshold; \
    p->bmod_1_to_mod_1_threshold = vec.bmod_1_to_mod_1_threshold; \
  } while (0)

/* A helper to check all fields are filled. */
#define ASSERT_CPUVEC(vec) \
  do { \
    ASSERT (vec.add_n != NULL); \
    ASSERT (vec.addmul_1 != NULL); \
    ASSERT (vec.bdiv_dbm1c != NULL); \
    ASSERT (vec.com != NULL); \
    ASSERT (vec.copyd != NULL); \
    ASSERT (vec.copyi != NULL); \
    ASSERT (vec.divexact_1 != NULL); \
    ASSERT (vec.divrem_1 != NULL); \
    ASSERT (vec.gcd_1 != NULL); \
    ASSERT (vec.lshift != NULL); \
    ASSERT (vec.lshiftc != NULL); \
    ASSERT (vec.mod_1 != NULL); \
    ASSERT (vec.mod_1_1p != NULL); \
    ASSERT (vec.mod_1_1p_cps != NULL); \
    ASSERT (vec.mod_1s_2p != NULL); \
    ASSERT (vec.mod_1s_2p_cps != NULL); \
    ASSERT (vec.mod_1s_4p != NULL); \
    ASSERT (vec.mod_1s_4p_cps != NULL); \
    ASSERT (vec.mod_34lsub1 != NULL); \
    ASSERT (vec.modexact_1c_odd != NULL); \
    ASSERT (vec.mul_1 != NULL); \
    ASSERT (vec.mul_basecase != NULL); \
    ASSERT (vec.mullo_basecase != NULL); \
    ASSERT (vec.preinv_divrem_1 != NULL); \
    ASSERT (vec.preinv_mod_1 != NULL); \
    ASSERT (vec.redc_1 != NULL); \
    ASSERT (vec.redc_2 != NULL); \
    ASSERT (vec.rshift != NULL); \
    ASSERT (vec.sqr_basecase != NULL); \
    ASSERT (vec.sub_n != NULL); \
    ASSERT (vec.submul_1 != NULL); \
    ASSERT (vec.addmul_2 != NULL); \
    ASSERT (vec.addlsh1_n != NULL); \
    ASSERT (vec.addlsh2_n != NULL); \
    ASSERT (vec.sublsh1_n != NULL); \
    ASSERT (vec.mul_toom22_threshold != 0); \
    ASSERT (vec.mul_toom33_threshold != 0); \
    ASSERT (vec.sqr_toom2_threshold != 0); \
    ASSERT (vec.sqr_toom3_threshold != 0); \
    ASSERT (vec.bmod_1_to_mod_1_threshold != 0); \
  } while (0)

/* Call ITERATE(field) for each fat threshold field. */
#define ITERATE_FAT_THRESHOLDS() \
  do { \
    ITERATE (MUL_TOOM22_THRESHOLD, mul_toom22_threshold); \
    ITERATE (MUL_TOOM33_THRESHOLD, mul_toom33_threshold); \
    ITERATE (SQR_TOOM2_THRESHOLD, sqr_toom2_threshold); \
    ITERATE (SQR_TOOM3_THRESHOLD, sqr_toom3_threshold); \
    ITERATE (BMOD_1_TO_MOD_1_THRESHOLD, bmod_1_to_mod_1_threshold); \
  } while (0)

DECL_add_n (__gmpn_add_n_x86_64);
DECL_addmul_1 (__gmpn_addmul_1_x86_64);
DECL_bdiv_dbm1c (__gmpn_bdiv_dbm1c_x86_64);
DECL_com (__gmpn_com_x86_64);
DECL_copyd (__gmpn_copyd_x86_64);
DECL_copyi (__gmpn_copyi_x86_64);
DECL_divexact_1 (__gmpn_divexact_1_x86_64);
DECL_divrem_1 (__gmpn_divrem_1_x86_64);
DECL_preinv_divrem_1 (__gmpn_preinv_divrem_1_x86_64);
DECL_gcd_1 (__gmpn_gcd_1_x86_64);
DECL_lshift (__gmpn_lshift_x86_64);
DECL_lshiftc (__gmpn_lshiftc_x86_64);
DECL_mod_1_1p (__gmpn_mod_1_1p_x86_64);
DECL_mod_1_1p_cps (__gmpn_mod_1_1p_cps_x86_64);
DECL_mod_1s_2p (__gmpn_mod_1s_2p_x86_64);
DECL_mod_1s_2p_cps (__gmpn_mod_1s_2p_cps_x86_64);
DECL_mod_1s_4p (__gmpn_mod_1s_4p_x86_64);
DECL_mod_1s_4p_cps (__gmpn_mod_1s_4p_cps_x86_64);
DECL_mod_34lsub1 (__gmpn_mod_34lsub1_x86_64);
DECL_modexact_1c_odd (__gmpn_modexact_1c_odd_x86_64);
DECL_mul_1 (__gmpn_mul_1_x86_64);
DECL_mul_basecase (__gmpn_mul_basecase_x86_64);
DECL_mullo_basecase (__gmpn_mullo_basecase_x86_64);
DECL_redc_1 (__gmpn_redc_1_x86_64);
DECL_rshift (__gmpn_rshift_x86_64);
DECL_sqr_basecase (__gmpn_sqr_basecase_x86_64);
DECL_sub_n (__gmpn_sub_n_x86_64);
DECL_submul_1 (__gmpn_submul_1_x86_64);
DECL_addmul_2 (__gmpn_addmul_2_x86_64);
DECL_addlsh1_n (__gmpn_addlsh1_n_x86_64);
DECL_addlsh2_n (__gmpn_addlsh2_n_x86_64);
DECL_sublsh1_n (__gmpn_sublsh1_n_x86_64);

#define CPUVEC_SETUP_x86_64 \
  do { \
    decided_cpuvec.mul_toom22_threshold = 27; \
    decided_cpuvec.mul_toom33_threshold = 81; \
    decided_cpuvec.sqr_toom2_threshold = 36; \
    decided_cpuvec.sqr_toom3_threshold = 117; \
    decided_cpuvec.bmod_1_to_mod_1_threshold = 15; \
    decided_cpuvec.add_n = __gmpn_add_n_x86_64; \
    decided_cpuvec.addmul_1 = __gmpn_addmul_1_x86_64; \
    decided_cpuvec.bdiv_dbm1c = __gmpn_bdiv_dbm1c_x86_64; \
    decided_cpuvec.com = __gmpn_com_x86_64; \
    decided_cpuvec.copyd = __gmpn_copyd_x86_64; \
    decided_cpuvec.copyi = __gmpn_copyi_x86_64; \
    decided_cpuvec.divexact_1 = __gmpn_divexact_1_x86_64; \
    decided_cpuvec.divrem_1 = __gmpn_divrem_1_x86_64; \
    decided_cpuvec.preinv_divrem_1 = __gmpn_preinv_divrem_1_x86_64; \
    decided_cpuvec.gcd_1 = __gmpn_gcd_1_x86_64; \
    decided_cpuvec.lshift = __gmpn_lshift_x86_64; \
    decided_cpuvec.lshiftc = __gmpn_lshiftc_x86_64; \
    decided_cpuvec.mod_1_1p = __gmpn_mod_1_1p_x86_64; \
    decided_cpuvec.mod_1_1p_cps = __gmpn_mod_1_1p_cps_x86_64; \
    decided_cpuvec.mod_1s_2p = __gmpn_mod_1s_2p_x86_64; \
    decided_cpuvec.mod_1s_2p_cps = __gmpn_mod_1s_2p_cps_x86_64; \
    decided_cpuvec.mod_1s_4p = __gmpn_mod_1s_4p_x86_64; \
    decided_cpuvec.mod_1s_4p_cps = __gmpn_mod_1s_4p_cps_x86_64; \
    decided_cpuvec.mod_34lsub1 = __gmpn_mod_34lsub1_x86_64; \
    decided_cpuvec.modexact_1c_odd = __gmpn_modexact_1c_odd_x86_64; \
    decided_cpuvec.mul_1 = __gmpn_mul_1_x86_64; \
    decided_cpuvec.mul_basecase = __gmpn_mul_basecase_x86_64; \
    decided_cpuvec.mullo_basecase = __gmpn_mullo_basecase_x86_64; \
    decided_cpuvec.redc_1 = __gmpn_redc_1_x86_64; \
    decided_cpuvec.rshift = __gmpn_rshift_x86_64; \
    decided_cpuvec.sqr_basecase = __gmpn_sqr_basecase_x86_64; \
    decided_cpuvec.sub_n = __gmpn_sub_n_x86_64; \
    decided_cpuvec.submul_1 = __gmpn_submul_1_x86_64; \
    decided_cpuvec.addmul_2 = __gmpn_addmul_2_x86_64; \
    decided_cpuvec.addlsh1_n = __gmpn_addlsh1_n_x86_64; \
    decided_cpuvec.addlsh2_n = __gmpn_addlsh2_n_x86_64; \
    decided_cpuvec.sublsh1_n = __gmpn_sublsh1_n_x86_64; \
  } while (0)

DECL_mod_1 (__gmpn_mod_1_fat);
DECL_redc_2 (__gmpn_redc_2_fat);

#define CPUVEC_SETUP_fat \
  do { \
    decided_cpuvec.mul_toom22_threshold = 20; \
    decided_cpuvec.mul_toom33_threshold = 130; \
    decided_cpuvec.sqr_toom2_threshold = 30; \
    decided_cpuvec.sqr_toom3_threshold = 200; \
    decided_cpuvec.bmod_1_to_mod_1_threshold = 20; \
    decided_cpuvec.mod_1 = __gmpn_mod_1_fat; \
    decided_cpuvec.redc_2 = __gmpn_redc_2_fat; \
  } while (0)


#define CPUVEC_SETUP_k8 \
  do { \
    decided_cpuvec.mul_toom22_threshold = 27; \
    decided_cpuvec.mul_toom33_threshold = 81; \
    decided_cpuvec.sqr_toom2_threshold = 34; \
    decided_cpuvec.sqr_toom3_threshold = 115; \
    decided_cpuvec.bmod_1_to_mod_1_threshold = 17; \
  } while (0)

DECL_gcd_1 (__gmpn_gcd_1_k10);
DECL_lshift (__gmpn_lshift_k10);
DECL_lshiftc (__gmpn_lshiftc_k10);
DECL_rshift (__gmpn_rshift_k10);

#define CPUVEC_SETUP_k10 \
  do { \
    decided_cpuvec.mul_toom22_threshold = 28; \
    decided_cpuvec.mul_toom33_threshold = 81; \
    decided_cpuvec.sqr_toom2_threshold = 34; \
    decided_cpuvec.sqr_toom3_threshold = 113; \
    decided_cpuvec.bmod_1_to_mod_1_threshold = 15; \
    decided_cpuvec.gcd_1 = __gmpn_gcd_1_k10; \
    decided_cpuvec.lshift = __gmpn_lshift_k10; \
    decided_cpuvec.lshiftc = __gmpn_lshiftc_k10; \
    decided_cpuvec.rshift = __gmpn_rshift_k10; \
  } while (0)

DECL_addmul_1 (__gmpn_addmul_1_bd1);
DECL_gcd_1 (__gmpn_gcd_1_bd1);
DECL_mul_1 (__gmpn_mul_1_bd1);
DECL_submul_1 (__gmpn_submul_1_bd1);

#define CPUVEC_SETUP_bd1 \
  do { \
    decided_cpuvec.mul_toom22_threshold = 16; \
    decided_cpuvec.mul_toom33_threshold = 57; \
    decided_cpuvec.sqr_toom2_threshold = 24; \
    decided_cpuvec.sqr_toom3_threshold = 139; \
    decided_cpuvec.bmod_1_to_mod_1_threshold = 20; \
    decided_cpuvec.addmul_1 = __gmpn_addmul_1_bd1; \
    decided_cpuvec.gcd_1 = __gmpn_gcd_1_bd1; \
    decided_cpuvec.mul_1 = __gmpn_mul_1_bd1; \
    decided_cpuvec.submul_1 = __gmpn_submul_1_bd1; \
  } while (0)

DECL_addmul_1 (__gmpn_addmul_1_bobcat);
DECL_copyd (__gmpn_copyd_bobcat);
DECL_copyi (__gmpn_copyi_bobcat);
DECL_mul_1 (__gmpn_mul_1_bobcat);
DECL_mul_basecase (__gmpn_mul_basecase_bobcat);
DECL_sqr_basecase (__gmpn_sqr_basecase_bobcat);
DECL_submul_1 (__gmpn_submul_1_bobcat);

#define CPUVEC_SETUP_bobcat \
  do { \
    decided_cpuvec.mul_toom22_threshold = 27; \
    decided_cpuvec.mul_toom33_threshold = 32; \
    decided_cpuvec.sqr_toom2_threshold = 28; \
    decided_cpuvec.sqr_toom3_threshold = 93; \
    decided_cpuvec.bmod_1_to_mod_1_threshold = 17; \
    decided_cpuvec.addmul_1 = __gmpn_addmul_1_bobcat; \
    decided_cpuvec.copyd = __gmpn_copyd_bobcat; \
    decided_cpuvec.copyi = __gmpn_copyi_bobcat; \
    decided_cpuvec.mul_1 = __gmpn_mul_1_bobcat; \
    decided_cpuvec.mul_basecase = __gmpn_mul_basecase_bobcat; \
    decided_cpuvec.sqr_basecase = __gmpn_sqr_basecase_bobcat; \
    decided_cpuvec.submul_1 = __gmpn_submul_1_bobcat; \
  } while (0)

DECL_add_n (__gmpn_add_n_pentium4);
DECL_lshift (__gmpn_lshift_pentium4);
DECL_lshiftc (__gmpn_lshiftc_pentium4);
DECL_mod_34lsub1 (__gmpn_mod_34lsub1_pentium4);
DECL_rshift (__gmpn_rshift_pentium4);
DECL_sub_n (__gmpn_sub_n_pentium4);
DECL_addlsh1_n (__gmpn_addlsh1_n_pentium4);
DECL_addlsh2_n (__gmpn_addlsh2_n_pentium4);
DECL_sublsh1_n (__gmpn_sublsh1_n_pentium4);

#define CPUVEC_SETUP_pentium4 \
  do { \
    decided_cpuvec.mul_toom22_threshold = 11; \
    decided_cpuvec.mul_toom33_threshold = 68; \
    decided_cpuvec.sqr_toom2_threshold = 18; \
    decided_cpuvec.sqr_toom3_threshold = 81; \
    decided_cpuvec.bmod_1_to_mod_1_threshold = 20; \
    decided_cpuvec.add_n = __gmpn_add_n_pentium4; \
    decided_cpuvec.lshift = __gmpn_lshift_pentium4; \
    decided_cpuvec.lshiftc = __gmpn_lshiftc_pentium4; \
    decided_cpuvec.mod_34lsub1 = __gmpn_mod_34lsub1_pentium4; \
    decided_cpuvec.rshift = __gmpn_rshift_pentium4; \
    decided_cpuvec.sub_n = __gmpn_sub_n_pentium4; \
    decided_cpuvec.addlsh1_n = __gmpn_addlsh1_n_pentium4; \
    decided_cpuvec.addlsh2_n = __gmpn_addlsh2_n_pentium4; \
    decided_cpuvec.sublsh1_n = __gmpn_sublsh1_n_pentium4; \
  } while (0)

DECL_add_n (__gmpn_add_n_core2);
DECL_addmul_1 (__gmpn_addmul_1_core2);
DECL_copyd (__gmpn_copyd_core2);
DECL_copyi (__gmpn_copyi_core2);
DECL_divrem_1 (__gmpn_divrem_1_core2);
DECL_preinv_divrem_1 (__gmpn_preinv_divrem_1_core2);
DECL_gcd_1 (__gmpn_gcd_1_core2);
DECL_lshift (__gmpn_lshift_core2);
DECL_lshiftc (__gmpn_lshiftc_core2);
DECL_rshift (__gmpn_rshift_core2);
DECL_sub_n (__gmpn_sub_n_core2);
DECL_submul_1 (__gmpn_submul_1_core2);
DECL_addlsh1_n (__gmpn_addlsh1_n_core2);
DECL_addlsh2_n (__gmpn_addlsh2_n_core2);
DECL_sublsh1_n (__gmpn_sublsh1_n_core2);

#define CPUVEC_SETUP_core2 \
  do { \
    decided_cpuvec.mul_toom22_threshold = 23; \
    decided_cpuvec.mul_toom33_threshold = 65; \
    decided_cpuvec.sqr_toom2_threshold = 28; \
    decided_cpuvec.sqr_toom3_threshold = 101; \
    decided_cpuvec.bmod_1_to_mod_1_threshold = 23; \
    decided_cpuvec.add_n = __gmpn_add_n_core2; \
    decided_cpuvec.addmul_1 = __gmpn_addmul_1_core2; \
    decided_cpuvec.copyd = __gmpn_copyd_core2; \
    decided_cpuvec.copyi = __gmpn_copyi_core2; \
    decided_cpuvec.divrem_1 = __gmpn_divrem_1_core2; \
    decided_cpuvec.preinv_divrem_1 = __gmpn_preinv_divrem_1_core2; \
    decided_cpuvec.gcd_1 = __gmpn_gcd_1_core2; \
    decided_cpuvec.lshift = __gmpn_lshift_core2; \
    decided_cpuvec.lshiftc = __gmpn_lshiftc_core2; \
    decided_cpuvec.rshift = __gmpn_rshift_core2; \
    decided_cpuvec.sub_n = __gmpn_sub_n_core2; \
    decided_cpuvec.submul_1 = __gmpn_submul_1_core2; \
    decided_cpuvec.addlsh1_n = __gmpn_addlsh1_n_core2; \
    decided_cpuvec.addlsh2_n = __gmpn_addlsh2_n_core2; \
    decided_cpuvec.sublsh1_n = __gmpn_sublsh1_n_core2; \
  } while (0)


#define CPUVEC_SETUP_coreinhm \
  do { \
    decided_cpuvec.mul_toom22_threshold = 18; \
    decided_cpuvec.mul_toom33_threshold = 57; \
    decided_cpuvec.sqr_toom2_threshold = 30; \
    decided_cpuvec.sqr_toom3_threshold = 101; \
    decided_cpuvec.bmod_1_to_mod_1_threshold = 16; \
  } while (0)

DECL_add_n (__gmpn_add_n_coreisbr);
DECL_addmul_1 (__gmpn_addmul_1_coreisbr);
DECL_lshift (__gmpn_lshift_coreisbr);
DECL_lshiftc (__gmpn_lshiftc_coreisbr);
DECL_mul_1 (__gmpn_mul_1_coreisbr);
DECL_rshift (__gmpn_rshift_coreisbr);
DECL_sub_n (__gmpn_sub_n_coreisbr);
DECL_submul_1 (__gmpn_submul_1_coreisbr);
DECL_addmul_2 (__gmpn_addmul_2_coreisbr);
DECL_addlsh1_n (__gmpn_addlsh1_n_coreisbr);
DECL_addlsh2_n (__gmpn_addlsh2_n_coreisbr);

#define CPUVEC_SETUP_coreisbr \
  do { \
    decided_cpuvec.mul_toom22_threshold = 18; \
    decided_cpuvec.mul_toom33_threshold = 57; \
    decided_cpuvec.sqr_toom2_threshold = 26; \
    decided_cpuvec.sqr_toom3_threshold = 81; \
    decided_cpuvec.bmod_1_to_mod_1_threshold = 30; \
    decided_cpuvec.add_n = __gmpn_add_n_coreisbr; \
    decided_cpuvec.addmul_1 = __gmpn_addmul_1_coreisbr; \
    decided_cpuvec.lshift = __gmpn_lshift_coreisbr; \
    decided_cpuvec.lshiftc = __gmpn_lshiftc_coreisbr; \
    decided_cpuvec.mul_1 = __gmpn_mul_1_coreisbr; \
    decided_cpuvec.rshift = __gmpn_rshift_coreisbr; \
    decided_cpuvec.sub_n = __gmpn_sub_n_coreisbr; \
    decided_cpuvec.submul_1 = __gmpn_submul_1_coreisbr; \
    decided_cpuvec.addmul_2 = __gmpn_addmul_2_coreisbr; \
    decided_cpuvec.addlsh1_n = __gmpn_addlsh1_n_coreisbr; \
    decided_cpuvec.addlsh2_n = __gmpn_addlsh2_n_coreisbr; \
  } while (0)

DECL_add_n (__gmpn_add_n_atom);
DECL_copyd (__gmpn_copyd_atom);
DECL_copyi (__gmpn_copyi_atom);
DECL_divexact_1 (__gmpn_divexact_1_atom);
DECL_lshift (__gmpn_lshift_atom);
DECL_lshiftc (__gmpn_lshiftc_atom);
DECL_rshift (__gmpn_rshift_atom);
DECL_sub_n (__gmpn_sub_n_atom);
DECL_addlsh1_n (__gmpn_addlsh1_n_atom);
DECL_addlsh2_n (__gmpn_addlsh2_n_atom);
DECL_sublsh1_n (__gmpn_sublsh1_n_atom);

#define CPUVEC_SETUP_atom \
  do { \
    decided_cpuvec.mul_toom22_threshold = 10; \
    decided_cpuvec.mul_toom33_threshold = 66; \
    decided_cpuvec.sqr_toom2_threshold = 16; \
    decided_cpuvec.sqr_toom3_threshold = 65; \
    decided_cpuvec.bmod_1_to_mod_1_threshold = 15; \
    decided_cpuvec.add_n = __gmpn_add_n_atom; \
    decided_cpuvec.copyd = __gmpn_copyd_atom; \
    decided_cpuvec.copyi = __gmpn_copyi_atom; \
    decided_cpuvec.divexact_1 = __gmpn_divexact_1_atom; \
    decided_cpuvec.lshift = __gmpn_lshift_atom; \
    decided_cpuvec.lshiftc = __gmpn_lshiftc_atom; \
    decided_cpuvec.rshift = __gmpn_rshift_atom; \
    decided_cpuvec.sub_n = __gmpn_sub_n_atom; \
    decided_cpuvec.addlsh1_n = __gmpn_addlsh1_n_atom; \
    decided_cpuvec.addlsh2_n = __gmpn_addlsh2_n_atom; \
    decided_cpuvec.sublsh1_n = __gmpn_sublsh1_n_atom; \
  } while (0)

DECL_copyd (__gmpn_copyd_nano);
DECL_copyi (__gmpn_copyi_nano);
DECL_divexact_1 (__gmpn_divexact_1_nano);
DECL_gcd_1 (__gmpn_gcd_1_nano);

#define CPUVEC_SETUP_nano \
  do { \
    decided_cpuvec.mul_toom22_threshold = 27; \
    decided_cpuvec.mul_toom33_threshold = 33; \
    decided_cpuvec.sqr_toom2_threshold = 34; \
    decided_cpuvec.sqr_toom3_threshold = 93; \
    decided_cpuvec.bmod_1_to_mod_1_threshold = 22; \
    decided_cpuvec.copyd = __gmpn_copyd_nano; \
    decided_cpuvec.copyi = __gmpn_copyi_nano; \
    decided_cpuvec.divexact_1 = __gmpn_divexact_1_nano; \
    decided_cpuvec.gcd_1 = __gmpn_gcd_1_nano; \
  } while (0)

#define MUL_TOOM22_THRESHOLD_LIMIT  28
#define MUL_TOOM33_THRESHOLD_LIMIT  130
#define SQR_TOOM2_THRESHOLD_LIMIT  36
#define SQR_TOOM3_THRESHOLD_LIMIT  200
#define BMOD_1_TO_MOD_1_THRESHOLD_LIMIT  30
