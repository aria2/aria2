
/* Copyright 1998 by the Massachusetts Institute of Technology.
 * Copyright (C) 2012 Marko Kreen <markokr@gmail.com>
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

#include "ares_setup.h"

#ifdef HAVE_NETINET_IN_H
#  include <netinet/in.h>
#endif
#ifdef HAVE_NETDB_H
#  include <netdb.h>
#endif
#ifdef HAVE_ARPA_INET_H
#  include <arpa/inet.h>
#endif
#ifdef HAVE_ARPA_NAMESER_H
#  include <arpa/nameser.h>
#else
#  include "nameser.h"
#endif
#ifdef HAVE_ARPA_NAMESER_COMPAT_H
#  include <arpa/nameser_compat.h>
#endif

#include "ares.h"
#include "ares_dns.h"
#include "ares_data.h"
#include "ares_private.h"

int
ares_parse_soa_reply(const unsigned char *abuf, int alen,
		     struct ares_soa_reply **soa_out)
{
  const unsigned char *aptr;
  long len;
  char *qname = NULL, *rr_name = NULL;
  struct ares_soa_reply *soa = NULL;
  int qdcount, ancount;
  int status;

  if (alen < HFIXEDSZ)
    return ARES_EBADRESP;

  /* parse message header */
  qdcount = DNS_HEADER_QDCOUNT(abuf);
  ancount = DNS_HEADER_ANCOUNT(abuf);
  if (qdcount != 1 || ancount != 1)
    return ARES_EBADRESP;
  aptr = abuf + HFIXEDSZ;

  /* query name */
  status = ares__expand_name_for_response(aptr, abuf, alen, &qname, &len);
  if (status != ARES_SUCCESS)
    goto failed_stat;
  aptr += len;

  /* skip qtype & qclass */
  if (aptr + QFIXEDSZ > abuf + alen)
    goto failed;
  aptr += QFIXEDSZ;

  /* rr_name */
  status = ares__expand_name_for_response(aptr, abuf, alen, &rr_name, &len);
  if (status != ARES_SUCCESS)
    goto failed_stat;
  aptr += len;

  /* skip rr_type, rr_class, rr_ttl, rr_rdlen */
  if (aptr + RRFIXEDSZ > abuf + alen)
    goto failed;
  aptr += RRFIXEDSZ;

  /* allocate result struct */
  soa = ares_malloc_data(ARES_DATATYPE_SOA_REPLY);
  if (!soa)
    return ARES_ENOMEM;

  /* nsname */
  status = ares__expand_name_for_response(aptr, abuf, alen, &soa->nsname, &len);
  if (status != ARES_SUCCESS)
    goto failed_stat;
  aptr += len;

  /* hostmaster */
  status = ares__expand_name_for_response(aptr, abuf, alen, &soa->hostmaster, &len);
  if (status != ARES_SUCCESS)
    goto failed_stat;
  aptr += len;

  /* integer fields */
  if (aptr + 5 * 4 > abuf + alen)
    goto failed;
  soa->serial = DNS__32BIT(aptr + 0 * 4);
  soa->refresh = DNS__32BIT(aptr + 1 * 4);
  soa->retry = DNS__32BIT(aptr + 2 * 4);
  soa->expire = DNS__32BIT(aptr + 3 * 4);
  soa->minttl = DNS__32BIT(aptr + 4 * 4);

  free(qname);
  free(rr_name);

  *soa_out = soa;

  return ARES_SUCCESS;

failed:
  status = ARES_EBADRESP;

failed_stat:
  ares_free_data(soa);
  if (qname)
    free(qname);
  if (rr_name)
    free(rr_name);
  return status;
}

