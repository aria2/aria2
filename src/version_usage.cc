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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#include "a2io.h"
#include "FeatureConfig.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "messageDigest.h"
#endif // ENABLE_MESSAGE_DIGEST
#include "TagContainer.h"
#include "HelpItem.h"
#include "HelpItemFactory.h"
#include "help_tags.h"
#include "prefs.h"
#include <iterator>

void showVersion() {
  cout << PACKAGE << _(" version ") << PACKAGE_VERSION << "\n"
       << "Copyright (C) 2006, 2008 Tatsuhiro Tsujikawa" << "\n"
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
      "GNU General Public License for more details.\n"
      "\n"
      "You should have received a copy of the GNU General Public License\n"
      "along with this program; if not, write to the Free Software\n"
      "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA\n")
       << "\n"

       <<
    "In addition, as a special exception, the copyright holders give\n"
    "permission to link the code of portions of this program with the\n"
    "OpenSSL library under certain conditions as described in each\n"
    "individual source file, and distribute linked combinations\n"
    "including the two.\n"
    "You must obey the GNU General Public License in all respects\n"
    "for all of the code used other than OpenSSL.  If you modify\n"
    "file(s) with this exception, you may extend this exception to your\n"
    "version of the file(s), but you are not obligated to do so.  If you\n"
    "do not wish to do so, delete this exception statement from your\n"
    "version.  If you delete this exception statement from all source\n"
    "files in the program, then also delete it here.\n"
       << "\n"
       << "** Configuration **" << "\n"
       << FeatureConfig::getInstance()->getConfigurationSummary()
#ifdef ENABLE_MESSAGE_DIGEST
       << "message digest algorithms: " << MessageDigestContext::getSupportedAlgoString() << "\n"
#endif // ENABLE_MESSAGE_DIGEST
       << "\n";
    printf(_("Report bugs to %s"), "<tujikawa at users dot sourceforge dot net>");
    cout << endl;
}

void showUsage(const string& category) {
  printf(_("Usage: %s [options] URL ...\n"), PACKAGE_NAME);
#ifdef ENABLE_BITTORRENT
  printf(_("       %s [options] -T TORRENT_FILE URL ...\n"), PACKAGE_NAME);
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  printf(_("       %s [options] -M METALINK_FILE\n"), PACKAGE_NAME);
#endif // ENABLE_METALINK
  cout << endl;

  TagContainerHandle tc = HelpItemFactory::createHelpItems();
  TaggedItems items = category == V_ALL ? tc->getAllItems() : tc->search(category);
  if(items.size() > 0) {
    if(category == V_ALL) {
      printf(_("Printing all options."));
    } else {
      printf(_("Printing options tagged with '%s'."), category.c_str());
      cout << "\n";
      printf(_("See -h option to know other command-line options(%s)."),
	       HelpItemHandle(tc->nameMatch("help"))->getAvailableValues().c_str());
    }
    cout << "\n";
    cout << _("Options:") << endl;
    copy(items.begin(), items.end(), ostream_iterator<HelpItemHandle>(cout, "\n"));
  } else {
    TaggedItems items = tc->nameMatchForward(category);
    if(items.size() > 0) {
      printf(_("Printing options whose name starts with '%s'."), category.c_str());
      cout << "\n";
      cout << _("Options:") << endl;
      copy(items.begin(), items.end(), ostream_iterator<HelpItemHandle>(cout, "\n"));
    } else {
      printf(_("No help category or option name matching with '%s'."), category.c_str());
      cout << "\n";
      cout << HelpItemHandle(tc->nameMatch("help")) << "\n";
    }
  }
  if(category == TAG_BASIC) {
    cout << "\n"
	 << "URL:" << "\n"
	 << _(" You can specify multiple URLs. Unless you specify -Z option, all URLs must\n"
	      " point to the same file or downloading will fail.") << "\n"
	 << _(" You can specify both torrent file with -T option and URLs. By doing this,\n"
	      " download a file from both torrent swarm and http/ftp server at the same time,\n"
	      " while the data from http/ftp are uploaded to the torrent swarm. Note that\n"
	      " only single file torrent can be integrated with http/ftp.") << "\n"
	 << "\n"
	 << _(" Make sure that URL is quoted with single(\') or double(\") quotation if it\n"
	      " contains \"&\" or any characters that have special meaning in shell.") << "\n";
  }
  cout << "\n";
  cout << _("Refer to man page for more information.") << endl;
}
