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
#include "download_handlers.h"
#include "DownloadHandlerConstants.h"
#include "ContentTypeRequestGroupCriteria.h"
#include "MemoryBufferPreDownloadHandler.h"
#include "a2functional.h"
#ifdef ENABLE_METALINK
#  include "MetalinkPostDownloadHandler.h"
#endif // ENABLE_METALINK
#ifdef ENABLE_BITTORRENT
#  include "BtPostDownloadHandler.h"
#  include "MemoryBencodePreDownloadHandler.h"
#  include "UTMetadataPostDownloadHandler.h"
#endif // ENABLE_BITTORRENT

namespace aria2 {

namespace download_handlers {

namespace {
std::unique_ptr<PreDownloadHandler> memoryPreDownloadHandler;
} // namespace

const PreDownloadHandler* getMemoryPreDownloadHandler()
{
  if (!memoryPreDownloadHandler) {
    memoryPreDownloadHandler = make_unique<MemoryBufferPreDownloadHandler>();
  }
  return memoryPreDownloadHandler.get();
}

#ifdef ENABLE_METALINK

namespace {
std::unique_ptr<PreDownloadHandler> metalinkPreDownloadHandler;
std::unique_ptr<PostDownloadHandler> metalinkPostDownloadHandler;
} // namespace

const PreDownloadHandler* getMetalinkPreDownloadHandler()
{
  if (!metalinkPreDownloadHandler) {
    metalinkPreDownloadHandler = make_unique<MemoryBufferPreDownloadHandler>();
    metalinkPreDownloadHandler->setCriteria(
        make_unique<ContentTypeRequestGroupCriteria>(getMetalinkContentTypes(),
                                                     getMetalinkExtensions()));
  }
  return metalinkPreDownloadHandler.get();
}

const PostDownloadHandler* getMetalinkPostDownloadHandler()
{
  if (!metalinkPostDownloadHandler) {
    metalinkPostDownloadHandler = make_unique<MetalinkPostDownloadHandler>();
  }
  return metalinkPostDownloadHandler.get();
}

#endif // ENABLE_METALINK

#ifdef ENABLE_BITTORRENT

namespace {
std::unique_ptr<PreDownloadHandler> btPreDownloadHandler;
std::unique_ptr<PostDownloadHandler> btPostDownloadHandler;
std::unique_ptr<PostDownloadHandler> btMetadataPostDownloadHandler;
} // namespace

const PreDownloadHandler* getBtPreDownloadHandler()
{
  if (!btPreDownloadHandler) {
    btPreDownloadHandler =
        make_unique<bittorrent::MemoryBencodePreDownloadHandler>();
    btPreDownloadHandler->setCriteria(
        make_unique<ContentTypeRequestGroupCriteria>(getBtContentTypes(),
                                                     getBtExtensions()));
  }
  return btPreDownloadHandler.get();
}

const PostDownloadHandler* getBtPostDownloadHandler()
{
  if (!btPostDownloadHandler) {
    btPostDownloadHandler = make_unique<BtPostDownloadHandler>();
  }
  return btPostDownloadHandler.get();
}

const PostDownloadHandler* getUTMetadataPostDownloadHandler()
{
  if (!btMetadataPostDownloadHandler) {
    btMetadataPostDownloadHandler =
        make_unique<UTMetadataPostDownloadHandler>();
  }
  return btMetadataPostDownloadHandler.get();
}

#endif // ENABLE_BITTORRENT

} // namespace download_handlers

} // namespace aria2
