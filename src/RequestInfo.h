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
#ifndef _D_REQUEST_INFO_H_
#define _D_REQUEST_INFO_H_

#include "common.h"
#include "LogFactory.h"
#include "Option.h"
#include "DownloadEngine.h"
#include "Util.h"
#include "Checksum.h"
#include <signal.h>

class FileInfo {
public:
  string filename;
  long long int length;
  Checksum checksum;
public:
  FileInfo():length(0) {}
  ~FileInfo() {}

  bool isEmpty() const {
    return filename.size() == 0 && length == 0;
  }

  bool checkReady() const {
#ifdef ENABLE_MESSAGE_DIGEST
    return !isEmpty() && !checksum.isEmpty();
#else
    return false;
#endif // ENABLE_MESSAGE_DIGEST
  }

  bool check() const {
#ifdef ENABLE_MESSAGE_DIGEST
    unsigned char md[MAX_MD_LENGTH];
    Util::fileChecksum(filename, md, checksum.getDigestAlgo());
    return Util::toHex(md,
		       MessageDigestContext::digestLength(checksum.getDigestAlgo()))
      == checksum.getMessageDigest();
#else
    return false;
#endif // ENABLE_MESSAGE_DIGEST
  }

  bool isTorrentFile() const {
    return Util::endsWith(filename, ".torrent");
  }

  bool isMetalinkFile() const {
    return Util::endsWith(filename, ".metalink");
  }
};

class RequestInfo;

typedef SharedHandle<RequestInfo> RequestInfoHandle;
typedef deque<RequestInfoHandle> RequestInfos;

class RequestInfo {
protected:
  Option* op;
  const Logger* logger;
  Checksum checksum;
  FileInfo fileInfo;
  bool fail;

  void printDownloadCompeleteMessage(const string& filename) {
    printf(_("\nThe download was complete. <%s>\n"), filename.c_str());
  }
  
  void printDownloadCompeleteMessage() {
    printf("\nThe download was complete.\n");
  }
  
  void printDownloadAbortMessage() {
    printf(_("\nThe download was not complete because of errors."
	     " Check the log.\n"
	     "aria2 will resume download if the transfer is restarted."));
  }
public:
  RequestInfo(Option* op):
    op(op),
    fail(false)
  {
    logger = LogFactory::getInstance();
  }
  virtual ~RequestInfo() {}

  virtual RequestInfos execute() = 0;

  bool isFail() const { return fail; }

  void setChecksum(const Checksum& checksum) {
    this->checksum = checksum;
  }
  const Checksum& getChecksum() const { return checksum; }
  const FileInfo& getFileInfo() const { return fileInfo; }
};

#endif // _D_REQUEST_INFO_H_
