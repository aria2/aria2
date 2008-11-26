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
#ifndef _D_HTTP_RESPONSE_COMMAND_H_
#define _D_HTTP_RESPONSE_COMMAND_H_

#include "AbstractCommand.h"
#include "Decoder.h"
#include "TimeA2.h"

namespace aria2 {

class HttpConnection;
class HttpDownloadCommand;
class HttpResponse;
class SocketCore;

class HttpResponseCommand : public AbstractCommand {
private:
  SharedHandle<HttpConnection> httpConnection;

  bool handleDefaultEncoding(const SharedHandle<HttpResponse>& httpResponse);
  bool handleOtherEncoding(const SharedHandle<HttpResponse>& httpResponse);
  bool skipResponseBody(const SharedHandle<HttpResponse>& httpResponse);

  HttpDownloadCommand*
  createHttpDownloadCommand(const SharedHandle<HttpResponse>& httpResponse,
			    const SharedHandle<Decoder>& transferEncodingDecoder
			    = SharedHandle<Decoder>(),
			    const SharedHandle<Decoder>& contentEncodingDecoder
			    = SharedHandle<Decoder>());

  void updateLastModifiedTime(const Time& lastModified);

  void poolConnection();
protected:
  bool executeInternal();

  bool shouldInflateContentEncoding
  (const SharedHandle<HttpResponse>& httpResponse);

public:
  HttpResponseCommand(int32_t cuid,
		      const SharedHandle<Request>& req,
		      RequestGroup* requestGroup,
		      const SharedHandle<HttpConnection>& httpConnection,
		      DownloadEngine* e,
		      const SharedHandle<SocketCore>& s);
  ~HttpResponseCommand();
};

} // namespace aria2

#endif // _D_HTTP_RESPONSE_COMMAND_H_
