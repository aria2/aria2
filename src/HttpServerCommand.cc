/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#include "HttpServerCommand.h"

#include <sstream>
#include <algorithm>
#include <iostream>
#include <iomanip>

#include "SocketCore.h"
#include "DownloadEngine.h"
#include "HttpServer.h"
#include "HttpHeader.h"
#include "Logger.h"
#include "RequestGroup.h"
#include "RequestGroupMan.h"
#include "BtContext.h"
#include "Util.h"
#include "HttpServerResponseCommand.h"
#include "CheckIntegrityEntry.h"
#include "FileAllocationEntry.h"

namespace aria2 {

HttpServerCommand::HttpServerCommand(int32_t cuid, DownloadEngine* e,
				     const SharedHandle<SocketCore>& socket):
  Command(cuid),
  _e(e),
  _socket(socket),
  _httpServer(new HttpServer(socket, e))
{
  _e->addSocketForReadCheck(_socket, this);
}

HttpServerCommand::~HttpServerCommand()
{
  _e->deleteSocketForReadCheck(_socket, this);
}

class PrintSummaryHtml
{
private:
  std::ostream& _o;
  
public:
  PrintSummaryHtml(std::ostream& o):_o(o) {}

  void operator()(const SharedHandle<RequestGroup>& rg)
  {
    _o << "<div id=\"gid" << rg->getGID() << "\">"
       << "[#" << rg->getGID() << "]"
       << " FILE:" << "<strong>"
       << Util::htmlEscape(rg->getFilePath()) << "</strong>";
#ifdef ENABLE_BITTORRENT
    SharedHandle<BtContext> btContext =
      dynamic_pointer_cast<BtContext>(rg->getDownloadContext());
    if(!btContext.isNull()) {
      _o << "<br />" << "  Info Hash:" << btContext->getInfoHashAsString();
    }
#endif // ENABLE_BITTORRENT
    _o << "<br />";

    TransferStat stat = rg->calculateStat();
    unsigned int eta = 0;
    if(rg->getTotalLength() > 0 && stat.getDownloadSpeed() > 0) {
      eta =
	(rg->getTotalLength()-rg->getCompletedLength())/stat.getDownloadSpeed();
    }
#ifdef ENABLE_BITTORRENT
    if(!btContext.isNull() && rg->downloadFinished()) {
      _o << "SEEDING" << "(" << "ratio:"
	 << std::fixed << std::setprecision(1)
	 << ((stat.getAllTimeUploadLength()*10)/rg->getCompletedLength())/10.0
	 << ") ";
    }
#endif // ENABLE_BITTORRENT
    _o << Util::abbrevSize(rg->getCompletedLength())
       << "B"
       << "/"
       << Util::abbrevSize(rg->getTotalLength())
       << "B";
    if(rg->getTotalLength() > 0) {
      _o << "("
	 << 100*rg->getCompletedLength()/rg->getTotalLength()
	 << "%)";
    }
    _o << " "
       << "CN:"
       << rg->getNumConnection();
    if(!rg->downloadFinished()) {
      _o << " "
	 << "SPD:"
	 << std::fixed << std::setprecision(2)
	 << stat.getDownloadSpeed()/1024.0 << "KiB/s";
    }
    if(stat.getSessionUploadLength() > 0) {
      _o << " "
	 << "UP:"
	 << std::fixed << std::setprecision(2)
	 << stat.getUploadSpeed()/1024.0 << "KiB/s"
	 << "(" << Util::abbrevSize(stat.getAllTimeUploadLength()) << "B)";
    }
    if(eta > 0) {
      _o << " "
	<< "ETA:"
	 << Util::htmlEscape(Util::secfmt(eta));
    }
    _o << "</div>"
       << "<hr />";
  }
};

static std::string createResponse(DownloadEngine* e)
{
  std::ostringstream strm;
  const std::deque<SharedHandle<RequestGroup> > groups =
    e->_requestGroupMan->getRequestGroups();
  std::for_each(groups.begin(), groups.end(), PrintSummaryHtml(strm));

  {
    SharedHandle<FileAllocationEntry> entry =
      e->_fileAllocationMan->getPickedEntry();
    if(!entry.isNull()) {
      strm << "<div id=\"filealloc\">"
	   << "[FileAlloc:"
	   << "#" << entry->getRequestGroup()->getGID() << " "
	   << Util::abbrevSize(entry->getCurrentLength())
	   << "B"
	   << "/"
	   << Util::abbrevSize(entry->getTotalLength())
	   << "B"
	   << "(";
      if(entry->getTotalLength() > 0) {
	strm << 100*entry->getCurrentLength()/entry->getTotalLength();
      } else {
	strm << "--";
      }
      strm << "%)"
	<< "]";
      if(e->_fileAllocationMan->hasNext()) {
	strm << "("
	     << e->_fileAllocationMan->countEntryInQueue()
	     << "waiting...)";
      }
      strm << "</div><hr />";
    }
  }
#ifdef ENABLE_MESSAGE_DIGEST
  {
    SharedHandle<CheckIntegrityEntry> entry =
      e->_checkIntegrityMan->getPickedEntry();
    if(!entry.isNull()) {
      strm << "<div id=\"hashcheck\">"
	   << "[HashCheck:"
	   << "#" << entry->getRequestGroup()->getGID() << " "
	   << Util::abbrevSize(entry->getCurrentLength())
	   << "B"
	   << "/"
	   << Util::abbrevSize(entry->getTotalLength())
	   << "B"
	   << "("
	   << 100*entry->getCurrentLength()/entry->getTotalLength()
	   << "%)"
	   << "]";
      if(e->_checkIntegrityMan->hasNext()) {
	strm << "("
	     << e->_checkIntegrityMan->countEntryInQueue()
	     << "waiting...)";
      }
      strm << "</div><hr />";
    }
  }
#endif // ENABLE_MESSAGE_DIGEST
  std::string body =
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\""
    " \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en\">"
    "<head>"
    "<meta http-equiv=\"refresh\" content=\"1\" />"
    "<title>aria2</title>"
    "</head>"
    "<body><h1>aria2 - Download Progress</h1>"+strm.str()+"</body>"
    "</html>";
  return body;
}

bool HttpServerCommand::execute()
{
  if(_socket->isReadable(0)) {
    _timeout.reset();
    SharedHandle<HttpHeader> header = _httpServer->receiveRequest();
    if(header.isNull()) {
      _e->commands.push_back(this);
      return false;
    } else {
      _httpServer->feedResponse(createResponse(_e));
      Command* command = new HttpServerResponseCommand(cuid, _httpServer, _e,
						       _socket);
      command->setStatus(Command::STATUS_ONESHOT_REALTIME);
      _e->commands.push_back(command);
      _e->setNoWait(true);
      return true;
    }
  } else {
    if(_timeout.elapsed(30)) {
      logger->info("HTTP request timeout.");
      return true;
    } else {
      _e->commands.push_back(this);
      return false;
    }
  }
}

} // namespace aria2
