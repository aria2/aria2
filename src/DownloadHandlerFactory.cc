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
#include "DownloadHandlerFactory.h"
#include "MemoryBufferPreDownloadHandler.h"
#include "MetalinkPostDownloadHandler.h"
#include "BtPostDownloadHandler.h"
#include "DownloadHandlerConstants.h"
#include "ContentTypeRequestGroupCriteria.h"
#include "UTMetadataPostDownloadHandler.h"

namespace aria2 {

#ifdef ENABLE_METALINK

SharedHandle<PreDownloadHandler>
DownloadHandlerFactory::metalinkPreDownloadHandler_;

SharedHandle<PostDownloadHandler>
DownloadHandlerFactory::metalinkPostDownloadHandler_;

#endif // ENABLE_METALINK

#ifdef ENABLE_BITTORRENT

SharedHandle<PreDownloadHandler>
DownloadHandlerFactory::btPreDownloadHandler_;

SharedHandle<PostDownloadHandler>
DownloadHandlerFactory::btPostDownloadHandler_;

SharedHandle<PostDownloadHandler>
DownloadHandlerFactory::btMetadataPostDownloadHandler_;
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK

SharedHandle<PreDownloadHandler>
DownloadHandlerFactory::getMetalinkPreDownloadHandler()
{
  if(!metalinkPreDownloadHandler_) {
    metalinkPreDownloadHandler_.reset(new MemoryBufferPreDownloadHandler());

    SharedHandle<RequestGroupCriteria> criteria
      (new ContentTypeRequestGroupCriteria
       (getMetalinkContentTypes(), getMetalinkExtensions()));
    metalinkPreDownloadHandler_->setCriteria(criteria);
  }
  return metalinkPreDownloadHandler_;
}

SharedHandle<PostDownloadHandler>
DownloadHandlerFactory::getMetalinkPostDownloadHandler()
{
  if(!metalinkPostDownloadHandler_) {
    metalinkPostDownloadHandler_.reset(new MetalinkPostDownloadHandler());
  }
  return metalinkPostDownloadHandler_;
}

#endif // ENABLE_METALINK

#ifdef ENABLE_BITTORRENT

SharedHandle<PreDownloadHandler>
DownloadHandlerFactory::getBtPreDownloadHandler()
{
  if(!btPreDownloadHandler_) {
    btPreDownloadHandler_.reset
      (new bittorrent::MemoryBencodePreDownloadHandler());

    SharedHandle<RequestGroupCriteria> criteria
      (new ContentTypeRequestGroupCriteria
       (getBtContentTypes(), getBtExtensions()));
    btPreDownloadHandler_->setCriteria(criteria);
  }
  return btPreDownloadHandler_;
}

SharedHandle<PostDownloadHandler>
DownloadHandlerFactory::getBtPostDownloadHandler()
{
  if(!btPostDownloadHandler_) {
    btPostDownloadHandler_.reset(new BtPostDownloadHandler());
  }
  return btPostDownloadHandler_;
}

SharedHandle<PostDownloadHandler>
DownloadHandlerFactory::getUTMetadataPostDownloadHandler()
{
  if(!btMetadataPostDownloadHandler_) {
    btMetadataPostDownloadHandler_.reset(new UTMetadataPostDownloadHandler());
  }
  return btMetadataPostDownloadHandler_;
}

#endif // ENABLE_BITTORRENT

} // namespace aria2
