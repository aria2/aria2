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
#include "SessionSerializer.h"
#include "ServerStatMan.h"
#include "FileAllocationEntry.h"
#include "CheckIntegrityEntry.h"
#include "TimeA2.h"
#ifdef ENABLE_SSL
# include "SocketCore.h"
# include "TLSContext.h"
#endif // ENABLE_SSL

namespace aria2 {

namespace global {

extern volatile sig_atomic_t globalHaltRequested;

} // namespace global

static void handler(int signal) {
  if(
#ifdef SIGHUP
     signal == SIGHUP ||
#endif // SIGHUP
     signal == SIGTERM) {
    if(global::globalHaltRequested == 0 || global::globalHaltRequested == 2) {
      global::globalHaltRequested = 3;
    }
  } else {
    if(global::globalHaltRequested == 0) {
      global::globalHaltRequested = 1;
    } else if(global::globalHaltRequested == 2) {
      global::globalHaltRequested = 3;
    }
  }
}

MultiUrlRequestInfo::MultiUrlRequestInfo
(const std::vector<SharedHandle<RequestGroup> >& requestGroups,
 const SharedHandle<Option>& op,
 const SharedHandle<StatCalc>& statCalc,
 std::ostream& summaryOut)
  :
  requestGroups_(requestGroups),
  option_(op),
  statCalc_(statCalc),
  summaryOut_(summaryOut),
  logger_(LogFactory::getInstance())
{}

MultiUrlRequestInfo::~MultiUrlRequestInfo() {}

void MultiUrlRequestInfo::printMessageForContinue()
{
  summaryOut_ << "\n"
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
      DownloadEngineFactory().newDownloadEngine(option_.get(), requestGroups_);

    if(!option_->blank(PREF_LOAD_COOKIES)) {
      File cookieFile(option_->get(PREF_LOAD_COOKIES));
      if(cookieFile.isFile()) {
        e->getCookieStorage()->load(cookieFile.getPath(), Time().getTime());
        logger_->info("Loaded cookies from '%s'.",
                      cookieFile.getPath().c_str());
      } else {
        logger_->error(MSG_LOADING_COOKIE_FAILED, cookieFile.getPath().c_str());
      }
    }

    SharedHandle<AuthConfigFactory> authConfigFactory(new AuthConfigFactory());
    File netrccf(option_->get(PREF_NETRC_PATH));
    if(!option_->getAsBool(PREF_NO_NETRC) && netrccf.isFile()) {
      mode_t mode = netrccf.mode();
      if(mode&(S_IRWXG|S_IRWXO)) {
        logger_->notice(MSG_INCORRECT_NETRC_PERMISSION,
                        option_->get(PREF_NETRC_PATH).c_str());
      } else {
        SharedHandle<Netrc> netrc(new Netrc());
        netrc->parse(option_->get(PREF_NETRC_PATH));
        authConfigFactory->setNetrc(netrc);
      }
    }
    e->setAuthConfigFactory(authConfigFactory);

#ifdef ENABLE_SSL
    SharedHandle<TLSContext> tlsContext(new TLSContext());
    if(!option_->blank(PREF_CERTIFICATE) &&
       !option_->blank(PREF_PRIVATE_KEY)) {
      tlsContext->addClientKeyFile(option_->get(PREF_CERTIFICATE),
                                   option_->get(PREF_PRIVATE_KEY));
    }
    if(!option_->blank(PREF_CA_CERTIFICATE)) {
      if(!tlsContext->addTrustedCACertFile(option_->get(PREF_CA_CERTIFICATE))) {
        logger_->info(MSG_WARN_NO_CA_CERT);
      }
    } else if(option_->getAsBool(PREF_CHECK_CERTIFICATE)) {
      logger_->info(MSG_WARN_NO_CA_CERT);
    }
    if(option_->getAsBool(PREF_CHECK_CERTIFICATE)) {
      tlsContext->enablePeerVerification();
    }
    SocketCore::setTLSContext(tlsContext);
#endif
    if(!Timer::monotonicClock()) {
      logger_->warn("Don't change system time while aria2c is running."
                    " Doing this may make aria2c hang for long time.");
    }

    std::string serverStatIf = option_->get(PREF_SERVER_STAT_IF);
    if(!serverStatIf.empty()) {
      e->getRequestGroupMan()->loadServerStat(serverStatIf);
      e->getRequestGroupMan()->removeStaleServerStat
        (option_->getAsInt(PREF_SERVER_STAT_TIMEOUT));
    }
    e->setStatCalc(statCalc_);
#ifdef SIGHUP
    util::setGlobalSignalHandler(SIGHUP, handler, 0);
#endif // SIGHUP
    util::setGlobalSignalHandler(SIGINT, handler, 0);
    util::setGlobalSignalHandler(SIGTERM, handler, 0);
    
    e->run();
    
    if(!option_->blank(PREF_SAVE_COOKIES)) {
      e->getCookieStorage()->saveNsFormat(option_->get(PREF_SAVE_COOKIES));
    }

    const std::string& serverStatOf = option_->get(PREF_SERVER_STAT_OF);
    if(!serverStatOf.empty()) {
      e->getRequestGroupMan()->saveServerStat(serverStatOf);
    }
    e->getRequestGroupMan()->showDownloadResults(summaryOut_);
    summaryOut_ << std::flush;

    RequestGroupMan::DownloadStat s =
      e->getRequestGroupMan()->getDownloadStat();
    if(!s.allCompleted()) {
      printMessageForContinue();
      if(s.getLastErrorResult() == downloadresultcode::FINISHED &&
         s.getInProgress() > 0) {
        returnValue = downloadresultcode::IN_PROGRESS;
      } else {
        returnValue = s.getLastErrorResult();
      }
    }
    SessionSerializer sessionSerializer(e->getRequestGroupMan());
    // TODO Add option: --save-session-status=error,inprogress,waiting
    if(!option_->blank(PREF_SAVE_SESSION)) {
      const std::string& filename = option_->get(PREF_SAVE_SESSION);
      if(sessionSerializer.save(filename)) {
        logger_->notice("Serialized session to '%s' successfully.",
                        filename.c_str());
      } else {
        logger_->notice("Failed to serialize session to '%s'.",
                        filename.c_str());
      }
    }
  } catch(RecoverableException& e) {
    if(returnValue == downloadresultcode::FINISHED) {
      returnValue = downloadresultcode::UNKNOWN_ERROR;
    }
    logger_->error(EX_EXCEPTION_CAUGHT, e);
  }
#ifdef SIGHUP
  util::setGlobalSignalHandler(SIGHUP, SIG_DFL, 0);
#endif // SIGHUP
  util::setGlobalSignalHandler(SIGINT, SIG_DFL, 0);
  util::setGlobalSignalHandler(SIGTERM, SIG_DFL, 0);
  return returnValue;
}

} // namespace aria2
