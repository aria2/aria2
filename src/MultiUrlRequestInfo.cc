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

#include <cstring>
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
#include "SessionSerializer.h"
#include "TimeA2.h"
#include "fmt.h"
#include "SocketCore.h"
#include "OutputFile.h"
#include "UriListParser.h"
#include "SingletonHolder.h"
#include "Notifier.h"
#ifdef ENABLE_WEBSOCKET
# include "WebSocketSessionMan.h"
#else // !ENABLE_WEBSOCKET
# include "NullWebSocketSessionMan.h"
#endif // !ENABLE_WEBSOCKET
#ifdef ENABLE_SSL
# include "TLSContext.h"
#endif // ENABLE_SSL
#ifdef ENABLE_ASYNC_DNS
#include "AsyncNameResolver.h"
#endif // ENABLE_ASYNC_DNS

namespace aria2 {

namespace global {

extern volatile sig_atomic_t globalHaltRequested;

} // namespace global

namespace {
void handler(int signal) {
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
} // namespace

MultiUrlRequestInfo::MultiUrlRequestInfo
(std::vector<SharedHandle<RequestGroup> >& requestGroups,
 const SharedHandle<Option>& op,
 const SharedHandle<StatCalc>& statCalc,
 const SharedHandle<OutputFile>& summaryOut,
 const SharedHandle<UriListParser>& uriListParser)
  : option_(op),
    statCalc_(statCalc),
    summaryOut_(summaryOut),
    uriListParser_(uriListParser),
    useSignalHandler_(true)
{
#ifdef HAVE_SIGACTION
  sigemptyset(&mask_);
#else // !HAVE_SIGACTION
  mask_ = 0;
#endif // !HAVE_SIGACTION
  requestGroups_.swap(requestGroups);
}

MultiUrlRequestInfo::~MultiUrlRequestInfo() {}

void MultiUrlRequestInfo::printMessageForContinue()
{
  summaryOut_->printf
    ("\n%s\n%s\n",
     _("aria2 will resume download if the transfer is restarted."),
     _("If there are any errors, then see the log file. See '-l' option in help/man page for details."));
}

int MultiUrlRequestInfo::prepare()
{
  global::globalHaltRequested = 0;
  try {
    SharedHandle<Notifier> notifier(new Notifier());
    SingletonHolder<Notifier>::instance(notifier);

#ifdef ENABLE_SSL
    if(option_->getAsBool(PREF_ENABLE_RPC) &&
       option_->getAsBool(PREF_RPC_SECURE)) {
      if(!option_->blank(PREF_RPC_CERTIFICATE)
#ifndef HAVE_APPLETLS
         && !option_->blank(PREF_RPC_PRIVATE_KEY)
#endif // HAVE_APPLETLS
         ) {
        // We set server TLS context to the SocketCore before creating
        // DownloadEngine instance.
        SharedHandle<TLSContext> svTlsContext(TLSContext::make(TLS_SERVER));
        svTlsContext->addCredentialFile(option_->get(PREF_RPC_CERTIFICATE),
                                        option_->get(PREF_RPC_PRIVATE_KEY));
        SocketCore::setServerTLSContext(svTlsContext);
      } else {
        throw DL_ABORT_EX("Specify --rpc-certificate and --rpc-private-key "
                          "options in order to use secure RPC.");
      }
    }
#endif // ENABLE_SSL

    e_ = DownloadEngineFactory().newDownloadEngine(option_.get(),
                                                   requestGroups_);
    // Avoid keeping RequestGroups alive longer than necessary
    requestGroups_.clear();

#ifdef ENABLE_WEBSOCKET
    if(option_->getAsBool(PREF_ENABLE_RPC)) {
      SharedHandle<rpc::WebSocketSessionMan> wsSessionMan
        (new rpc::WebSocketSessionMan());
      e_->setWebSocketSessionMan(wsSessionMan);
      SingletonHolder<Notifier>::instance()->addDownloadEventListener
        (wsSessionMan);
    }
#endif // ENABLE_WEBSOCKET

    if(!option_->blank(PREF_LOAD_COOKIES)) {
      File cookieFile(option_->get(PREF_LOAD_COOKIES));
      if(cookieFile.isFile() &&
         e_->getCookieStorage()->load(cookieFile.getPath(),
                                      Time().getTime())) {
        A2_LOG_INFO(fmt("Loaded cookies from '%s'.",
                        cookieFile.getPath().c_str()));
      } else {
        A2_LOG_ERROR(fmt(MSG_LOADING_COOKIE_FAILED,
                         cookieFile.getPath().c_str()));
      }
    }

    SharedHandle<AuthConfigFactory> authConfigFactory(new AuthConfigFactory());
    File netrccf(option_->get(PREF_NETRC_PATH));
    if(!option_->getAsBool(PREF_NO_NETRC) && netrccf.isFile()) {
#ifdef __MINGW32__
      // Windows OS does not have permission, so set it to 0.
      mode_t mode  = 0;
#else // !__MINGW32__
      mode_t mode = netrccf.mode();
#endif // !__MINGW32__
      if(mode&(S_IRWXG|S_IRWXO)) {
        A2_LOG_NOTICE(fmt(MSG_INCORRECT_NETRC_PERMISSION,
                          option_->get(PREF_NETRC_PATH).c_str()));
      } else {
        SharedHandle<Netrc> netrc(new Netrc());
        netrc->parse(option_->get(PREF_NETRC_PATH));
        authConfigFactory->setNetrc(netrc);
      }
    }
    e_->setAuthConfigFactory(authConfigFactory);

#ifdef ENABLE_SSL
    SharedHandle<TLSContext> clTlsContext(TLSContext::make(TLS_CLIENT));
    if(!option_->blank(PREF_CERTIFICATE) &&
       !option_->blank(PREF_PRIVATE_KEY)) {
      clTlsContext->addCredentialFile(option_->get(PREF_CERTIFICATE),
                                      option_->get(PREF_PRIVATE_KEY));
    }

    if(!option_->blank(PREF_CA_CERTIFICATE)) {
      if(!clTlsContext->addTrustedCACertFile
         (option_->get(PREF_CA_CERTIFICATE))) {
        A2_LOG_INFO(MSG_WARN_NO_CA_CERT);
      }
    } else if(option_->getAsBool(PREF_CHECK_CERTIFICATE)) {
      if(!clTlsContext->addSystemTrustedCACerts()) {
        A2_LOG_INFO(MSG_WARN_NO_CA_CERT);
      }
    }
    clTlsContext->setVerifyPeer(option_->getAsBool(PREF_CHECK_CERTIFICATE));
    SocketCore::setClientTLSContext(clTlsContext);
#endif
#ifdef HAVE_ARES_ADDR_NODE
    ares_addr_node* asyncDNSServers =
      parseAsyncDNSServers(option_->get(PREF_ASYNC_DNS_SERVER));
    e_->setAsyncDNSServers(asyncDNSServers);
#endif // HAVE_ARES_ADDR_NODE
    if(!Timer::monotonicClock()) {
      A2_LOG_WARN("Don't change system time while aria2c is running."
                  " Doing this may make aria2c hang for long time.");
    }

    std::string serverStatIf = option_->get(PREF_SERVER_STAT_IF);
    if(!serverStatIf.empty()) {
      e_->getRequestGroupMan()->loadServerStat(serverStatIf);
      e_->getRequestGroupMan()->removeStaleServerStat
        (option_->getAsInt(PREF_SERVER_STAT_TIMEOUT));
    }
    e_->setStatCalc(statCalc_);
    if(uriListParser_) {
      e_->getRequestGroupMan()->setUriListParser(uriListParser_);
    }
    if(useSignalHandler_) {
      setupSignalHandlers();
    }
    e_->getRequestGroupMan()->getNetStat().downloadStart();
  } catch(RecoverableException& e) {
    SingletonHolder<Notifier>::clear();
    if(useSignalHandler_) {
      resetSignalHandlers();
    }
    return -1;
  }
  return 0;
}

error_code::Value MultiUrlRequestInfo::getResult()
{
  error_code::Value returnValue = error_code::FINISHED;
  if(!option_->blank(PREF_SAVE_COOKIES)) {
    e_->getCookieStorage()->saveNsFormat(option_->get(PREF_SAVE_COOKIES));
  }

  const std::string& serverStatOf = option_->get(PREF_SERVER_STAT_OF);
  if(!serverStatOf.empty()) {
    e_->getRequestGroupMan()->saveServerStat(serverStatOf);
  }
  e_->getRequestGroupMan()->showDownloadResults
    (*summaryOut_, option_->get(PREF_DOWNLOAD_RESULT) == A2_V_FULL);
  summaryOut_->flush();

  RequestGroupMan::DownloadStat s =
    e_->getRequestGroupMan()->getDownloadStat();
  if(!s.allCompleted()) {
    printMessageForContinue();
    if(s.getLastErrorResult() == error_code::FINISHED &&
       s.getInProgress() > 0) {
      returnValue = error_code::IN_PROGRESS;
    } else {
      returnValue = s.getLastErrorResult();
    }
  }
  SessionSerializer sessionSerializer(e_->getRequestGroupMan());
  // TODO Add option: --save-session-status=error,inprogress,waiting
  if(!option_->blank(PREF_SAVE_SESSION)) {
    const std::string& filename = option_->get(PREF_SAVE_SESSION);
    if(sessionSerializer.save(filename)) {
      A2_LOG_NOTICE(fmt(_("Serialized session to '%s' successfully."),
                        filename.c_str()));
    } else {
      A2_LOG_NOTICE(fmt(_("Failed to serialize session to '%s'."),
                        filename.c_str()));
    }
  }
  SingletonHolder<Notifier>::clear();
  return returnValue;
}

error_code::Value MultiUrlRequestInfo::execute()
{
  if(prepare() != 0) {
    return error_code::UNKNOWN_ERROR;
  }
  // TODO Enclosed in try..catch block for just in case. Really need
  // this?
  try {
    e_->run();
  } catch(RecoverableException& e) {
    A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, e);
  }
  error_code::Value returnValue = getResult();
  if(useSignalHandler_) {
    resetSignalHandlers();
  }
  return returnValue;
}

void MultiUrlRequestInfo::setupSignalHandlers()
{
#ifdef HAVE_SIGACTION
    sigemptyset(&mask_);
#else // !HAVE_SIGACTION
    mask_ = 0;
#endif // !HAVE_SIGACTION
#ifdef SIGPIPE
    util::setGlobalSignalHandler(SIGPIPE, &mask_, SIG_IGN, 0);
#endif // SIGPIPE
#ifdef SIGCHLD
    // Avoid to create zombie process when forked child processes are
    // died.
    util::setGlobalSignalHandler(SIGCHLD, &mask_, SIG_IGN, 0);
#endif // SIGCHILD

#ifdef HAVE_SIGACTION
    sigaddset(&mask_, SIGINT);
    sigaddset(&mask_, SIGTERM);
# ifdef SIGHUP
    sigaddset(&mask_, SIGHUP);
# endif // SIGHUP
#endif // HAVE_SIGACTION

#ifdef SIGHUP
    util::setGlobalSignalHandler(SIGHUP, &mask_, handler, 0);
#endif // SIGHUP
    util::setGlobalSignalHandler(SIGINT, &mask_, handler, 0);
    util::setGlobalSignalHandler(SIGTERM, &mask_, handler, 0);
}

void MultiUrlRequestInfo::resetSignalHandlers()
{
#ifdef HAVE_SIGACTION
  sigemptyset(&mask_);
#endif // HAVE_SIGACTION
#ifdef SIGHUP
  util::setGlobalSignalHandler(SIGHUP, &mask_, SIG_DFL, 0);
#endif // SIGHUP
  util::setGlobalSignalHandler(SIGINT, &mask_, SIG_DFL, 0);
  util::setGlobalSignalHandler(SIGTERM, &mask_, SIG_DFL, 0);

#ifdef SIGCHLD
    util::setGlobalSignalHandler(SIGCHLD, &mask_, SIG_DFL, 0);
#endif // SIGCHILD
#ifdef SIGPIPE
    util::setGlobalSignalHandler(SIGPIPE, &mask_, SIG_DFL, 0);
#endif // SIGPIPE
}

const SharedHandle<DownloadEngine>&
MultiUrlRequestInfo::getDownloadEngine() const
{
  return e_;
}

} // namespace aria2
