/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2012 Tatsuhiro Tsujikawa
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
#ifndef D_NOTIFIER_H
#define D_NOTIFIER_H

#include "common.h"

#include <vector>
#include <memory>

#include <aria2/aria2.h>

namespace aria2 {

class RequestGroup;

struct DownloadEventListener {
  virtual ~DownloadEventListener() = default;
  virtual void onEvent(DownloadEvent event, const RequestGroup* group) = 0;
};

class Notifier {
public:
  Notifier();
  ~Notifier();
  void addDownloadEventListener(DownloadEventListener* listener);
  // Notifies the download event to all listeners.
  void notifyDownloadEvent(DownloadEvent event, const RequestGroup* group);

  void notifyDownloadEvent(DownloadEvent event,
                           const std::shared_ptr<RequestGroup>& group)
  {
    notifyDownloadEvent(event, group.get());
  }

private:
  std::vector<DownloadEventListener*> listeners_;
};

} // namespace aria2

#endif // D_NOTIFIER_H
