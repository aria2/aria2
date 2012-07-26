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
#ifndef D_DOWNLOAD_HANDLER_FACTORY_H
#define D_DOWNLOAD_HANDLER_FACTORY_H

#include "common.h"
#include "SharedHandle.h"
#include "MemoryBufferPreDownloadHandler.h"
#ifdef ENABLE_BITTORRENT
#  include "MemoryBencodePreDownloadHandler.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

#ifdef ENABLE_METALINK
class MetalinkPostDownloadHandler;
#endif // ENABLE_METALINK
#ifdef ENABLE_BITTORRENT
class BtPostDownloadHandler;
class UTMetadataPostDownloadHandler;
#endif // ENABLE_BITTORRENT

class DownloadHandlerFactory
{
private:
#ifdef ENABLE_METALINK
  static SharedHandle<MemoryBufferPreDownloadHandler>
  metalinkPreDownloadHandler_;

  static SharedHandle<MetalinkPostDownloadHandler>
  metalinkPostDownloadHandler_;
#endif // ENABLE_METALINK

#ifdef ENABLE_BITTORRENT
  static SharedHandle<bittorrent::MemoryBencodePreDownloadHandler>
  btPreDownloadHandler_;

  static SharedHandle<BtPostDownloadHandler>
  btPostDownloadHandler_;

  static SharedHandle<UTMetadataPostDownloadHandler>
  btMetadataPostDownloadHandler_;
#endif // ENABLE_BITTORRENT
public:
#ifdef ENABLE_METALINK
  static SharedHandle<MemoryBufferPreDownloadHandler>
  getMetalinkPreDownloadHandler();

  static SharedHandle<MetalinkPostDownloadHandler>
  getMetalinkPostDownloadHandler();
#endif // ENABLE_METALINK

#ifdef ENABLE_BITTORRENT
  static SharedHandle<bittorrent::MemoryBencodePreDownloadHandler>
  getBtPreDownloadHandler();

  static SharedHandle<BtPostDownloadHandler>
  getBtPostDownloadHandler();

  static SharedHandle<UTMetadataPostDownloadHandler>
  getUTMetadataPostDownloadHandler();
#endif // ENABLE_BITTORRENT
};

} // namespace aria2

#endif // D_DOWNLOAD_HANDLER_FACTORY_H
