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
#include "DownloadHandlerFactory.h"
#include "MemoryBufferPreDownloadHandler.h"
#include "MetalinkPostDownloadHandler.h"
#include "BtPostDownloadHandler.h"
#include "DownloadHandlerConstants.h"
#include "ContentTypeRequestGroupCriteria.h"
#include "UTMetadataPostDownloadHandler.h"

namespace aria2 {

#ifdef ENABLE_METALINK

MemoryBufferPreDownloadHandlerHandle
DownloadHandlerFactory::_metalinkPreDownloadHandler;

MetalinkPostDownloadHandlerHandle
DownloadHandlerFactory::_metalinkPostDownloadHandler;

#endif // ENABLE_METALINK

#ifdef ENABLE_BITTORRENT

MemoryBufferPreDownloadHandlerHandle
DownloadHandlerFactory::_btPreDownloadHandler;

BtPostDownloadHandlerHandle
DownloadHandlerFactory::_btPostDownloadHandler;

SharedHandle<UTMetadataPostDownloadHandler>
DownloadHandlerFactory::_btMetadataPostDownloadHandler;
#endif // ENABLE_BITTORRENT

#ifdef ENABLE_METALINK

MemoryBufferPreDownloadHandlerHandle
DownloadHandlerFactory::getMetalinkPreDownloadHandler()
{
  if(_metalinkPreDownloadHandler.isNull()) {
    _metalinkPreDownloadHandler.reset(new MemoryBufferPreDownloadHandler());

    RequestGroupCriteriaHandle criteria
      (new ContentTypeRequestGroupCriteria
       (DownloadHandlerConstants::getMetalinkContentTypes().begin(),
	DownloadHandlerConstants::getMetalinkContentTypes().end(),
	DownloadHandlerConstants::getMetalinkExtensions().begin(),
	DownloadHandlerConstants::getMetalinkExtensions().end()));
    _metalinkPreDownloadHandler->setCriteria(criteria);
  }
  return _metalinkPreDownloadHandler;
}

MetalinkPostDownloadHandlerHandle
DownloadHandlerFactory::getMetalinkPostDownloadHandler()
{
  if(_metalinkPostDownloadHandler.isNull()) {
    _metalinkPostDownloadHandler.reset(new MetalinkPostDownloadHandler());
  }
  return _metalinkPostDownloadHandler;
}

#endif // ENABLE_METALINK

#ifdef ENABLE_BITTORRENT

MemoryBufferPreDownloadHandlerHandle
DownloadHandlerFactory::getBtPreDownloadHandler()
{
  if(_btPreDownloadHandler.isNull()) {
    _btPreDownloadHandler.reset(new MemoryBufferPreDownloadHandler());

    RequestGroupCriteriaHandle criteria
      (new ContentTypeRequestGroupCriteria
       (DownloadHandlerConstants::getBtContentTypes().begin(),
	DownloadHandlerConstants::getBtContentTypes().end(),
	DownloadHandlerConstants::getBtExtensions().begin(),
	DownloadHandlerConstants::getBtExtensions().end()));
    _btPreDownloadHandler->setCriteria(criteria);
  }
  return _btPreDownloadHandler;
}

BtPostDownloadHandlerHandle DownloadHandlerFactory::getBtPostDownloadHandler()
{
  if(_btPostDownloadHandler.isNull()) {
    _btPostDownloadHandler.reset(new BtPostDownloadHandler());
  }
  return _btPostDownloadHandler;
}

SharedHandle<UTMetadataPostDownloadHandler>
DownloadHandlerFactory::getUTMetadataPostDownloadHandler()
{
  if(_btMetadataPostDownloadHandler.isNull()) {
    _btMetadataPostDownloadHandler.reset(new UTMetadataPostDownloadHandler());
  }
  return _btMetadataPostDownloadHandler;
}

#endif // ENABLE_BITTORRENT

} // namespace aria2
