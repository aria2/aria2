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
#ifndef D_HTTP_RESPONSE_COMMAND_H
#define D_HTTP_RESPONSE_COMMAND_H

#include "AbstractCommand.h"
#include "TimeA2.h"

namespace aria2 {

class HttpConnection;
class HttpDownloadCommand;
class HttpResponse;
class SocketCore;
class StreamFilter;
#ifdef ENABLE_MESSAGE_DIGEST
class Checksum;
#endif // ENABLE_MESSAGE_DIGEST

// HttpResponseCommand receives HTTP response header from remote
// server.  Because network I/O is non-blocking, execute() returns
// false when all response headers are not received.  Subsequent calls
// of execute() receives remaining response headers and when all
// headers are received, it examines headers.  If status code is 200
// or 206 and Range header satisfies requested range, it creates
// HttpDownloadCommand to receive response entity body.  Otherwise, it
// creates HttpSkipResponseCommand to receive response entity body and
// returns true. Handling errors such as 404 or redirect is handled in
// HttpSkipResponseCommand.
class HttpResponseCommand : public AbstractCommand {
private:
  SharedHandle<HttpConnection> httpConnection_;

  bool handleDefaultEncoding(const SharedHandle<HttpResponse>& httpResponse);
  bool handleOtherEncoding(const SharedHandle<HttpResponse>& httpResponse);
  bool skipResponseBody(const SharedHandle<HttpResponse>& httpResponse);

  HttpDownloadCommand*
  createHttpDownloadCommand
  (const SharedHandle<HttpResponse>& httpResponse,
   const SharedHandle<StreamFilter>& streamFilter);

  void updateLastModifiedTime(const Time& lastModified);

  void poolConnection();

  void onDryRunFileFound();
#ifdef ENABLE_MESSAGE_DIGEST
  // Returns true if dctx and checksum has same hash type and hash
  // value.  If they have same hash type but different hash value,
  // throws exception.  Otherwise returns false.
  bool checkChecksum
  (const SharedHandle<DownloadContext>& dctx,
   const Checksum& checksum);
#endif // ENABLE_MESSAGE_DIGEST
protected:
  bool executeInternal();

  bool shouldInflateContentEncoding
  (const SharedHandle<HttpResponse>& httpResponse);

public:
  HttpResponseCommand(cuid_t cuid,
                      const SharedHandle<Request>& req,
                      const SharedHandle<FileEntry>& fileEntry,
                      RequestGroup* requestGroup,
                      const SharedHandle<HttpConnection>& httpConnection,
                      DownloadEngine* e,
                      const SharedHandle<SocketCore>& s);
  ~HttpResponseCommand();
};

} // namespace aria2

#endif // D_HTTP_RESPONSE_COMMAND_H
