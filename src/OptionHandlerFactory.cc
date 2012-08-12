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
#include "OptionHandlerFactory.h"
#include "prefs.h"
#include "OptionHandlerImpl.h"
#include "array_fun.h"
#include "usage_text.h"
#include "A2STR.h"
#include "util.h"
#include "help_tags.h"
#include "a2functional.h"
#include "File.h"

namespace aria2 {

std::vector<SharedHandle<OptionHandler> >
OptionHandlerFactory::createOptionHandlers()
{
  std::vector<SharedHandle<OptionHandler> > handlers;
  // General Options
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_ALLOW_OVERWRITE,
                                    TEXT_ALLOW_OVERWRITE,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_ADVANCED);
    op->addTag(TAG_FILE);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_ALLOW_PIECE_LENGTH_CHANGE,
                                    TEXT_ALLOW_PIECE_LENGTH_CHANGE,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_ADVANCED);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_ALWAYS_RESUME,
                                    TEXT_ALWAYS_RESUME,
                                    A2_V_TRUE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_ADVANCED);
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
#ifdef ENABLE_ASYNC_DNS
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_ASYNC_DNS,
                                    TEXT_ASYNC_DNS,
#if defined(__ANDROID__) || defined(ANDROID)
                                    A2_V_FALSE,
#else // !__ANDROID__ && !ANDROID
                                    A2_V_TRUE,
#endif // !__ANDROID__ && !ANDROID
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_ADVANCED);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
#if defined HAVE_ARES_SET_SERVERS && HAVE_ARES_ADDR_NODE
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_ASYNC_DNS_SERVER,
                                    TEXT_ASYNC_DNS_SERVER,
                                    NO_DEFAULT_VALUE));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
#endif // HAVE_ARES_SET_SERVERS && HAVE_ARES_ADDR_NODE
#endif // ENABLE_ASYNC_DNS
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_AUTO_FILE_RENAMING,
                                    TEXT_AUTO_FILE_RENAMING,
                                    A2_V_TRUE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_ADVANCED);
    op->addTag(TAG_FILE);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
                                   (PREF_AUTO_SAVE_INTERVAL,
                                    TEXT_AUTO_SAVE_INTERVAL,
                                    "60",
                                    0, 600));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
#ifdef ENABLE_MESSAGE_DIGEST
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_CHECK_INTEGRITY,
                                    TEXT_CHECK_INTEGRITY,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG,
                                    'V'));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_BITTORRENT);
    op->addTag(TAG_METALINK);
    op->addTag(TAG_FILE);
    op->addTag(TAG_CHECKSUM);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
#endif // ENABLE_MESSAGE_DIGEST
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_CONDITIONAL_GET,
                                    TEXT_CONDITIONAL_GET,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_ADVANCED);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_CONF_PATH,
                                    TEXT_CONF_PATH,
                                    util::getHomeDir()+"/.aria2/aria2.conf",
                                    PATH_TO_FILE));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_CONTINUE,
                                    TEXT_CONTINUE,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG,
                                    'c'));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_DAEMON,
                                    TEXT_DAEMON,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG,
                                    'D'));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_DEFERRED_INPUT,
                                    TEXT_DEFERRED_INPUT,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_DIR,
                                    TEXT_DIR,
                                    File::getCurrentDir(),
                                    PATH_TO_DIR,
                                    OptionHandler::REQ_ARG,
                                    'd'));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_FILE);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_DISABLE_IPV6,
                                    TEXT_DISABLE_IPV6,
#if defined(__MINGW32__) && !defined(__MINGW64__)
                                    // Disable IPv6 by default for
                                    // MinGW build.  This is because
                                    // numerous IPv6 routines are
                                    // available from Vista.  Checking
                                    // getaddrinfo failed in
                                    // configure.
                                    A2_V_TRUE,
#else // !defined(__MINGW32__) || defined(__MINGW64__)
                                    A2_V_FALSE,
#endif // !defined(__MINGW32__) || defined(__MINGW64__)
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<NumberOptionHandler> op(new NumberOptionHandler
                                         (PREF_DNS_TIMEOUT,
                                          NO_DESCRIPTION,
                                          "30",
                                          1, 60));
    op->hide();
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
                                   (PREF_DOWNLOAD_RESULT,
                                    TEXT_DOWNLOAD_RESULT,
                                    A2_V_DEFAULT,
                                    A2_V_DEFAULT,
                                    A2_V_FULL));
    op->addTag(TAG_ADVANCED);
    op->setChangeGlobalOption(true);
    handlers.push_back(op);
  }
#ifdef ENABLE_ASYNC_DNS
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_ENABLE_ASYNC_DNS6,
                                    TEXT_ENABLE_ASYNC_DNS6,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_ADVANCED);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
#endif // ENABLE_ASYNC_DNS
  {
    // TODO Deprecated
    SharedHandle<OptionHandler> op
      (new DeprecatedOptionHandler
       (SharedHandle<OptionHandler>(new BooleanOptionHandler
                                    (PREF_ENABLE_DIRECT_IO,
                                     TEXT_ENABLE_DIRECT_IO,
                                     NO_DEFAULT_VALUE,
                                     OptionHandler::OPT_ARG))));
    op->addTag(TAG_DEPRECATED);
    op->addTag(TAG_ADVANCED);
    op->addTag(TAG_FILE);
    handlers.push_back(op);
  }
#ifdef HAVE_MMAP
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_ENABLE_MMAP,
                                    TEXT_ENABLE_MMAP,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_ADVANCED);
    op->addTag(TAG_EXPERIMENTAL);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
#endif // HAVE_MMAP
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_ENABLE_RPC,
                                    TEXT_ENABLE_RPC,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_RPC);
    handlers.push_back(op);
  }
  {
    std::string params[] = {
#ifdef HAVE_EPOLL
      V_EPOLL,
#endif // HAVE_EPOLL
#ifdef HAVE_KQUEUE
      V_KQUEUE,
#endif // HAVE_KQUEUE
#ifdef HAVE_PORT_ASSOCIATE
      V_PORT,
#endif // HAVE_PORT_ASSOCIATE
#ifdef HAVE_POLL
      V_POLL,
#endif // HAVE_POLL
      V_SELECT
    };
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
                                   (PREF_EVENT_POLL,
                                    TEXT_EVENT_POLL,
#ifdef HAVE_EPOLL
                                    V_EPOLL,
#elif HAVE_KQUEUE
                                    V_KQUEUE,
#elif HAVE_PORT_ASSOCIATE
                                    V_PORT,
#else
                                    V_SELECT,
#endif // !HAVE_EPOLL
                                    std::vector<std::string>
                                    (vbegin(params), vend(params))));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    const std::string params[] = { V_NONE, V_PREALLOC, V_TRUNC,
#ifdef HAVE_SOME_FALLOCATE
                                   V_FALLOC
#endif // HAVE_SOME_FALLOCATE
    };
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
                                   (PREF_FILE_ALLOCATION,
                                    TEXT_FILE_ALLOCATION,
                                    V_PREALLOC,
                                    std::vector<std::string>
                                    (vbegin(params), vend(params)),
                                    'a'));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_FILE);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_FORCE_SEQUENTIAL,
                                    TEXT_FORCE_SEQUENTIAL,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG,
                                    'Z'));
    op->addTag(TAG_BASIC);
    handlers.push_back(op);
  }
#ifdef ENABLE_MESSAGE_DIGEST
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_HASH_CHECK_ONLY,
                                    TEXT_HASH_CHECK_ONLY,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_ADVANCED);
    op->addTag(TAG_BITTORRENT);
    op->addTag(TAG_METALINK);
    op->addTag(TAG_FILE);
    op->addTag(TAG_CHECKSUM);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
#endif // ENABLE_MESSAGE_DIGEST
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_HUMAN_READABLE,
                                    TEXT_HUMAN_READABLE,
                                    A2_V_TRUE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_INPUT_FILE,
                                    TEXT_INPUT_FILE,
                                    NO_DEFAULT_VALUE,
                                    PATH_TO_FILE_STDIN,
                                    OptionHandler::REQ_ARG,
                                    'i'));
    op->addTag(TAG_BASIC);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_INTERFACE,
                                    TEXT_INTERFACE,
                                    NO_DEFAULT_VALUE,
                                    "interface, IP address, hostname",
                                    OptionHandler::REQ_ARG));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_LOG,
                                    TEXT_LOG,
                                    NO_DEFAULT_VALUE,
                                    PATH_TO_FILE_STDOUT,
                                    OptionHandler::REQ_ARG,
                                    'l'));
    op->addTag(TAG_BASIC);
    op->setChangeGlobalOption(true);
    handlers.push_back(op);
  }
  {
    const std::string params[] = { V_DEBUG, V_INFO, V_NOTICE, V_WARN, V_ERROR };
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
                                   (PREF_LOG_LEVEL,
                                    TEXT_LOG_LEVEL,
                                    V_DEBUG,
                                    std::vector<std::string>
                                    (vbegin(params), vend(params))));
    op->addTag(TAG_ADVANCED);
    op->setChangeGlobalOption(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
                                   (PREF_MAX_CONCURRENT_DOWNLOADS,
                                    TEXT_MAX_CONCURRENT_DOWNLOADS,
                                    "5",
                                    1, -1,
                                    'j'));
    op->addTag(TAG_BASIC);
    op->setChangeGlobalOption(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
                                   (PREF_MAX_CONNECTION_PER_SERVER,
                                    TEXT_MAX_CONNECTION_PER_SERVER,
                                    "1",
                                    1, 16,
                                    'x'));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new UnitNumberOptionHandler
                                   (PREF_MAX_DOWNLOAD_LIMIT,
                                    TEXT_MAX_DOWNLOAD_LIMIT,
                                    "0",
                                    0));
    op->addTag(TAG_BITTORRENT);
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
                                   (PREF_MAX_DOWNLOAD_RESULT,
                                    TEXT_MAX_DOWNLOAD_RESULT,
                                    "1000",
                                    0));
    op->addTag(TAG_ADVANCED);
    op->setChangeGlobalOption(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new UnitNumberOptionHandler
                                   (PREF_MAX_OVERALL_DOWNLOAD_LIMIT,
                                    TEXT_MAX_OVERALL_DOWNLOAD_LIMIT,
                                    "0",
                                    0));
    op->addTag(TAG_BITTORRENT);
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setChangeGlobalOption(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
                                   (PREF_MAX_RESUME_FAILURE_TRIES,
                                    TEXT_MAX_RESUME_FAILURE_TRIES,
                                    "0",
                                    0));
    op->addTag(TAG_ADVANCED);
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new UnitNumberOptionHandler
                                   (PREF_MIN_SPLIT_SIZE,
                                    TEXT_MIN_SPLIT_SIZE,
                                    "20M",
                                    1024*1024, 1024*1024*1024,
                                    'k'));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_NO_CONF,
                                    TEXT_NO_CONF,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new UnitNumberOptionHandler
                                   (PREF_NO_FILE_ALLOCATION_LIMIT,
                                    TEXT_NO_FILE_ALLOCATION_LIMIT,
                                    "5M",
                                    0));
    op->addTag(TAG_ADVANCED);
    op->addTag(TAG_FILE);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_ON_DOWNLOAD_COMPLETE,
                                    TEXT_ON_DOWNLOAD_COMPLETE,
                                    NO_DEFAULT_VALUE,
                                    PATH_TO_COMMAND));
    op->addTag(TAG_ADVANCED);
    op->addTag(TAG_HOOK);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_ON_DOWNLOAD_ERROR,
                                    TEXT_ON_DOWNLOAD_ERROR,
                                    NO_DEFAULT_VALUE,
                                    PATH_TO_COMMAND));
    op->addTag(TAG_ADVANCED);
    op->addTag(TAG_HOOK);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_ON_DOWNLOAD_PAUSE,
                                    TEXT_ON_DOWNLOAD_PAUSE,
                                    NO_DEFAULT_VALUE,
                                    PATH_TO_COMMAND));
    op->addTag(TAG_ADVANCED);
    op->addTag(TAG_HOOK);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_ON_DOWNLOAD_START,
                                    TEXT_ON_DOWNLOAD_START,
                                    NO_DEFAULT_VALUE,
                                    PATH_TO_COMMAND));
    op->addTag(TAG_ADVANCED);
    op->addTag(TAG_HOOK);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_ON_DOWNLOAD_STOP,
                                    TEXT_ON_DOWNLOAD_STOP,
                                    NO_DEFAULT_VALUE,
                                    PATH_TO_COMMAND));
    op->addTag(TAG_ADVANCED);
    op->addTag(TAG_HOOK);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_PARAMETERIZED_URI,
                                    TEXT_PARAMETERIZED_URI,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG,
                                    'P'));
    op->addTag(TAG_ADVANCED);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_PAUSE,
                                    TEXT_PAUSE,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_ADVANCED);
    op->addTag(TAG_RPC);
    op->setInitialOption(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_QUIET,
                                    TEXT_QUIET,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG,
                                    'q'));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
#ifdef ENABLE_MESSAGE_DIGEST
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_REALTIME_CHUNK_CHECKSUM,
                                    TEXT_REALTIME_CHUNK_CHECKSUM,
                                    A2_V_TRUE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_METALINK);
    op->addTag(TAG_CHECKSUM);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
#endif // ENABLE_MESSAGE_DIGEST
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_REMOVE_CONTROL_FILE,
                                    TEXT_REMOVE_CONTROL_FILE,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_ADVANCED);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_SAVE_SESSION,
                                    TEXT_SAVE_SESSION,
                                    NO_DEFAULT_VALUE,
                                    PATH_TO_FILE));
    op->addTag(TAG_ADVANCED);
    op->setChangeGlobalOption(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_SELECT_LEAST_USED_HOST,
                                    NO_DESCRIPTION,
                                    A2_V_TRUE));
    op->hide();
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_SHOW_CONSOLE_READOUT,
                                    TEXT_SHOW_CONSOLE_READOUT,
                                    A2_V_TRUE));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
                                   (PREF_STOP,
                                    TEXT_STOP,
                                    "0",
                                    0, INT32_MAX));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
                                   (PREF_STOP_WITH_PROCESS,
                                    TEXT_STOP_WITH_PROCESS,
                                    NO_DEFAULT_VALUE,
                                    0));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
                                   (PREF_SUMMARY_INTERVAL,
                                    TEXT_SUMMARY_INTERVAL,
                                    "60",
                                    0, INT32_MAX));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_TRUNCATE_CONSOLE_READOUT,
                                    TEXT_TRUNCATE_CONSOLE_READOUT,
                                    A2_V_TRUE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_ADVANCED);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_RPC_ALLOW_ORIGIN_ALL,
                                    TEXT_RPC_ALLOW_ORIGIN_ALL,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_RPC);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_RPC_LISTEN_ALL,
                                    TEXT_RPC_LISTEN_ALL,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_RPC);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
                                   (PREF_RPC_LISTEN_PORT,
                                    TEXT_RPC_LISTEN_PORT,
                                    "6800",
                                    1024, UINT16_MAX));
    op->addTag(TAG_RPC);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new UnitNumberOptionHandler
                                   (PREF_RPC_MAX_REQUEST_SIZE,
                                    TEXT_RPC_MAX_REQUEST_SIZE,
                                    "2M",
                                    0));
    op->addTag(TAG_RPC);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_RPC_USER,
                                    TEXT_RPC_USER));
    op->addTag(TAG_RPC);
    op->setEraseAfterParse(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_RPC_PASSWD,
                                    TEXT_RPC_PASSWD));
    op->addTag(TAG_RPC);
    op->setEraseAfterParse(true);
    handlers.push_back(op);
  }
  // HTTP/FTP options
#ifdef ENABLE_MESSAGE_DIGEST
  {
    SharedHandle<OptionHandler> op(new ChecksumOptionHandler
                                   (PREF_CHECKSUM,
                                    TEXT_CHECKSUM));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->addTag(TAG_CHECKSUM);
    op->setInitialOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
#endif // ENABLE_MESSAGE_DIGEST
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
                                   (PREF_CONNECT_TIMEOUT,
                                    TEXT_CONNECT_TIMEOUT,
                                    "60",
                                    1, 600));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_DRY_RUN,
                                    TEXT_DRY_RUN,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new UnitNumberOptionHandler
                                   (PREF_LOWEST_SPEED_LIMIT,
                                    TEXT_LOWEST_SPEED_LIMIT,
                                    "0",
                                    0));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
                                   (PREF_MAX_FILE_NOT_FOUND,
                                    TEXT_MAX_FILE_NOT_FOUND,
                                    "0",
                                    0));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
                                   (PREF_MAX_TRIES,
                                    TEXT_MAX_TRIES,
                                    "5",
                                    0, -1,
                                    'm'));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_NO_NETRC,
                                    TEXT_NO_NETRC,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG,
                                    'n'));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_OUT,
                                    TEXT_OUT,
                                    NO_DEFAULT_VALUE,
                                    PATH_TO_FILE,
                                    OptionHandler::REQ_ARG,
                                    'o'));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->addTag(TAG_FILE);
    op->setInitialOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<UnitNumberOptionHandler> op(new UnitNumberOptionHandler
                                             (PREF_PIECE_LENGTH,
                                              TEXT_PIECE_LENGTH,
                                              "1M",
                                              1024*1024,
                                              1024*1024*1024));
    op->addTag(TAG_ADVANCED);
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_REMOTE_TIME,
                                    TEXT_REMOTE_TIME,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG,
                                    'R'));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
                                   (PREF_RETRY_WAIT,
                                    TEXT_RETRY_WAIT,
                                    "0",
                                    0, 600));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_REUSE_URI,
                                    TEXT_REUSE_URI,
                                    A2_V_TRUE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_SERVER_STAT_IF,
                                    TEXT_SERVER_STAT_IF,
                                    NO_DEFAULT_VALUE,
                                    PATH_TO_FILE));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_SERVER_STAT_OF,
                                    TEXT_SERVER_STAT_OF,
                                    NO_DEFAULT_VALUE,
                                    PATH_TO_FILE));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setChangeGlobalOption(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
                                   (PREF_SERVER_STAT_TIMEOUT,
                                    TEXT_SERVER_STAT_TIMEOUT,
                                    "86400",
                                    0, INT32_MAX));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    handlers.push_back(op);
  }
  SharedHandle<OptionHandler> splitHandler;
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
                                   (PREF_SPLIT,
                                    TEXT_SPLIT,
                                    "5",
                                    1, -1,
                                    's'));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    splitHandler = op;
    handlers.push_back(op);
  }
  {
    SharedHandle<NumberOptionHandler> op(new NumberOptionHandler
                                         (PREF_STARTUP_IDLE_TIME,
                                          NO_DESCRIPTION,
                                          "10",
                                          1, 60));
    op->hide();
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
                                   (PREF_STREAM_PIECE_SELECTOR,
                                    TEXT_STREAM_PIECE_SELECTOR,
                                    A2_V_DEFAULT,
                                    A2_V_DEFAULT,
                                    V_INORDER,
                                    A2_V_GEOM));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
                                   (PREF_TIMEOUT,
                                    TEXT_TIMEOUT,
                                    "60",
                                    1, 600,
                                    't'));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    const std::string params[] = { V_INORDER, V_FEEDBACK, V_ADAPTIVE };
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
                                   (PREF_URI_SELECTOR,
                                    TEXT_URI_SELECTOR,
                                    V_FEEDBACK,
                                    std::vector<std::string>
                                    (vbegin(params), vend(params))));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  // HTTP Specific Options
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_CA_CERTIFICATE,
                                    TEXT_CA_CERTIFICATE,
                                    CA_BUNDLE,
                                    PATH_TO_FILE));
    op->addTag(TAG_HTTP);
    op->addTag(TAG_HTTPS);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_CERTIFICATE,
                                    TEXT_CERTIFICATE,
                                    NO_DEFAULT_VALUE,
                                    PATH_TO_FILE));
    op->addTag(TAG_HTTP);
    op->addTag(TAG_HTTPS);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_CHECK_CERTIFICATE,
                                    TEXT_CHECK_CERTIFICATE,
                                    A2_V_TRUE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_HTTP);
    op->addTag(TAG_HTTPS);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_ENABLE_HTTP_KEEP_ALIVE,
                                    TEXT_ENABLE_HTTP_KEEP_ALIVE,
                                    A2_V_TRUE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_ENABLE_HTTP_PIPELINING,
                                    TEXT_ENABLE_HTTP_PIPELINING,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new CumulativeOptionHandler
                                   (PREF_HEADER,
                                    TEXT_HEADER,
                                    NO_DEFAULT_VALUE,
                                    "\n"));
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setCumulative(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_HTTP_ACCEPT_GZIP,
                                    TEXT_HTTP_ACCEPT_GZIP,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_HTTP_AUTH_CHALLENGE,
                                    TEXT_HTTP_AUTH_CHALLENGE,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_HTTP_NO_CACHE,
                                    TEXT_HTTP_NO_CACHE,
                                    A2_V_TRUE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_HTTP_PASSWD,
                                    TEXT_HTTP_PASSWD));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_HTTP);
    op->setEraseAfterParse(true);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_HTTP_USER,
                                    TEXT_HTTP_USER));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_HTTP);
    op->setEraseAfterParse(true);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_LOAD_COOKIES,
                                    TEXT_LOAD_COOKIES,
                                    NO_DEFAULT_VALUE,
                                    PATH_TO_FILE));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_HTTP);
    op->addTag(TAG_COOKIE);
    handlers.push_back(op);
  }
  {
    SharedHandle<NumberOptionHandler> op(new NumberOptionHandler
                                         (PREF_MAX_HTTP_PIPELINING,
                                          NO_DESCRIPTION,
                                          "2",
                                          1, 8));
    op->hide();
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_METALINK_LOCATION,
                                    TEXT_METALINK_LOCATION));
    op->addTag(TAG_METALINK);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_PRIVATE_KEY,
                                    TEXT_PRIVATE_KEY,
                                    NO_DEFAULT_VALUE,
                                    PATH_TO_FILE));
    op->addTag(TAG_HTTP);
    op->addTag(TAG_HTTPS);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_REFERER,
                                    TEXT_REFERER));
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_SAVE_COOKIES,
                                    TEXT_SAVE_COOKIES,
                                    NO_DEFAULT_VALUE,
                                    PATH_TO_FILE));
    op->addTag(TAG_HTTP);
    op->addTag(TAG_COOKIE);
    op->setChangeGlobalOption(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_USE_HEAD,
                                    TEXT_USE_HEAD,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_USER_AGENT,
                                    TEXT_USER_AGENT,
                                    "aria2/"PACKAGE_VERSION,
                                    A2STR::NIL,
                                    OptionHandler::REQ_ARG,
                                    'U'));
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  // FTP Specific Options
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_FTP_PASSWD,
                                    TEXT_FTP_PASSWD));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_FTP);
    op->setEraseAfterParse(true);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_FTP_PASV,
                                    TEXT_FTP_PASV,
                                    A2_V_TRUE,
                                    OptionHandler::OPT_ARG,
                                    'p'));
    op->addTag(TAG_FTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_FTP_REUSE_CONNECTION,
                                    TEXT_FTP_REUSE_CONNECTION,
                                    A2_V_TRUE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_FTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
                                   (PREF_FTP_TYPE,
                                    TEXT_FTP_TYPE,
                                    V_BINARY,
                                    V_BINARY, V_ASCII));
    op->addTag(TAG_FTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_FTP_USER,
                                    TEXT_FTP_USER));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_FTP);
    op->setEraseAfterParse(true);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<DefaultOptionHandler> op(new DefaultOptionHandler
                                          (PREF_NETRC_PATH,
                                           NO_DESCRIPTION,
                                           util::getHomeDir()+"/.netrc",
                                           PATH_TO_FILE));
    op->hide();
    handlers.push_back(op);
  }
  // Proxy options
  {
    SharedHandle<OptionHandler> op(new HttpProxyOptionHandler
                                   (PREF_HTTP_PROXY,
                                    TEXT_HTTP_PROXY,
                                    NO_DEFAULT_VALUE));
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_HTTP_PROXY_PASSWD,
                                    TEXT_HTTP_PROXY_PASSWD,
                                    NO_DEFAULT_VALUE));
    op->addTag(TAG_HTTP);
    op->setEraseAfterParse(true);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_HTTP_PROXY_USER,
                                    TEXT_HTTP_PROXY_USER,
                                    NO_DEFAULT_VALUE));
    op->addTag(TAG_HTTP);
    op->setEraseAfterParse(true);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new HttpProxyOptionHandler
                                   (PREF_HTTPS_PROXY,
                                    TEXT_HTTPS_PROXY,
                                    NO_DEFAULT_VALUE));
    op->addTag(TAG_HTTP);
    op->addTag(TAG_HTTPS);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_HTTPS_PROXY_PASSWD,
                                    TEXT_HTTPS_PROXY_PASSWD,
                                    NO_DEFAULT_VALUE));
    op->addTag(TAG_HTTP);
    op->addTag(TAG_HTTPS);
    op->setEraseAfterParse(true);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_HTTPS_PROXY_USER,
                                    TEXT_HTTPS_PROXY_USER,
                                    NO_DEFAULT_VALUE));
    op->addTag(TAG_HTTP);
    op->addTag(TAG_HTTPS);
    op->setEraseAfterParse(true);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new HttpProxyOptionHandler
                                   (PREF_FTP_PROXY,
                                    TEXT_FTP_PROXY,
                                    NO_DEFAULT_VALUE));
    op->addTag(TAG_FTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_FTP_PROXY_PASSWD,
                                    TEXT_FTP_PROXY_PASSWD,
                                    NO_DEFAULT_VALUE));
    op->addTag(TAG_FTP);
    op->setEraseAfterParse(true);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_FTP_PROXY_USER,
                                    TEXT_FTP_PROXY_USER,
                                    NO_DEFAULT_VALUE));
    op->addTag(TAG_FTP);
    op->setEraseAfterParse(true);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new HttpProxyOptionHandler
                                   (PREF_ALL_PROXY,
                                    TEXT_ALL_PROXY,
                                    NO_DEFAULT_VALUE));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->addTag(TAG_HTTPS);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_ALL_PROXY_PASSWD,
                                    TEXT_ALL_PROXY_PASSWD,
                                    NO_DEFAULT_VALUE));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->addTag(TAG_HTTPS);
    op->setEraseAfterParse(true);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_ALL_PROXY_USER,
                                    TEXT_ALL_PROXY_USER,
                                    NO_DEFAULT_VALUE));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->addTag(TAG_HTTPS);
    op->setEraseAfterParse(true);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_NO_PROXY,
                                    TEXT_NO_PROXY,
                                    NO_DEFAULT_VALUE,
                                    "HOSTNAME,DOMAIN,NETWORK/CIDR"));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->addTag(TAG_HTTPS);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
                                   (PREF_PROXY_METHOD,
                                    TEXT_PROXY_METHOD,
                                    V_GET,
                                    V_GET, V_TUNNEL));
    op->addTag(TAG_FTP);
    op->addTag(TAG_HTTP);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  // BitTorrent/Metalink Options
#if defined ENABLE_BITTORRENT || defined ENABLE_METALINK
  {
    SharedHandle<OptionHandler> op(new IntegerRangeOptionHandler
                                   (PREF_SELECT_FILE,
                                    TEXT_SELECT_FILE,
                                    NO_DEFAULT_VALUE,
                                    1, 65535));
    op->addTag(TAG_BITTORRENT);
    op->addTag(TAG_METALINK);
    op->setInitialOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_SHOW_FILES,
                                    TEXT_SHOW_FILES,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG,
                                    'S'));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_BITTORRENT);
    op->addTag(TAG_METALINK);
    handlers.push_back(op);
  }
#endif // ENABLE_BITTORRENT || ENABLE_METALINK
  // BitTorrent Specific Options
#ifdef ENABLE_BITTORRENT
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_BT_ENABLE_LPD,
                                    TEXT_BT_ENABLE_LPD,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<DefaultOptionHandler> op(new DefaultOptionHandler
                                          (PREF_BT_EXCLUDE_TRACKER,
                                           TEXT_BT_EXCLUDE_TRACKER,
                                           NO_DESCRIPTION,
                                           "URI,... "
                                           "or *"));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_BT_EXTERNAL_IP,
                                    TEXT_BT_EXTERNAL_IP,
                                    NO_DEFAULT_VALUE,
                                    "a numeric IP address"));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<NumberOptionHandler> op(new NumberOptionHandler
                                         (PREF_BT_KEEP_ALIVE_INTERVAL,
                                          NO_DESCRIPTION,
                                          "120",
                                          1, 120));
    op->hide();
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_BT_HASH_CHECK_SEED,
                                    TEXT_BT_HASH_CHECK_SEED,
                                    A2_V_TRUE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_BITTORRENT);
    op->addTag(TAG_CHECKSUM);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_BT_LPD_INTERFACE,
                                    TEXT_BT_LPD_INTERFACE,
                                    NO_DEFAULT_VALUE,
                                    "interface, IP address",
                                    OptionHandler::REQ_ARG));
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
                                   (PREF_BT_MAX_OPEN_FILES,
                                    TEXT_BT_MAX_OPEN_FILES,
                                    "100",
                                    1));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
                                   (PREF_BT_MAX_PEERS,
                                    TEXT_BT_MAX_PEERS,
                                    "55",
                                    0));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);

  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_BT_METADATA_ONLY,
                                    TEXT_BT_METADATA_ONLY,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
                                   (PREF_BT_MIN_CRYPTO_LEVEL,
                                    TEXT_BT_MIN_CRYPTO_LEVEL,
                                    V_PLAIN,
                                    V_PLAIN, V_ARC4));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new PrioritizePieceOptionHandler
                                   (PREF_BT_PRIORITIZE_PIECE,
                                    TEXT_BT_PRIORITIZE_PIECE));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_BT_REMOVE_UNSELECTED_FILE,
                                    TEXT_BT_REMOVE_UNSELECTED_FILE,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new UnitNumberOptionHandler
                                   (PREF_BT_REQUEST_PEER_SPEED_LIMIT,
                                    TEXT_BT_REQUEST_PEER_SPEED_LIMIT,
                                    "50K",
                                    0));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_BT_REQUIRE_CRYPTO,
                                    TEXT_BT_REQUIRE_CRYPTO,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<NumberOptionHandler> op(new NumberOptionHandler
                                         (PREF_BT_REQUEST_TIMEOUT,
                                          NO_DESCRIPTION,
                                          "60",
                                          1, 600));
    op->hide();
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_BT_SEED_UNVERIFIED,
                                    TEXT_BT_SEED_UNVERIFIED,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_BT_SAVE_METADATA,
                                    TEXT_BT_SAVE_METADATA,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
                                   (PREF_BT_STOP_TIMEOUT,
                                    TEXT_BT_STOP_TIMEOUT,
                                    "0",
                                    0));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<NumberOptionHandler> op(new NumberOptionHandler
                                         (PREF_BT_TIMEOUT,
                                          NO_DESCRIPTION,
                                          "180",
                                          1, 600));
    op->hide();
    handlers.push_back(op);
  }
  {
    SharedHandle<DefaultOptionHandler> op(new DefaultOptionHandler
                                          (PREF_BT_TRACKER,
                                           TEXT_BT_TRACKER,
                                           NO_DESCRIPTION,
                                           "URI,..."));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<NumberOptionHandler> op(new NumberOptionHandler
                                         (PREF_BT_TRACKER_CONNECT_TIMEOUT,
                                          TEXT_BT_TRACKER_CONNECT_TIMEOUT,
                                          "60",
                                          1, 600));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<NumberOptionHandler> op(new NumberOptionHandler
                                         (PREF_BT_TRACKER_INTERVAL,
                                          TEXT_BT_TRACKER_INTERVAL,
                                          "0",
                                          0));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<NumberOptionHandler> op(new NumberOptionHandler
                                         (PREF_BT_TRACKER_TIMEOUT,
                                          TEXT_BT_TRACKER_TIMEOUT,
                                          "60",
                                          1, 600));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new HostPortOptionHandler
                                   (PREF_DHT_ENTRY_POINT,
                                    TEXT_DHT_ENTRY_POINT,
                                    NO_DEFAULT_VALUE,
                                    PREF_DHT_ENTRY_POINT_HOST,
                                    PREF_DHT_ENTRY_POINT_PORT));
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new HostPortOptionHandler
                                   (PREF_DHT_ENTRY_POINT6,
                                    TEXT_DHT_ENTRY_POINT6,
                                    NO_DEFAULT_VALUE,
                                    PREF_DHT_ENTRY_POINT_HOST6,
                                    PREF_DHT_ENTRY_POINT_PORT6));
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_DHT_FILE_PATH,
                                    TEXT_DHT_FILE_PATH,
                                    util::getHomeDir()+"/.aria2/dht.dat",
                                    PATH_TO_FILE));
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_DHT_FILE_PATH6,
                                    TEXT_DHT_FILE_PATH6,
                                    util::getHomeDir()+"/.aria2/dht6.dat",
                                    PATH_TO_FILE));
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_DHT_LISTEN_ADDR,
                                    NO_DESCRIPTION,
                                    NO_DEFAULT_VALUE));
    op->hide();
    op->addTag(TAG_BASIC);
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_DHT_LISTEN_ADDR6,
                                    TEXT_DHT_LISTEN_ADDR6,
                                    NO_DEFAULT_VALUE));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new IntegerRangeOptionHandler
                                   (PREF_DHT_LISTEN_PORT,
                                    TEXT_DHT_LISTEN_PORT,
                                    "6881-6999",
                                    1024, UINT16_MAX));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
                                   (PREF_DHT_MESSAGE_TIMEOUT,
                                    TEXT_DHT_MESSAGE_TIMEOUT,
                                    "10",
                                    1, 60));
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_ENABLE_DHT,
                                    TEXT_ENABLE_DHT,
                                    A2_V_TRUE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_ENABLE_DHT6,
                                    TEXT_ENABLE_DHT6,
                                    A2_V_FALSE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_ENABLE_PEER_EXCHANGE,
                                    TEXT_ENABLE_PEER_EXCHANGE,
                                    A2_V_TRUE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
                                   (PREF_FOLLOW_TORRENT,
                                    TEXT_FOLLOW_TORRENT,
                                    A2_V_TRUE,
                                    A2_V_TRUE, V_MEM, A2_V_FALSE));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new IndexOutOptionHandler
                                   (PREF_INDEX_OUT,
                                    TEXT_INDEX_OUT,
                                    'O'));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setCumulative(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new IntegerRangeOptionHandler
                                   (PREF_LISTEN_PORT,
                                    TEXT_LISTEN_PORT,
                                    "6881-6999",
                                    1024, UINT16_MAX));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new UnitNumberOptionHandler
                                   (PREF_MAX_OVERALL_UPLOAD_LIMIT,
                                    TEXT_MAX_OVERALL_UPLOAD_LIMIT,
                                    "0",
                                    0));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_BITTORRENT);
    op->setChangeGlobalOption(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new UnitNumberOptionHandler
                                   (PREF_MAX_UPLOAD_LIMIT,
                                    TEXT_MAX_UPLOAD_LIMIT,
                                    "0",
                                    0, -1,
                                    'u'));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_ON_BT_DOWNLOAD_COMPLETE,
                                    TEXT_ON_BT_DOWNLOAD_COMPLETE,
                                    NO_DEFAULT_VALUE,
                                    PATH_TO_COMMAND));
    op->addTag(TAG_ADVANCED);
    op->addTag(TAG_HOOK);
    handlers.push_back(op);
  }
  {
    SharedHandle<NumberOptionHandler> op(new NumberOptionHandler
                                         (PREF_PEER_CONNECTION_TIMEOUT,
                                          NO_DESCRIPTION,
                                          "20",
                                          1, 600));
    op->hide();
    handlers.push_back(op);
  }
  {
    int major, minor, micro;
    sscanf(PACKAGE_VERSION, "%d.%d.%d", &major, &minor, &micro);
    char prefix[21];
    snprintf(prefix, sizeof(prefix), "A2-%d-%d-%d-", major, minor, micro);
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_PEER_ID_PREFIX,
                                    TEXT_PEER_ID_PREFIX,
                                    prefix));
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new NumberOptionHandler
                                   (PREF_SEED_TIME,
                                    TEXT_SEED_TIME,
                                    NO_DEFAULT_VALUE,
                                    0));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new FloatNumberOptionHandler
                                   (PREF_SEED_RATIO,
                                    TEXT_SEED_RATIO,
                                    "1.0",
                                    0.0));
    op->addTag(TAG_BITTORRENT);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new LocalFilePathOptionHandler
                                   (PREF_TORRENT_FILE,
                                    TEXT_TORRENT_FILE,
                                    NO_DEFAULT_VALUE,
                                    false,
                                    'T'));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_BITTORRENT);
    handlers.push_back(op);
  }
#endif // ENABLE_BITTORRENT
  // Metalink Specific Options
#ifdef ENABLE_METALINK
  {
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
                                   (PREF_FOLLOW_METALINK,
                                    TEXT_FOLLOW_METALINK,
                                    A2_V_TRUE,
                                    A2_V_TRUE, V_MEM, A2_V_FALSE));
    op->addTag(TAG_METALINK);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_METALINK_BASE_URI,
                                    TEXT_METALINK_BASE_URI,
                                    NO_DEFAULT_VALUE));
    op->addTag(TAG_METALINK);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new BooleanOptionHandler
                                   (PREF_METALINK_ENABLE_UNIQUE_PROTOCOL,
                                    TEXT_METALINK_ENABLE_UNIQUE_PROTOCOL,
                                    A2_V_TRUE,
                                    OptionHandler::OPT_ARG));
    op->addTag(TAG_METALINK);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new LocalFilePathOptionHandler
                                   (PREF_METALINK_FILE,
                                    TEXT_METALINK_FILE,
                                    NO_DEFAULT_VALUE,
                                    true,
                                    'M'));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_METALINK);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_METALINK_LANGUAGE,
                                    TEXT_METALINK_LANGUAGE));
    op->addTag(TAG_METALINK);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_METALINK_OS,
                                    TEXT_METALINK_OS));
    op->addTag(TAG_METALINK);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    const std::string params[] = { V_HTTP, V_HTTPS, V_FTP, V_NONE };
    SharedHandle<OptionHandler> op(new ParameterOptionHandler
                                   (PREF_METALINK_PREFERRED_PROTOCOL,
                                    TEXT_METALINK_PREFERRED_PROTOCOL,
                                    V_NONE,
                                    std::vector<std::string>
                                    (vbegin(params), vend(params))));
    op->addTag(TAG_METALINK);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op
      (new DeprecatedOptionHandler
       (SharedHandle<OptionHandler>(new NumberOptionHandler
                                    (PREF_METALINK_SERVERS,
                                     TEXT_METALINK_SERVERS,
                                     NO_DEFAULT_VALUE,
                                     1, -1,
                                     'C')),
        splitHandler));
    op->addTag(TAG_DEPRECATED);
    op->addTag(TAG_METALINK);
    op->setInitialOption(true);
    handlers.push_back(op);
  }
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_METALINK_VERSION,
                                    TEXT_METALINK_VERSION));
    op->addTag(TAG_METALINK);
    op->setInitialOption(true);
    op->setChangeGlobalOption(true);
    op->setChangeOptionForReserved(true);
    handlers.push_back(op);
  }
#endif // ENABLE_METALINK
  // Version Option
  {
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_VERSION,
                                    TEXT_VERSION,
                                    NO_DEFAULT_VALUE,
                                    A2STR::NIL,
                                    OptionHandler::NO_ARG,
                                    'v'));
    op->addTag(TAG_BASIC);
    handlers.push_back(op);
  }
  // Help Option
  {
    static std::string tags[] = {
      TAG_BASIC,
      TAG_ADVANCED,
      TAG_HTTP,
      TAG_HTTPS,
      TAG_FTP,
      TAG_METALINK,
      TAG_BITTORRENT,
      TAG_COOKIE,
      TAG_HOOK,
      TAG_FILE,
      TAG_RPC,
      TAG_CHECKSUM,
      TAG_EXPERIMENTAL,
      TAG_DEPRECATED,
      TAG_HELP,
      TAG_ALL
    };
    static std::string tagsStr = strjoin(vbegin(tags), vend(tags), ", ");
    SharedHandle<OptionHandler> op(new DefaultOptionHandler
                                   (PREF_HELP,
                                    TEXT_HELP,
                                    TAG_BASIC,
                                    tagsStr,
                                    OptionHandler::OPT_ARG,
                                    'h'));
    op->addTag(TAG_BASIC);
    op->addTag(TAG_HELP);
    handlers.push_back(op);
  }

  return handlers;
}

} // namespace aria2
