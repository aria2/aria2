/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2012 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
#ifndef D_URI_SPLIT_H
#define D_URI_SPLIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <stdint.h>

typedef enum {
  USR_SCHEME,
  USR_HOST,
  USR_PORT,
  USR_PATH,
  USR_QUERY,
  USR_FRAGMENT,
  USR_USERINFO,
  USR_USER,
  USR_PASSWD,
  USR_BASENAME,
  USR_MAX
} uri_split_field;

typedef enum { USF_IPV6ADDR = 1 } uri_split_flag;

/* The structure is based on http-parser by Joyent, Inc and other Node
   contributors. https://github.com/joyent/http-parser */
typedef struct {
  uint16_t field_set;
  uint16_t port;

  struct {
    uint16_t off;
    uint16_t len;
  } fields[USR_MAX];

  uint8_t flags;
} uri_split_result;

/* Splits URI |uri| and stores the results in the |res|. To check
 * particular URI component is available, evaluate |res->field_set|
 * with 1 shifted by the field defined in uri_split_field. If the
 * |res| is NULL, processing is done but the result will not stored.
 * If the host component of the |uri| is IPv6 numeric address, then
 * USF_IPV6ADDR & res->flags will be nonzero.
 *
 * This function returns 0 if it succeeds, or -1.
 */
int uri_split(uri_split_result* res, const char* uri);

#ifdef __cplusplus
}
#endif

#endif /* D_URI_SPLIT_H */
