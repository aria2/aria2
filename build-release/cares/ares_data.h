
/* Copyright (C) 2009-2013 by Daniel Stenberg
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

typedef enum {
  ARES_DATATYPE_UNKNOWN = 1,  /* unknown data type     - introduced in 1.7.0 */
  ARES_DATATYPE_SRV_REPLY,    /* struct ares_srv_reply - introduced in 1.7.0 */
  ARES_DATATYPE_TXT_REPLY,    /* struct ares_txt_reply - introduced in 1.7.0 */
  ARES_DATATYPE_ADDR_NODE,    /* struct ares_addr_node - introduced in 1.7.1 */
  ARES_DATATYPE_MX_REPLY,    /* struct ares_mx_reply   - introduced in 1.7.2 */
  ARES_DATATYPE_NAPTR_REPLY,/* struct ares_naptr_reply - introduced in 1.7.6 */
  ARES_DATATYPE_SOA_REPLY,    /* struct ares_soa_reply - introduced in 1.9.0 */
#if 0
  ARES_DATATYPE_ADDR6TTL,     /* struct ares_addrttl   */
  ARES_DATATYPE_ADDRTTL,      /* struct ares_addr6ttl  */
  ARES_DATATYPE_HOSTENT,      /* struct hostent        */
  ARES_DATATYPE_OPTIONS,      /* struct ares_options   */
#endif
  ARES_DATATYPE_LAST          /* not used              - introduced in 1.7.0 */
} ares_datatype;

#define ARES_DATATYPE_MARK 0xbead

/*
 * ares_data struct definition is internal to c-ares and shall not
 * be exposed by the public API in order to allow future changes
 * and extensions to it without breaking ABI.  This will be used
 * internally by c-ares as the container of multiple types of data
 * dynamically allocated for which a reference will be returned
 * to the calling application.
 *
 * c-ares API functions returning a pointer to c-ares internally
 * allocated data will actually be returning an interior pointer
 * into this ares_data struct.
 *
 * All this is 'invisible' to the calling application, the only
 * requirement is that this kind of data must be free'ed by the
 * calling application using ares_free_data() with the pointer
 * it has received from a previous c-ares function call.
 */

struct ares_data {
  ares_datatype type;  /* Actual data type identifier. */
  unsigned int  mark;  /* Private ares_data signature. */
  union {
    struct ares_txt_reply   txt_reply;
    struct ares_srv_reply   srv_reply;
    struct ares_addr_node   addr_node;
    struct ares_mx_reply    mx_reply;
    struct ares_naptr_reply naptr_reply;
    struct ares_soa_reply soa_reply;
  } data;
};

void *ares_malloc_data(ares_datatype type);

