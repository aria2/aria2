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

#include <iostream>
#include <iterator>
#include <algorithm>

#include "SharedHandle.h"
#include "a2io.h"
#include "FeatureConfig.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "MessageDigest.h"
#endif // ENABLE_MESSAGE_DIGEST
#include "help_tags.h"
#include "prefs.h"
#include "fmt.h"
#include "OptionParser.h"
#include "OptionHandler.h"
#include "util.h"

namespace aria2 {

void showVersion() {
  std::cout << PACKAGE << _(" version ") << PACKAGE_VERSION << "\n"
            << "Copyright (C) 2006, 2012 Tatsuhiro Tsujikawa" << "\n"
            << "\n"
            <<
    _("This program is free software; you can redistribute it and/or modify\n"
      "it under the terms of the GNU General Public License as published by\n"
      "the Free Software Foundation; either version 2 of the License, or\n"
      "(at your option) any later version.\n"
      "\n"
      "This program is distributed in the hope that it will be useful,\n"
      "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
      "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
      "GNU General Public License for more details.\n")
            << "\n"
            << _("** Configuration **") << "\n"
            << _("Enabled Features") << ": "
            << FeatureConfig::getInstance()->featureSummary() << "\n"
#ifdef ENABLE_MESSAGE_DIGEST
            << _("Hash Algorithms") << ": "
            << MessageDigest::getSupportedHashTypeString() << "\n"
#endif // ENABLE_MESSAGE_DIGEST
            << "\n"
            << fmt(_("Report bugs to %s"), PACKAGE_BUGREPORT) << "\n"
            << _("Visit") << " " << PACKAGE_URL << std::endl;
}

void showUsage
(const std::string& keyword,
 const SharedHandle<OptionParser>& oparser,
 const Console& out) {
  out->printf(_("Usage: aria2c [OPTIONS] [URI | MAGNET | TORRENT_FILE |"
                " METALINK_FILE]..."));
  out->printf("\n");
  if(keyword.empty()) {
    // Very short version of usage.
    out->printf(_("See 'aria2c -h'."));
    out->printf("\n");
    return;
  } else if(keyword[0] == '#') {
    std::vector<SharedHandle<OptionHandler> > handlers =
      keyword == TAG_ALL ? oparser->findAll() : oparser->findByTag(keyword);
    if(keyword == TAG_ALL) {
      out->printf(_("Printing all options."));
    } else {
      out->printf(_("Printing options tagged with '%s'."),
                  keyword.c_str());
      out->printf("\n");
      out->printf(_("See 'aria2c -h#help' to know all available tags."));
    }
    out->printf("\n");
    out->printf(_("Options:"));
    out->printf("\n");
    for(std::vector<SharedHandle<OptionHandler> >::const_iterator i =
          handlers.begin(), eoi = handlers.end(); i != eoi; ++i) {
      write(out, *(*i));
      out->printf("\n");
    }
  } else {    
    std::vector<SharedHandle<OptionHandler> > handlers =
      oparser->findByNameSubstring(keyword);
    if(!handlers.empty()) {
      out->printf(_("Printing options whose name includes '%s'."),
                  keyword.c_str());
      out->printf("\n");
      out->printf(_("Options:"));
      out->printf("\n");
      for(std::vector<SharedHandle<OptionHandler> >::const_iterator i =
            handlers.begin(), eoi = handlers.end(); i != eoi; ++i) {
        write(out, *(*i));
        out->printf("\n");
      }
    } else {
      out->printf(_("No option matching with '%s'."),
                  keyword.c_str());
      out->printf("\n");
      write(out, *oparser->find(PREF_HELP));
    }
  }
  if(keyword == TAG_BASIC) {
    out->printf("URI, MAGNET, TORRENT_FILE, METALINK_FILE:\n");
    out->printf(_(" You can specify multiple HTTP(S)/FTP URIs. Unless you specify -Z option, all\n"
                  " URIs must point to the same file or downloading will fail."));
    out->printf("\n");
    out->printf(_(" You can also specify arbitrary number of BitTorrent Magnet URIs, torrent/\n"
                  " metalink files stored in a local drive. Please note that they are always\n"
                  " treated as a separate download."));
    out->printf("\n\n");
    out->printf(_(" You can specify both torrent file with -T option and URIs. By doing this,\n"
                  " download a file from both torrent swarm and HTTP/FTP server at the same time,\n"
                  " while the data from HTTP/FTP are uploaded to the torrent swarm. For single file\n"
                  " torrents, URI can be a complete URI pointing to the resource or if URI ends\n"
                  " with '/', 'name' in torrent file is added. For multi-file torrents, 'name' and\n"
                  " 'path' in torrent are added to form a URI for each file."));
    out->printf("\n\n");
    out->printf(_(" Make sure that URI is quoted with single(\') or double(\") quotation if it\n"
                  " contains \"&\" or any characters that have special meaning in shell."));
    out->printf("\n\n");
    out->printf(_("About the number of connections\n"
                " Since 1.10.0 release, aria2 uses 1 connection per host by default and has 20MiB\n"
                " segment size restriction. So whatever value you specify using -s option, it\n"
                " uses 1 connection per host. To make it behave like 1.9.x, use\n"
                " --max-connection-per-server=4 --min-split-size=1M.\n"
                "\n"));
  }
  out->printf(_("Refer to man page for more information."));
  out->printf("\n");
}

} // namespace aria2
