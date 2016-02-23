/* Copyright 1998 by the Massachusetts Institute of Technology.
 *
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
#ifdef HAVE_ARPA_INET_H
#  include <arpa/inet.h>
#endif
#ifdef HAVE_NETDB_H
#  include <netdb.h>
#endif
#ifdef HAVE_ARPA_NAMESER_H
#  include <arpa/nameser.h>
#else
#  include "nameser.h"
#endif
#ifdef HAVE_ARPA_NAMESER_COMPAT_H
#  include <arpa/nameser_compat.h>
#endif

#ifdef HAVE_STRINGS_H
#  include <strings.h>
#endif

#include "ares.h"
#include "ares_dns.h"
#include "ares_getopt.h"
#include "ares_nowarn.h"

#ifndef HAVE_STRDUP
#  include "ares_strdup.h"
#  define strdup(ptr) ares_strdup(ptr)
#endif

#ifndef HAVE_STRCASECMP
#  include "ares_strcasecmp.h"
#  define strcasecmp(p1,p2) ares_strcasecmp(p1,p2)
#endif

#ifndef HAVE_STRNCASECMP
#  include "ares_strcasecmp.h"
#  define strncasecmp(p1,p2,n) ares_strncasecmp(p1,p2,n)
#endif

#ifdef WATT32
#undef WIN32  /* Redefined in MingW headers */
#endif

#ifndef T_SRV
#  define T_SRV     33 /* Server selection */
#endif
#ifndef T_NAPTR
#  define T_NAPTR   35 /* Naming authority pointer */
#endif
#ifndef T_DS
#  define T_DS      43 /* Delegation Signer (RFC4034) */
#endif
#ifndef T_SSHFP
#  define T_SSHFP   44 /* SSH Key Fingerprint (RFC4255) */
#endif
#ifndef T_RRSIG
#  define T_RRSIG   46 /* Resource Record Signature (RFC4034) */
#endif
#ifndef T_NSEC
#  define T_NSEC    47 /* Next Secure (RFC4034) */
#endif
#ifndef T_DNSKEY
#  define T_DNSKEY  48 /* DNS Public Key (RFC4034) */
#endif

struct nv {
  const char *name;
  int value;
};

static const struct nv flags[] = {
  { "usevc",            ARES_FLAG_USEVC },
  { "primary",          ARES_FLAG_PRIMARY },
  { "igntc",            ARES_FLAG_IGNTC },
  { "norecurse",        ARES_FLAG_NORECURSE },
  { "stayopen",         ARES_FLAG_STAYOPEN },
  { "noaliases",        ARES_FLAG_NOALIASES }
};
static const int nflags = sizeof(flags) / sizeof(flags[0]);

static const struct nv classes[] = {
  { "IN",       C_IN },
  { "CHAOS",    C_CHAOS },
  { "HS",       C_HS },
  { "ANY",      C_ANY }
};
static const int nclasses = sizeof(classes) / sizeof(classes[0]);

static const struct nv types[] = {
  { "A",        T_A },
  { "NS",       T_NS },
  { "MD",       T_MD },
  { "MF",       T_MF },
  { "CNAME",    T_CNAME },
  { "SOA",      T_SOA },
  { "MB",       T_MB },
  { "MG",       T_MG },
  { "MR",       T_MR },
  { "NULL",     T_NULL },
  { "WKS",      T_WKS },
  { "PTR",      T_PTR },
  { "HINFO",    T_HINFO },
  { "MINFO",    T_MINFO },
  { "MX",       T_MX },
  { "TXT",      T_TXT },
  { "RP",       T_RP },
  { "AFSDB",    T_AFSDB },
  { "X25",      T_X25 },
  { "ISDN",     T_ISDN },
  { "RT",       T_RT },
  { "NSAP",     T_NSAP },
  { "NSAP_PTR", T_NSAP_PTR },
  { "SIG",      T_SIG },
  { "KEY",      T_KEY },
  { "PX",       T_PX },
  { "GPOS",     T_GPOS },
  { "AAAA",     T_AAAA },
  { "LOC",      T_LOC },
  { "SRV",      T_SRV },
  { "AXFR",     T_AXFR },
  { "MAILB",    T_MAILB },
  { "MAILA",    T_MAILA },
  { "NAPTR",    T_NAPTR },
  { "DS",       T_DS },
  { "SSHFP",    T_SSHFP },
  { "RRSIG",    T_RRSIG },
  { "NSEC",     T_NSEC },
  { "DNSKEY",   T_DNSKEY },
  { "ANY",      T_ANY }
};
static const int ntypes = sizeof(types) / sizeof(types[0]);

static const char *opcodes[] = {
  "QUERY", "IQUERY", "STATUS", "(reserved)", "NOTIFY",
  "(unknown)", "(unknown)", "(unknown)", "(unknown)",
  "UPDATEA", "UPDATED", "UPDATEDA", "UPDATEM", "UPDATEMA",
  "ZONEINIT", "ZONEREF"
};

static const char *rcodes[] = {
  "NOERROR", "FORMERR", "SERVFAIL", "NXDOMAIN", "NOTIMP", "REFUSED",
  "(unknown)", "(unknown)", "(unknown)", "(unknown)", "(unknown)",
  "(unknown)", "(unknown)", "(unknown)", "(unknown)", "NOCHANGE"
};

static void callback(void *arg, int status, int timeouts,
                     unsigned char *abuf, int alen);
static const unsigned char *display_question(const unsigned char *aptr,
                                             const unsigned char *abuf,
                                             int alen);
static const unsigned char *display_rr(const unsigned char *aptr,
                                       const unsigned char *abuf, int alen);
static const char *type_name(int type);
static const char *class_name(int dnsclass);
static void usage(void);
static void destroy_addr_list(struct ares_addr_node *head);
static void append_addr_list(struct ares_addr_node **head,
                             struct ares_addr_node *node);

int main(int argc, char **argv)
{
  ares_channel channel;
  int c, i, optmask = ARES_OPT_FLAGS, dnsclass = C_IN, type = T_A;
  int status, nfds, count;
  struct ares_options options;
  struct hostent *hostent;
  fd_set read_fds, write_fds;
  struct timeval *tvp, tv;
  struct ares_addr_node *srvr, *servers = NULL;

#ifdef USE_WINSOCK
  WORD wVersionRequested = MAKEWORD(USE_WINSOCK,USE_WINSOCK);
  WSADATA wsaData;
  WSAStartup(wVersionRequested, &wsaData);
#endif

  status = ares_library_init(ARES_LIB_INIT_ALL);
  if (status != ARES_SUCCESS)
    {
      fprintf(stderr, "ares_library_init: %s\n", ares_strerror(status));
      return 1;
    }

  options.flags = ARES_FLAG_NOCHECKRESP;
  options.servers = NULL;
  options.nservers = 0;
  while ((c = ares_getopt(argc, argv, "df:s:c:t:T:U:")) != -1)
    {
      switch (c)
        {
        case 'd':
#ifdef WATT32
          dbug_init();
#endif
          break;

        case 'f':
          /* Add a flag. */
          for (i = 0; i < nflags; i++)
            {
              if (strcmp(flags[i].name, optarg) == 0)
                break;
            }
          if (i < nflags)
            options.flags |= flags[i].value;
          else
            usage();
          break;

        case 's':
          /* User-specified name servers override default ones. */
          srvr = malloc(sizeof(struct ares_addr_node));
          if (!srvr)
            {
              fprintf(stderr, "Out of memory!\n");
              destroy_addr_list(servers);
              return 1;
            }
          append_addr_list(&servers, srvr);
          if (ares_inet_pton(AF_INET, optarg, &srvr->addr.addr4) > 0)
            srvr->family = AF_INET;
          else if (ares_inet_pton(AF_INET6, optarg, &srvr->addr.addr6) > 0)
            srvr->family = AF_INET6;
          else
            {
              hostent = gethostbyname(optarg);
              if (!hostent)
                {
                  fprintf(stderr, "adig: server %s not found.\n", optarg);
                  destroy_addr_list(servers);
                  return 1;
                }
              switch (hostent->h_addrtype)
                {
                  case AF_INET:
                    srvr->family = AF_INET;
                    memcpy(&srvr->addr.addr4, hostent->h_addr,
                           sizeof(srvr->addr.addr4));
                    break;
                  case AF_INET6:
                    srvr->family = AF_INET6;
                    memcpy(&srvr->addr.addr6, hostent->h_addr,
                           sizeof(srvr->addr.addr6));
                    break;
                  default:
                    fprintf(stderr,
                      "adig: server %s unsupported address family.\n", optarg);
                    destroy_addr_list(servers);
                    return 1;
                }
            }
          /* Notice that calling ares_init_options() without servers in the
           * options struct and with ARES_OPT_SERVERS set simultaneously in
           * the options mask, results in an initialization with no servers.
           * When alternative name servers have been specified these are set
           * later calling ares_set_servers() overriding any existing server
           * configuration. To prevent initial configuration with default
           * servers that will be discarded later, ARES_OPT_SERVERS is set.
           * If this flag is not set here the result shall be the same but
           * ares_init_options() will do needless work. */
          optmask |= ARES_OPT_SERVERS;
          break;

        case 'c':
          /* Set the query class. */
          for (i = 0; i < nclasses; i++)
            {
              if (strcasecmp(classes[i].name, optarg) == 0)
                break;
            }
          if (i < nclasses)
            dnsclass = classes[i].value;
          else
            usage();
          break;

        case 't':
          /* Set the query type. */
          for (i = 0; i < ntypes; i++)
            {
              if (strcasecmp(types[i].name, optarg) == 0)
                break;
            }
          if (i < ntypes)
            type = types[i].value;
          else
            usage();
          break;

        case 'T':
          /* Set the TCP port number. */
          if (!ISDIGIT(*optarg))
            usage();
          options.tcp_port = (unsigned short)strtol(optarg, NULL, 0);
          optmask |= ARES_OPT_TCP_PORT;
          break;

        case 'U':
          /* Set the UDP port number. */
          if (!ISDIGIT(*optarg))
            usage();
          options.udp_port = (unsigned short)strtol(optarg, NULL, 0);
          optmask |= ARES_OPT_UDP_PORT;
          break;
        }
    }
  argc -= optind;
  argv += optind;
  if (argc == 0)
    usage();

  status = ares_init_options(&channel, &options, optmask);

  if (status != ARES_SUCCESS)
    {
      fprintf(stderr, "ares_init_options: %s\n",
              ares_strerror(status));
      return 1;
    }

  if(servers)
    {
      status = ares_set_servers(channel, servers);
      destroy_addr_list(servers);
      if (status != ARES_SUCCESS)
        {
          fprintf(stderr, "ares_init_options: %s\n",
                  ares_strerror(status));
          return 1;
        }
    }

  /* Initiate the queries, one per command-line argument.  If there is
   * only one query to do, supply NULL as the callback argument;
   * otherwise, supply the query name as an argument so we can
   * distinguish responses for the user when printing them out.
   */
  if (argc == 1)
    ares_query(channel, *argv, dnsclass, type, callback, (char *) NULL);
  else
    {
      for (; *argv; argv++)
        ares_query(channel, *argv, dnsclass, type, callback, *argv);
    }

  /* Wait for all queries to complete. */
  for (;;)
    {
      FD_ZERO(&read_fds);
      FD_ZERO(&write_fds);
      nfds = ares_fds(channel, &read_fds, &write_fds);
      if (nfds == 0)
        break;
      tvp = ares_timeout(channel, NULL, &tv);
      count = select(nfds, &read_fds, &write_fds, NULL, tvp);
      if (count < 0 && (status = SOCKERRNO) != EINVAL)
        {
          printf("select fail: %d", status);
          return 1;
        }
      ares_process(channel, &read_fds, &write_fds);
    }

  ares_destroy(channel);

  ares_library_cleanup();

#ifdef USE_WINSOCK
  WSACleanup();
#endif

  return 0;
}

static void callback(void *arg, int status, int timeouts,
                     unsigned char *abuf, int alen)
{
  char *name = (char *) arg;
  int id, qr, opcode, aa, tc, rd, ra, rcode;
  unsigned int qdcount, ancount, nscount, arcount, i;
  const unsigned char *aptr;

  (void) timeouts;

  /* Display the query name if given. */
  if (name)
    printf("Answer for query %s:\n", name);

  /* Display an error message if there was an error, but only stop if
   * we actually didn't get an answer buffer.
   */
  if (status != ARES_SUCCESS)
    {
      printf("%s\n", ares_strerror(status));
      if (!abuf)
        return;
    }

  /* Won't happen, but check anyway, for safety. */
  if (alen < HFIXEDSZ)
    return;

  /* Parse the answer header. */
  id = DNS_HEADER_QID(abuf);
  qr = DNS_HEADER_QR(abuf);
  opcode = DNS_HEADER_OPCODE(abuf);
  aa = DNS_HEADER_AA(abuf);
  tc = DNS_HEADER_TC(abuf);
  rd = DNS_HEADER_RD(abuf);
  ra = DNS_HEADER_RA(abuf);
  rcode = DNS_HEADER_RCODE(abuf);
  qdcount = DNS_HEADER_QDCOUNT(abuf);
  ancount = DNS_HEADER_ANCOUNT(abuf);
  nscount = DNS_HEADER_NSCOUNT(abuf);
  arcount = DNS_HEADER_ARCOUNT(abuf);

  /* Display the answer header. */
  printf("id: %d\n", id);
  printf("flags: %s%s%s%s%s\n",
         qr ? "qr " : "",
         aa ? "aa " : "",
         tc ? "tc " : "",
         rd ? "rd " : "",
         ra ? "ra " : "");
  printf("opcode: %s\n", opcodes[opcode]);
  printf("rcode: %s\n", rcodes[rcode]);

  /* Display the questions. */
  printf("Questions:\n");
  aptr = abuf + HFIXEDSZ;
  for (i = 0; i < qdcount; i++)
    {
      aptr = display_question(aptr, abuf, alen);
      if (aptr == NULL)
        return;
    }

  /* Display the answers. */
  printf("Answers:\n");
  for (i = 0; i < ancount; i++)
    {
      aptr = display_rr(aptr, abuf, alen);
      if (aptr == NULL)
        return;
    }

  /* Display the NS records. */
  printf("NS records:\n");
  for (i = 0; i < nscount; i++)
    {
      aptr = display_rr(aptr, abuf, alen);
      if (aptr == NULL)
        return;
    }

  /* Display the additional records. */
  printf("Additional records:\n");
  for (i = 0; i < arcount; i++)
    {
      aptr = display_rr(aptr, abuf, alen);
      if (aptr == NULL)
        return;
    }
}

static const unsigned char *display_question(const unsigned char *aptr,
                                             const unsigned char *abuf,
                                             int alen)
{
  char *name;
  int type, dnsclass, status;
  long len;

  /* Parse the question name. */
  status = ares_expand_name(aptr, abuf, alen, &name, &len);
  if (status != ARES_SUCCESS)
    return NULL;
  aptr += len;

  /* Make sure there's enough data after the name for the fixed part
   * of the question.
   */
  if (aptr + QFIXEDSZ > abuf + alen)
    {
      ares_free_string(name);
      return NULL;
    }

  /* Parse the question type and class. */
  type = DNS_QUESTION_TYPE(aptr);
  dnsclass = DNS_QUESTION_CLASS(aptr);
  aptr += QFIXEDSZ;

  /* Display the question, in a format sort of similar to how we will
   * display RRs.
   */
  printf("\t%-15s.\t", name);
  if (dnsclass != C_IN)
    printf("\t%s", class_name(dnsclass));
  printf("\t%s\n", type_name(type));
  ares_free_string(name);
  return aptr;
}

static const unsigned char *display_rr(const unsigned char *aptr,
                                       const unsigned char *abuf, int alen)
{
  const unsigned char *p;
  int type, dnsclass, ttl, dlen, status;
  long len;
  char addr[46];
  union {
    unsigned char * as_uchar;
             char * as_char;
  } name;

  /* Parse the RR name. */
  status = ares_expand_name(aptr, abuf, alen, &name.as_char, &len);
  if (status != ARES_SUCCESS)
    return NULL;
  aptr += len;

  /* Make sure there is enough data after the RR name for the fixed
   * part of the RR.
   */
  if (aptr + RRFIXEDSZ > abuf + alen)
    {
      ares_free_string(name.as_char);
      return NULL;
    }

  /* Parse the fixed part of the RR, and advance to the RR data
   * field. */
  type = DNS_RR_TYPE(aptr);
  dnsclass = DNS_RR_CLASS(aptr);
  ttl = DNS_RR_TTL(aptr);
  dlen = DNS_RR_LEN(aptr);
  aptr += RRFIXEDSZ;
  if (aptr + dlen > abuf + alen)
    {
      ares_free_string(name.as_char);
      return NULL;
    }

  /* Display the RR name, class, and type. */
  printf("\t%-15s.\t%d", name.as_char, ttl);
  if (dnsclass != C_IN)
    printf("\t%s", class_name(dnsclass));
  printf("\t%s", type_name(type));
  ares_free_string(name.as_char);

  /* Display the RR data.  Don't touch aptr. */
  switch (type)
    {
    case T_CNAME:
    case T_MB:
    case T_MD:
    case T_MF:
    case T_MG:
    case T_MR:
    case T_NS:
    case T_PTR:
      /* For these types, the RR data is just a domain name. */
      status = ares_expand_name(aptr, abuf, alen, &name.as_char, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      printf("\t%s.", name.as_char);
      ares_free_string(name.as_char);
      break;

    case T_HINFO:
      /* The RR data is two length-counted character strings. */
      p = aptr;
      len = *p;
      if (p + len + 1 > aptr + dlen)
        return NULL;
      status = ares_expand_string(p, abuf, alen, &name.as_uchar, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      printf("\t%s", name.as_char);
      ares_free_string(name.as_char);
      p += len;
      len = *p;
      if (p + len + 1 > aptr + dlen)
        return NULL;
      status = ares_expand_string(p, abuf, alen, &name.as_uchar, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      printf("\t%s", name.as_char);
      ares_free_string(name.as_char);
      break;

    case T_MINFO:
      /* The RR data is two domain names. */
      p = aptr;
      status = ares_expand_name(p, abuf, alen, &name.as_char, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      printf("\t%s.", name.as_char);
      ares_free_string(name.as_char);
      p += len;
      status = ares_expand_name(p, abuf, alen, &name.as_char, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      printf("\t%s.", name.as_char);
      ares_free_string(name.as_char);
      break;

    case T_MX:
      /* The RR data is two bytes giving a preference ordering, and
       * then a domain name.
       */
      if (dlen < 2)
        return NULL;
      printf("\t%d", (int)DNS__16BIT(aptr));
      status = ares_expand_name(aptr + 2, abuf, alen, &name.as_char, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      printf("\t%s.", name.as_char);
      ares_free_string(name.as_char);
      break;

    case T_SOA:
      /* The RR data is two domain names and then five four-byte
       * numbers giving the serial number and some timeouts.
       */
      p = aptr;
      status = ares_expand_name(p, abuf, alen, &name.as_char, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      printf("\t%s.\n", name.as_char);
      ares_free_string(name.as_char);
      p += len;
      status = ares_expand_name(p, abuf, alen, &name.as_char, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      printf("\t\t\t\t\t\t%s.\n", name.as_char);
      ares_free_string(name.as_char);
      p += len;
      if (p + 20 > aptr + dlen)
        return NULL;
      printf("\t\t\t\t\t\t( %u %u %u %u %u )",
             DNS__32BIT(p), DNS__32BIT(p+4),
             DNS__32BIT(p+8), DNS__32BIT(p+12),
             DNS__32BIT(p+16));
      break;

    case T_TXT:
      /* The RR data is one or more length-counted character
       * strings. */
      p = aptr;
      while (p < aptr + dlen)
        {
          len = *p;
          if (p + len + 1 > aptr + dlen)
            return NULL;
          status = ares_expand_string(p, abuf, alen, &name.as_uchar, &len);
          if (status != ARES_SUCCESS)
            return NULL;
          printf("\t%s", name.as_char);
          ares_free_string(name.as_char);
          p += len;
        }
      break;

    case T_A:
      /* The RR data is a four-byte Internet address. */
      if (dlen != 4)
        return NULL;
      printf("\t%s", ares_inet_ntop(AF_INET,aptr,addr,sizeof(addr)));
      break;

    case T_AAAA:
      /* The RR data is a 16-byte IPv6 address. */
      if (dlen != 16)
        return NULL;
      printf("\t%s", ares_inet_ntop(AF_INET6,aptr,addr,sizeof(addr)));
      break;

    case T_WKS:
      /* Not implemented yet */
      break;

    case T_SRV:
      /* The RR data is three two-byte numbers representing the
       * priority, weight, and port, followed by a domain name.
       */

      printf("\t%d", (int)DNS__16BIT(aptr));
      printf(" %d", (int)DNS__16BIT(aptr + 2));
      printf(" %d", (int)DNS__16BIT(aptr + 4));

      status = ares_expand_name(aptr + 6, abuf, alen, &name.as_char, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      printf("\t%s.", name.as_char);
      ares_free_string(name.as_char);
      break;

    case T_NAPTR:

      printf("\t%d", (int)DNS__16BIT(aptr)); /* order */
      printf(" %d\n", (int)DNS__16BIT(aptr + 2)); /* preference */

      p = aptr + 4;
      status = ares_expand_string(p, abuf, alen, &name.as_uchar, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      printf("\t\t\t\t\t\t%s\n", name.as_char);
      ares_free_string(name.as_char);
      p += len;

      status = ares_expand_string(p, abuf, alen, &name.as_uchar, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      printf("\t\t\t\t\t\t%s\n", name.as_char);
      ares_free_string(name.as_char);
      p += len;

      status = ares_expand_string(p, abuf, alen, &name.as_uchar, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      printf("\t\t\t\t\t\t%s\n", name.as_char);
      ares_free_string(name.as_char);
      p += len;

      status = ares_expand_name(p, abuf, alen, &name.as_char, &len);
      if (status != ARES_SUCCESS)
        return NULL;
      printf("\t\t\t\t\t\t%s", name.as_char);
      ares_free_string(name.as_char);
      break;

    case T_DS:
    case T_SSHFP:
    case T_RRSIG:
    case T_NSEC:
    case T_DNSKEY:
      printf("\t[RR type parsing unavailable]");
      break;

    default:
      printf("\t[Unknown RR; cannot parse]");
      break;
    }
  printf("\n");

  return aptr + dlen;
}

static const char *type_name(int type)
{
  int i;

  for (i = 0; i < ntypes; i++)
    {
      if (types[i].value == type)
        return types[i].name;
    }
  return "(unknown)";
}

static const char *class_name(int dnsclass)
{
  int i;

  for (i = 0; i < nclasses; i++)
    {
      if (classes[i].value == dnsclass)
        return classes[i].name;
    }
  return "(unknown)";
}

static void usage(void)
{
  fprintf(stderr, "usage: adig [-f flag] [-s server] [-c class] "
          "[-t type] [-p port] name ...\n");
  exit(1);
}

static void destroy_addr_list(struct ares_addr_node *head)
{
  while(head)
    {
      struct ares_addr_node *detached = head;
      head = head->next;
      free(detached);
    }
}

static void append_addr_list(struct ares_addr_node **head,
                             struct ares_addr_node *node)
{
  struct ares_addr_node *last;
  node->next = NULL;
  if(*head)
    {
      last = *head;
      while(last->next)
        last = last->next;
      last->next = node;
    }
  else
    *head = node;
}
