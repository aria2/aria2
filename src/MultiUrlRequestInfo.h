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
#ifndef D_MULTI_URL_REQUEST_INFO_H
#define D_MULTI_URL_REQUEST_INFO_H

#include "common.h"

#include <signal.h>

#include <vector>

#include "SharedHandle.h"
#include "DownloadResult.h"
#include "util.h"

namespace aria2 {

class RequestGroup;
class Option;
class StatCalc;
class OutputFile;
class UriListParser;
class DownloadEngine;

class MultiUrlRequestInfo {
private:
  std::vector<SharedHandle<RequestGroup> > requestGroups_;

  SharedHandle<Option> option_;

  SharedHandle<StatCalc> statCalc_;

  SharedHandle<OutputFile> summaryOut_;

  SharedHandle<UriListParser> uriListParser_;

  SharedHandle<DownloadEngine> e_;

  sigset_t mask_;

  bool useSignalHandler_;

  void printMessageForContinue();
  void setupSignalHandlers();
  void resetSignalHandlers();
public:
  /*
   * MultiRequestInfo effectively takes ownership of the
   * requestGroups.
   */
  MultiUrlRequestInfo
  (std::vector<SharedHandle<RequestGroup> >& requestGroups,
   const SharedHandle<Option>& op,
   const SharedHandle<StatCalc>& statCalc,
   const SharedHandle<OutputFile>& summaryOut,
   const SharedHandle<UriListParser>& uriListParser);

  virtual ~MultiUrlRequestInfo();

  // Returns FINISHED if all downloads have completed, otherwise returns the
  // last download result.
  //
  // This method actually calls prepare() and
  // getDownloadEngine()->run(true) and getResult().
  error_code::Value execute();

  // Performs preparations for downloads, including creating
  // DownloadEngine instance. This function returns 0 if it succeeds,
  // or -1.
  int prepare();

  // Performs finalization of download process, including saving
  // sessions. This function returns last error code in this session,
  // in particular, this function returns FINISHED if all downloads
  // have completed.
  error_code::Value getResult();

  const SharedHandle<DownloadEngine>& getDownloadEngine() const;

  // Signal handlers are not prepared if false is given.
  void setUseSignalHandler(bool useSignalHandler)
  {
    useSignalHandler_ = useSignalHandler;
  }
};

} // namespace aria2

#endif // D_MULTI_URL_REQUEST_INFO_H
