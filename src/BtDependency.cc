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
#include "BtDependency.h"
#include "RequestGroup.h"
#include "Option.h"
#include "LogFactory.h"
#include "Logger.h"
#include "DefaultBtContext.h"
#include "RecoverableException.h"
#include "message.h"
#include "prefs.h"
#include "Util.h"
#include "PieceStorage.h"
#include "DiskAdaptor.h"
#include "File.h"

namespace aria2 {

BtDependency::BtDependency(const RequestGroupWeakHandle& dependant,
			   const RequestGroupHandle& dependee,
			   const Option* option):
  _dependant(dependant),
  _dependee(dependee),
  _option(option),
  _logger(LogFactory::getInstance()) {}

BtDependency::~BtDependency() {}

bool BtDependency::resolve()
{
  if(_dependee->getNumCommand() == 0 && _dependee->downloadFinished()) {
    RequestGroupHandle dependee = _dependee;
    // cut reference here
    _dependee = 0;
    DefaultBtContextHandle btContext = new DefaultBtContext();
    try {
      DiskAdaptorHandle diskAdaptor = dependee->getPieceStorage()->getDiskAdaptor();
      diskAdaptor->openExistingFile();
      std::string content = Util::toString(diskAdaptor);
      btContext->loadFromMemory(content,
				File(dependee->getFilePath()).getBasename());
      if(_option->defined(PREF_PEER_ID_PREFIX)) {
	btContext->setPeerIdPrefix(_option->get(PREF_PEER_ID_PREFIX));
      }
      btContext->setDir(_dependant->getDownloadContext()->getDir());
    } catch(RecoverableException* e) {
      _logger->error(EX_EXCEPTION_CAUGHT, e);
      delete e;
      _logger->debug("BtDependency for GID#%d failed. Go without Bt.",
		     _dependant->getGID());
      return true;
    }
    _logger->debug("Dependency resolved for GID#%d", _dependant->getGID());
    _dependant->setDownloadContext(btContext);
    btContext->setOwnerRequestGroup(_dependant.get());
    return true;
  } else if(_dependee->getNumCommand() == 0) {
    // _dependee's download failed.
    // cut reference here
    _dependee = 0;
    _logger->debug("BtDependency for GID#%d failed. Go without Bt.",
		   _dependant->getGID());    
    return true;
  } else {
    return false;
  }
}

} // namespace aria2
