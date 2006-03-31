/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#ifndef _D_FTP_NEGOTIATION_COMMAND_H_
#define _D_FTP_NEGOTIATION_COMMAND_H_

#include "AbstractCommand.h"
#include "FtpConnection.h"

class FtpNegotiationCommand : public AbstractCommand {
private:
  enum Seq {
    SEQ_RECV_GREETING,
    SEQ_SEND_USER,
    SEQ_RECV_USER,
    SEQ_SEND_PASS,
    SEQ_RECV_PASS,
    SEQ_SEND_TYPE,
    SEQ_RECV_TYPE,
    SEQ_SEND_CWD,
    SEQ_RECV_CWD,
    SEQ_SEND_SIZE,
    SEQ_RECV_SIZE,
    SEQ_SEND_PORT,
    SEQ_RECV_PORT,
    SEQ_SEND_PASV,
    SEQ_RECV_PASV,
    SEQ_SEND_REST_PASV,
    SEQ_SEND_REST,
    SEQ_RECV_REST,
    SEQ_SEND_RETR,
    SEQ_RECV_RETR,
    SEQ_NEGOTIATION_COMPLETED,
    SEQ_RETRY
  };
  bool recvGreeting();
  bool sendUser();
  bool recvUser();
  bool sendPass();
  bool recvPass();
  bool sendType();
  bool recvType();
  bool sendCwd();
  bool recvCwd();
  bool sendSize();
  bool recvSize();
  bool sendPort();
  bool recvPort();
  bool sendPasv();
  bool recvPasv();
  bool sendRest(const Segment& segment);
  bool sendRestPasv(const Segment& segment);
  bool recvRest();
  bool sendRetr();
  bool recvRetr();
  bool processSequence(const Segment& segment);

  Socket* dataSocket;
  Socket* serverSocket;
  int sequence;
  FtpConnection* ftp;
protected:
  bool executeInternal(Segment segment);
public:
  FtpNegotiationCommand(int cuid, Request* req, DownloadEngine* e, const Socket* s);
  ~FtpNegotiationCommand();
};

#endif // _D_FTP_NEGOTIATION_COMMAND_H_
