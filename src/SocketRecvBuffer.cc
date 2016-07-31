/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2011 Tatsuhiro Tsujikawa
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
#include "SocketRecvBuffer.h"

#include <cstring>
#include <cassert>

#include "SocketCore.h"
#include "LogFactory.h"

namespace aria2 {

SocketRecvBuffer::SocketRecvBuffer(std::shared_ptr<SocketCore> socket)
    : socket_(std::move(socket)), pos_(buf_.data()), last_(pos_)
{
}

SocketRecvBuffer::~SocketRecvBuffer() = default;

ssize_t SocketRecvBuffer::recv()
{
  size_t n = std::end(buf_) - last_;
  if (n == 0) {
    A2_LOG_DEBUG("Buffer full");
    return 0;
  }
  socket_->readData(last_, n);
  last_ += n;
  return n;
}

void SocketRecvBuffer::drain(size_t n)
{
  assert(pos_ + n <= last_);
  pos_ += n;
  if (pos_ == last_) {
    truncateBuffer();
  }
}

void SocketRecvBuffer::truncateBuffer() { pos_ = last_ = buf_.data(); }

} // namespace aria2
