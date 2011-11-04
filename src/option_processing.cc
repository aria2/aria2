/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
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
#include "common.h"

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iostream>

#include "Option.h"
#include "prefs.h"
#include "OptionParser.h"
#include "OptionHandlerFactory.h"
#include "OptionHandler.h"
#include "util.h"
#include "message.h"
#include "Exception.h"
#include "a2io.h"
#include "help_tags.h"
#include "File.h"
#include "fmt.h"
#include "OptionHandlerException.h"
#include "error_code.h"
#include "SimpleRandomizer.h"
#include "bittorrent_helper.h"
#include "BufferedFile.h"
#include "console.h"
#include "array_fun.h"
#ifndef HAVE_DAEMON
#include "daemon.h"
#endif // !HAVE_DAEMON

namespace aria2 {

extern void showVersion();
extern void showUsage
(const std::string& keyword, const SharedHandle<OptionParser>& oparser);

namespace {
void overrideWithEnv
(Option& op,
 const SharedHandle<OptionParser>& optionParser,
 const Pref* pref,
 const std::string& envName)
{
  char* value = getenv(envName.c_str());
  if(value) {
    try {
      optionParser->find(pref)->parse(op, value);
    } catch(Exception& e) {
      global::cerr()->printf
        ("Caught Error while parsing environment variable '%s'\n%s\n",
         envName.c_str(),
         e.stackTrace().c_str());
    }
  }
}
} // namespace

void option_processing(Option& op, std::vector<std::string>& uris,
                       int argc, char* argv[])
{
  const SharedHandle<OptionParser>& oparser = OptionParser::getInstance();
  try {
    bool noConf = false;
    std::string ucfname;
    std::stringstream cmdstream;
    oparser->parseArg(cmdstream, uris, argc, argv);
    {
      // first evaluate --no-conf and --conf-path options.
      Option op;
      oparser->parse(op, cmdstream);
      noConf = op.getAsBool(PREF_NO_CONF);
      ucfname = op.get(PREF_CONF_PATH);

      if(op.defined(PREF_VERSION)) {
        showVersion();
        exit(error_code::FINISHED);
      }
      if(op.defined(PREF_HELP)) {
        std::string keyword;
        if(op.get(PREF_HELP).empty()) {
          keyword = TAG_BASIC;
        } else {
          keyword = op.get(PREF_HELP);
          const char A2_HH[] = "--";
          if(util::startsWith(keyword.begin(), keyword.end(),
                              A2_HH, vend(A2_HH)-1)) {
            keyword = keyword.substr(2);
          }
          std::string::size_type eqpos = keyword.find("=");
          if(eqpos != std::string::npos) {
            keyword = keyword.substr(0, eqpos);
          }
        }
        showUsage(keyword, oparser);
        exit(error_code::FINISHED);
      }
    }

    oparser->parseDefaultValues(op);

    if(!noConf) {
      std::string cfname = 
        ucfname.empty() ?
        oparser->find(PREF_CONF_PATH)->getDefaultValue() : ucfname;

      if(File(cfname).isFile()) {
        std::stringstream ss;
        {
          BufferedFile fp(cfname, BufferedFile::READ);
          if(fp) {
            fp.transfer(ss);
          }
        }
        try {
          oparser->parse(op, ss);
        } catch(OptionHandlerException& e) {
          global::cerr()->printf("Parse error in %s\n%s\n",
                                 cfname.c_str(),
                                 e.stackTrace().c_str());
          const SharedHandle<OptionHandler>& h = oparser->find(e.getPref());
          if(h) {
            global::cerr()->printf("Usage:\n%s\n", h->getDescription().c_str());
          }
          exit(e.getErrorCode());
        } catch(Exception& e) {
          global::cerr()->printf("Parse error in %s\n%s\n",
                                 cfname.c_str(),
                                 e.stackTrace().c_str());
          exit(e.getErrorCode());
        }
      } else if(!ucfname.empty()) {
        global::cerr()->printf("Configuration file %s is not found.\n",
                               cfname.c_str());
        showUsage(TAG_HELP, oparser);
        exit(error_code::UNKNOWN_ERROR);
      }
    }
    // Override configuration with environment variables.
    overrideWithEnv(op, oparser, PREF_HTTP_PROXY, "http_proxy");
    overrideWithEnv(op, oparser, PREF_HTTPS_PROXY, "https_proxy");
    overrideWithEnv(op, oparser, PREF_FTP_PROXY, "ftp_proxy");
    overrideWithEnv(op, oparser, PREF_ALL_PROXY, "all_proxy");
    overrideWithEnv(op, oparser, PREF_NO_PROXY, "no_proxy");

    // we must clear eof bit and seek to the beginning of the buffer.
    cmdstream.clear();
    cmdstream.seekg(0, std::ios::beg);
    // finaly let's parse and store command-iine options.
    oparser->parse(op, cmdstream);
#ifdef __MINGW32__
    for(std::map<std::string, std::string>::iterator i = op.begin();
        i != op.end(); ++i) {
      if(!util::isUtf8((*i).second)) {
        (*i).second = nativeToUtf8((*i).second);
      }
    }
#endif // __MINGW32__
  } catch(OptionHandlerException& e) {
    global::cerr()->printf("%s\n", e.stackTrace().c_str());
    const SharedHandle<OptionHandler>& h = oparser->find(e.getPref());
    if(h) {
      std::ostringstream ss;
      ss << *h;
      global::cerr()->printf("Usage:\n%s\n", ss.str().c_str());
    }
    exit(e.getErrorCode());
  } catch(Exception& e) {
    global::cerr()->printf("%s\n", e.stackTrace().c_str());
    showUsage(TAG_HELP, oparser);
    exit(e.getErrorCode());
  }
  if(!op.getAsBool(PREF_ENABLE_RPC) &&
#ifdef ENABLE_BITTORRENT
     op.blank(PREF_TORRENT_FILE) &&
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
     op.blank(PREF_METALINK_FILE) &&
#endif // ENABLE_METALINK
     op.blank(PREF_INPUT_FILE)) {
    if(uris.empty()) {
      global::cerr()->printf("%s\n", MSG_URI_REQUIRED);
      showUsage(TAG_HELP, oparser);
      exit(error_code::UNKNOWN_ERROR);
    }
  }
  if(op.getAsBool(PREF_DAEMON)) {
    if(daemon(0, 0) < 0) {
      perror(MSG_DAEMON_FAILED);
      exit(error_code::UNKNOWN_ERROR);
    }
  }
}

} // namespace aria2
