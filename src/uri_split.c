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
#include "uri_split.h"

#include <stdlib.h>

typedef enum {
  URI_BEFORE_SCHEME,
  URI_SCHEME,
  URI_SCHEME_SLASH1,
  URI_SCHEME_SLASH2,
  URI_BEFORE_MAYBE_USER,
  URI_MAYBE_USER,
  URI_BEFORE_MAYBE_PASSWD,
  URI_MAYBE_PASSWD,
  URI_BEFORE_HOST,
  URI_HOST,
  URI_BEFORE_IPV6HOST,
  URI_IPV6HOST,
  URI_AFTER_IPV6HOST,
  URI_BEFORE_PORT,
  URI_PORT,
  URI_PATH,
  URI_BEFORE_QUERY,
  URI_QUERY,
  URI_BEFORE_FRAGMENT,
  URI_FRAGMENT
} uri_split_state;

static void uri_set_field(uri_split_result* res, int field, const char* first,
                          const char* last, const char* uri)
{
  if (first) {
    res->field_set |= 1 << field;
    res->fields[field].off = first - uri;
    res->fields[field].len = last - first;
  }
}

static int is_digit(char c) { return '0' <= c && c <= '9'; }

int uri_split(uri_split_result* res, const char* uri)
{
  int state = URI_BEFORE_SCHEME;
  const char *scheme_first = NULL, *scheme_last = NULL, *host_first = NULL,
             *host_last = NULL, *path_first = NULL, *path_last = NULL,
             *query_first = NULL, *query_last = NULL, *fragment_first = NULL,
             *fragment_last = NULL, *user_first = NULL, *user_last = NULL,
             *passwd_first = NULL, *passwd_last = NULL, *last_atmark = NULL,
             *last_slash = NULL, *p = uri;
  int32_t port = -1;
  uint8_t flags = 0;

  for (; *p; ++p) {
    switch (state) {
    case URI_BEFORE_SCHEME:
      scheme_first = p;
      state = URI_SCHEME;
      break;
    case URI_SCHEME:
      if (*p == ':') {
        scheme_last = p;
        state = URI_SCHEME_SLASH1;
      }
      break;
    case URI_SCHEME_SLASH1:
      if (*p == '/') {
        state = URI_SCHEME_SLASH2;
      }
      else {
        return -1;
      }
      break;
    case URI_SCHEME_SLASH2:
      if (*p == '/') {
        state = URI_BEFORE_MAYBE_USER;
      }
      else {
        return -1;
      }
      break;
    case URI_BEFORE_MAYBE_USER:
      switch (*p) {
      case '@':
      case ':':
      case '/':
        return -1;
      case '[':
        state = URI_BEFORE_IPV6HOST;
        break;
      default:
        user_first = p;
        state = URI_MAYBE_USER;
      }
      break;
    case URI_MAYBE_USER:
      switch (*p) {
      case '@':
        last_atmark = p;
        break;
      case ':':
        user_last = p;
        state = URI_BEFORE_MAYBE_PASSWD;
        break;
      case '[':
        if (last_atmark == p - 1) {
          user_last = last_atmark;
          state = URI_BEFORE_IPV6HOST;
        }
        else {
          return -1;
        }
        break;
      case '/':
      case '?':
      case '#':
        /* It turns out that this is only host or user + host if
           last_atmark is not NULL. */
        if (last_atmark) {
          host_first = last_atmark + 1;
          host_last = p;
          user_last = last_atmark;
        }
        else {
          host_first = user_first;
          host_last = p;
          user_first = user_last = NULL;
        }
        switch (*p) {
        case '/':
          path_first = last_slash = p;
          state = URI_PATH;
          break;
        case '?':
          state = URI_BEFORE_QUERY;
          break;
        case '#':
          state = URI_BEFORE_FRAGMENT;
          break;
        }
        break;
      }
      break;
    case URI_BEFORE_MAYBE_PASSWD:
      passwd_first = p;
      switch (*p) {
      case '@':
        passwd_last = last_atmark = p;
        state = URI_BEFORE_HOST;
        break;
      case '/':
        return -1;
      default:
        /* sums up port number in case of port. */
        if (is_digit(*p)) {
          port = *p - '0';
        }
        state = URI_MAYBE_PASSWD;
      }
      break;
    case URI_MAYBE_PASSWD:
      switch (*p) {
      case '@':
        passwd_last = last_atmark = p;
        /* Passwd confirmed, reset port to -1. */
        port = -1;
        state = URI_BEFORE_HOST;
        break;
      case '[':
        return -1;
      case '/':
      case '?':
      case '#':
        /* This is port not password.  port is in [passwd_first, p) */
        if (port == -1) {
          return -1;
        }
        if (last_atmark) {
          host_first = last_atmark + 1;
          host_last = passwd_first - 1;
          user_last = last_atmark;
        }
        else {
          host_first = user_first;
          host_last = passwd_first - 1;
          user_first = user_last = NULL;
        }
        passwd_first = passwd_last = NULL;
        switch (*p) {
        case '/':
          path_first = last_slash = p;
          state = URI_PATH;
          break;
        case '?':
          state = URI_BEFORE_QUERY;
          break;
        case '#':
          state = URI_BEFORE_FRAGMENT;
          break;
        }
        break;
      default:
        if (port != -1) {
          if (is_digit(*p)) {
            port *= 10;
            port += *p - '0';
            if (port > UINT16_MAX) {
              port = -1;
            }
          }
          else {
            port = -1;
          }
        }
        break;
      }
      break;
    case URI_BEFORE_HOST:
      switch (*p) {
      case ':':
      case '/':
        return -1;
      case '[':
        state = URI_BEFORE_IPV6HOST;
        break;
      default:
        host_first = p;
        state = URI_HOST;
        break;
      }
      break;
    case URI_HOST:
      switch (*p) {
      case ':':
        host_last = p;
        state = URI_BEFORE_PORT;
        break;
      case '/':
        host_last = path_first = last_slash = p;
        state = URI_PATH;
        break;
      case '?':
        host_last = p;
        state = URI_BEFORE_QUERY;
        break;
      case '#':
        host_last = p;
        state = URI_BEFORE_FRAGMENT;
        break;
      }
      break;
    case URI_BEFORE_IPV6HOST:
      if (*p == ']') {
        return -1;
      }
      host_first = p;
      state = URI_IPV6HOST;
      break;
    case URI_IPV6HOST:
      if (*p == ']') {
        flags |= USF_IPV6ADDR;
        host_last = p;
        state = URI_AFTER_IPV6HOST;
      }
      break;
    case URI_AFTER_IPV6HOST:
      switch (*p) {
      case ':':
        state = URI_BEFORE_PORT;
        break;
      case '/':
        path_first = last_slash = p;
        state = URI_PATH;
        break;
      case '?':
        state = URI_BEFORE_QUERY;
        break;
      case '#':
        state = URI_BEFORE_FRAGMENT;
        break;
      default:
        return -1;
      }
      break;
    case URI_BEFORE_PORT:
      if (is_digit(*p)) {
        port = *p - '0';
        state = URI_PORT;
      }
      else {
        return -1;
      }
      break;
    case URI_PORT:
      switch (*p) {
      case '/':
        path_first = last_slash = p;
        state = URI_PATH;
        break;
      case '?':
        state = URI_BEFORE_QUERY;
        break;
      case '#':
        state = URI_BEFORE_FRAGMENT;
        break;
      default:
        if (is_digit(*p)) {
          port *= 10;
          port += *p - '0';
          if (port > UINT16_MAX) {
            return -1;
          }
        }
        else {
          return -1;
        }
      }
      break;
    case URI_PATH:
      switch (*p) {
      case '/':
        last_slash = p;
        break;
      case '?':
        path_last = p;
        state = URI_BEFORE_QUERY;
        break;
      case '#':
        path_last = p;
        state = URI_BEFORE_FRAGMENT;
        break;
      }
      break;
    case URI_BEFORE_QUERY:
      query_first = p;
      if (*p == '#') {
        query_last = p;
        state = URI_BEFORE_FRAGMENT;
      }
      else {
        state = URI_QUERY;
      }
      break;
    case URI_QUERY:
      if (*p == '#') {
        query_last = p;
        state = URI_BEFORE_FRAGMENT;
      }
      break;
    case URI_BEFORE_FRAGMENT:
      fragment_first = p;
      state = URI_FRAGMENT;
      break;
    case URI_FRAGMENT:
      break;
    }
  }
  /* Handle premature states */
  switch (state) {
  case URI_BEFORE_SCHEME:
  case URI_SCHEME:
  case URI_SCHEME_SLASH1:
  case URI_SCHEME_SLASH2:
    return -1;
  case URI_BEFORE_MAYBE_USER:
    return -1;
  case URI_MAYBE_USER:
    if (last_atmark) {
      host_first = last_atmark + 1;
      host_last = p;
      if (host_first == host_last) {
        return -1;
      }
      user_last = last_atmark;
    }
    else {
      host_first = user_first;
      host_last = p;
      user_first = user_last = NULL;
    }
    break;
  case URI_BEFORE_MAYBE_PASSWD:
    return -1;
  case URI_MAYBE_PASSWD:
    if (port == -1) {
      return -1;
    }
    if (last_atmark) {
      host_first = last_atmark + 1;
      host_last = passwd_first - 1;
      user_last = last_atmark;
    }
    else {
      host_first = user_first;
      host_last = passwd_first - 1;
      user_first = user_last = NULL;
    }
    passwd_first = passwd_last = NULL;
    break;
  case URI_BEFORE_HOST:
    return -1;
  case URI_HOST:
    host_last = p;
    break;
  case URI_BEFORE_IPV6HOST:
  case URI_IPV6HOST:
    return -1;
  case URI_AFTER_IPV6HOST:
    break;
  case URI_BEFORE_PORT:
    return -1;
  case URI_PORT:
    if (port == -1) {
      return -1;
    }
    break;
  case URI_PATH:
    path_last = p;
    break;
  case URI_BEFORE_QUERY:
    query_first = query_last = p;
    break;
  case URI_QUERY:
    query_last = p;
    break;
  case URI_BEFORE_FRAGMENT:
    fragment_first = fragment_last = p;
    break;
  case URI_FRAGMENT:
    fragment_last = p;
    break;
  default:
    return -1;
  };

  if (res) {
    res->field_set = 0;
    res->port = 0;
    res->flags = flags;

    uri_set_field(res, USR_SCHEME, scheme_first, scheme_last, uri);
    uri_set_field(res, USR_HOST, host_first, host_last, uri);
    uri_set_field(res, USR_PATH, path_first, path_last, uri);
    uri_set_field(res, USR_QUERY, query_first, query_last, uri);
    uri_set_field(res, USR_FRAGMENT, fragment_first, fragment_last, uri);
    uri_set_field(res, USR_USER, user_first, user_last, uri);
    uri_set_field(res, USR_PASSWD, passwd_first, passwd_last, uri);
    if (res->field_set & (1 << USR_USER)) {
      uri_set_field(res, USR_USERINFO, user_first, last_atmark, uri);
    }
    if (last_slash && last_slash + 1 != path_last) {
      uri_set_field(res, USR_BASENAME, last_slash + 1, path_last, uri);
    }
    if (port != -1) {
      res->field_set |= 1 << USR_PORT;
      res->port = port;
    }
  }

  return 0;
}
