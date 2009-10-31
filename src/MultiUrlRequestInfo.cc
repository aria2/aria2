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
#include "MultiUrlRequestInfo.h"

#include <signal.h>

#include <ostream>

#include "RequestGroupMan.h"
#include "DownloadEngine.h"
#include "LogFactory.h"
#include "Logger.h"
#include "RequestGroup.h"
#include "prefs.h"
#include "DownloadEngineFactory.h"
#include "RecoverableException.h"
#include "message.h"
#include "util.h"
#include "Option.h"
#include "StatCalc.h"
#include "CookieStorage.h"
#include "File.h"
#include "Netrc.h"
#include "AuthConfigFactory.h"
#include "DownloadContext.h"
#ifdef ENABLE_SSL
# include "SocketCore.h"
# include "TLSContext.h"
#endif // ENABLE_SSL

namespace aria2 {

#ifndef SA_RESETHAND
# define SA_RESETHAND 0x80000000
#endif // SA_RESETHAND

extern volatile sig_atomic_t globalHaltRequested;

static void handler(int signal) {
  if(globalHaltRequested == 0) {
    globalHaltRequested = 1;
  } else if(globalHaltRequested == 2) {
    globalHaltRequested = 3;
  }
}

MultiUrlRequestInfo::MultiUrlRequestInfo
(const RequestGroups& requestGroups,
 const SharedHandle<Option>& op,
 const SharedHandle<StatCalc>& statCalc,
 std::ostream& summaryOut)
  :
  _requestGroups(requestGroups),
  _option(op),
  _statCalc(statCalc),
  _summaryOut(summaryOut),
  _logger(LogFactory::getInstance())
{}

MultiUrlRequestInfo::~MultiUrlRequestInfo() {}

void MultiUrlRequestInfo::printMessageForContinue()
{
  _summaryOut << "\n"
	      << _("aria2 will resume download if the transfer is restarted.")
	      << "\n"
	      << _("If there are any errors, then see the log file. See '-l' option in help/man page for details.")
	      << "\n";
}

downloadresultcode::RESULT MultiUrlRequestInfo::execute()
{
  downloadresultcode::RESULT returnValue = downloadresultcode::FINISHED;
  try {
    DownloadEngineHandle e =
      DownloadEngineFactory().newDownloadEngine(_option.get(), _requestGroups);

    if(!_option->blank(PREF_LOAD_COOKIES)) {
      File cookieFile(_option->get(PREF_LOAD_COOKIES));
      if(cookieFile.isFile()) {
	e->getCookieStorage()->load(_option->get(PREF_LOAD_COOKIES));
      } else {
	_logger->error(MSG_LOADING_COOKIE_FAILED,
		       _option->get(PREF_LOAD_COOKIES).c_str());
      }
    }

    SharedHandle<AuthConfigFactory> authConfigFactory(new AuthConfigFactory());
    File netrccf(_option->get(PREF_NETRC_PATH));
    if(!_option->getAsBool(PREF_NO_NETRC) && netrccf.isFile()) {
      mode_t mode = netrccf.mode();
      if(mode&(S_IRWXG|S_IRWXO)) {
	_logger->notice(MSG_INCORRECT_NETRC_PERMISSION,
			_option->get(PREF_NETRC_PATH).c_str());
      } else {
	SharedHandle<Netrc> netrc(new Netrc());
	netrc->parse(_option->get(PREF_NETRC_PATH));
	authConfigFactory->setNetrc(netrc);
      }
    }
    e->setAuthConfigFactory(authConfigFactory);

#ifdef ENABLE_SSL
    SharedHandle<TLSContext> tlsContext(new TLSContext());
    if(!_option->blank(PREF_CERTIFICATE) &&
       !_option->blank(PREF_PRIVATE_KEY)) {
      tlsContext->addClientKeyFile(_option->get(PREF_CERTIFICATE),
				   _option->get(PREF_PRIVATE_KEY));
    }
    if(!_option->blank(PREF_CA_CERTIFICATE)) {
      if(!tlsContext->addTrustedCACertFile(_option->get(PREF_CA_CERTIFICATE))) {
	_logger->warn(MSG_WARN_NO_CA_CERT);
      }
    } else if(_option->getAsBool(PREF_CHECK_CERTIFICATE)) {
      _logger->warn(MSG_WARN_NO_CA_CERT);
    }

    if(_option->getAsBool(PREF_CHECK_CERTIFICATE)) {
      tlsContext->enablePeerVerification();
    }
    SocketCore::setTLSContext(tlsContext);
#endif

    std::string serverStatIf = _option->get(PREF_SERVER_STAT_IF);
    if(!serverStatIf.empty()) {
      e->_requestGroupMan->loadServerStat(serverStatIf);
      e->_requestGroupMan->removeStaleServerStat
	(_option->getAsInt(PREF_SERVER_STAT_TIMEOUT));
    }
    e->setStatCalc(_statCalc);

    // The number of simultaneous download is specified by
    // PREF_MAX_CONCURRENT_DOWNLOADS.
    // The remaining urls are queued into FillRequestGroupCommand.
    // It observes the number of simultaneous downloads and if it is under
    // the limit, it adds RequestGroup object from its queue to DownloadEngine.
    // This is done every 1 second. At the same time, it removes finished/error
    // RequestGroup from DownloadEngine.

    util::setGlobalSignalHandler(SIGINT, handler, 0);
    util::setGlobalSignalHandler(SIGTERM, handler, 0);
    
    e->run();
    
    if(!_option->blank(PREF_SAVE_COOKIES)) {
      e->getCookieStorage()->saveNsFormat(_option->get(PREF_SAVE_COOKIES));
    }

    std::string serverStatOf = _option->get(PREF_SERVER_STAT_OF);
    if(!serverStatOf.empty()) {
      e->_requestGroupMan->saveServerStat(serverStatOf);
    }
    e->_requestGroupMan->showDownloadResults(_summaryOut);
    _summaryOut << std::flush;

    RequestGroupMan::DownloadStat s = e->_requestGroupMan->getDownloadStat();
    if(!s.allCompleted()) {
      printMessageForContinue();
      if(s.getLastErrorResult() == downloadresultcode::FINISHED &&
	 s.getInProgress() > 0) {
	returnValue = downloadresultcode::IN_PROGRESS;
      } else {
	returnValue = s.getLastErrorResult();
      }
    }
  } catch(RecoverableException& e) {
    _logger->error(EX_EXCEPTION_CAUGHT, e);
  }
  util::setGlobalSignalHandler(SIGINT, SIG_DFL, 0);
  util::setGlobalSignalHandler(SIGTERM, SIG_DFL, 0);
  return returnValue;
}

} // namespace aria2
