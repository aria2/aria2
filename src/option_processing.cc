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
#include "UnknownOptionException.h"
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
(const std::string& keyword,
 const SharedHandle<OptionParser>& oparser,
 const Console& out);

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
        (_("Caught Error while parsing environment variable '%s'"),
         envName.c_str());
      global::cerr()->printf("\n%s\n", e.stackTrace().c_str());
    }
  }
}
} // namespace

namespace {
// Calculates Damerau–Levenshtein distance between c-string a and b
// with given costs.  swapcost, subcost, addcost and delcost are cost
// to swap 2 adjacent characters, substitute characters, add character
// and delete character respectively.
int levenshtein
(const char* a,
 const char* b,
 int swapcost,
 int subcost,
 int addcost,
 int delcost)
{
  int alen = strlen(a);
  int blen = strlen(b);
  std::vector<std::vector<int> > dp(3, std::vector<int>(blen+1));
  for(int i = 0; i <= blen; ++i) {
    dp[1][i] = i;
  }
  for(int i = 1; i <= alen; ++i) {
    dp[0][0] = i;
    for(int j = 1; j <= blen; ++j) {
      dp[0][j] = dp[1][j-1]+(a[i-1] == b[j-1] ? 0 : subcost);
      if(i >= 2 && j >= 2 && a[i-1] != b[j-1] &&
         a[i-2] == b[j-1] && a[i-1] == b[j-2]) {
        dp[0][j] = std::min(dp[0][j], dp[2][j-2]+swapcost);
      }
      dp[0][j] = std::min(dp[0][j],
                          std::min(dp[1][j]+delcost, dp[0][j-1]+addcost));
    }
    std::rotate(dp.begin(), dp.begin()+2, dp.end());
  }
  return dp[1][blen];
}
} // namespace

namespace {

void showCandidates
(const std::string& unknownOption, const SharedHandle<OptionParser>& parser)
{
  const char* optstr = unknownOption.c_str();
  for(; *optstr == '-'; ++optstr);
  if(*optstr == '\0') {
    return;
  }
  int optstrlen = strlen(optstr);
  std::vector<std::pair<int, const Pref*> > cands;
  for(int i = 1, len = option::countOption(); i < len; ++i) {
    const Pref* pref = option::i2p(i);
    const SharedHandle<OptionHandler>& h = parser->find(pref);
    if(!h || h->isHidden()) {
      continue;
    }
    // Use cost 0 for prefix match
    if(util::startsWith(pref->k, pref->k+strlen(pref->k),
                        optstr, optstr+optstrlen)) {
      cands.push_back(std::make_pair(0, pref));
      continue;
    }
    // cost values are borrowed from git, help.c.
    int sim = levenshtein(optstr, pref->k, 0, 2, 1, 4);
    cands.push_back(std::make_pair(sim, pref));
  }
  if(cands.empty()) {
    return;
  }
  std::sort(cands.begin(), cands.end());
  int threshold = cands[0].first;
  // threshold value 7 is borrowed from git, help.c.
  if(threshold >= 7) {
    return;
  }
  global::cerr()->printf("\n");
  global::cerr()->printf(_("Did you mean:"));
  global::cerr()->printf("\n");
  for(std::vector<std::pair<int, const Pref*> >::iterator i = cands.begin(),
        eoi = cands.end(); i != eoi && (*i).first <= threshold; ++i) {
    global::cerr()->printf("\t--%s\n", (*i).second->k);
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
          if(util::startsWith(keyword, "--")) {
            keyword.erase(keyword.begin(), keyword.begin()+2);
          }
          std::string::size_type eqpos = keyword.find("=");
          if(eqpos != std::string::npos) {
            keyword.erase(keyword.begin()+eqpos, keyword.end());
          }
        }
        showUsage(keyword, oparser, global::cout());
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
          global::cerr()->printf(_("Parse error in %s"), cfname.c_str());
          global::cerr()->printf("\n%s", e.stackTrace().c_str());
          const SharedHandle<OptionHandler>& h = oparser->find(e.getPref());
          if(h) {
            global::cerr()->printf(_("Usage:"));
            global::cerr()->printf("\n%s\n", h->getDescription());
          }
          exit(e.getErrorCode());
        } catch(Exception& e) {
          global::cerr()->printf(_("Parse error in %s"), cfname.c_str());
          global::cerr()->printf("\n%s", e.stackTrace().c_str());
          exit(e.getErrorCode());
        }
      } else if(!ucfname.empty()) {
        global::cerr()->printf(_("Configuration file %s is not found."),
                               cfname.c_str());
        global::cerr()->printf("\n");
        showUsage(TAG_HELP, oparser, global::cerr());
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
    for(size_t i = 1, len = option::countOption(); i < len; ++i) {
      const Pref* pref = option::i2p(i);
      if(op.defined(pref) && !util::isUtf8(op.get(pref))) {
        op.put(pref, nativeToUtf8(op.get(pref)));
      }
    }
#endif // __MINGW32__
  } catch(OptionHandlerException& e) {
    global::cerr()->printf("%s", e.stackTrace().c_str());
    const SharedHandle<OptionHandler>& h = oparser->find(e.getPref());
    if(h) {
      global::cerr()->printf(_("Usage:"));
      global::cerr()->printf("\n");
      write(global::cerr(), *h);
    }
    exit(e.getErrorCode());
  } catch(UnknownOptionException& e) {
    showUsage("", oparser, global::cerr());
    showCandidates(e.getUnknownOption(), oparser);
    exit(e.getErrorCode());
  } catch(Exception& e) {
    global::cerr()->printf("%s", e.stackTrace().c_str());
    showUsage("", oparser, global::cerr());
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
      global::cerr()->printf(MSG_URI_REQUIRED);
      global::cerr()->printf("\n");
      showUsage("", oparser, global::cerr());
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
