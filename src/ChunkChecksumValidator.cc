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
#include "ChunkChecksumValidator.h"
#include "TimeA2.h"
#include "message.h"

void ChunkChecksumValidator::validate()
{
  /*
  if(!_validator->canValidate()) {
    // insufficient checksums.
    logger->error(MSG_INSUFFICIENT_CHECKSUM,
		  _validator->getChunkChecksum()->getChecksumLength(),
		  _validator->getChunkChecksum()->countChecksum());
    return;
  }
  _validator->init();

  int32_t numChecksum = _validator->getChunkChecksum()->countChecksum();
  fileAllocationMonitor->setMinValue(0);
  fileAllocationMonitor->setMaxValue(numChecksum);
  fileAllocationMonitor->setCurrentValue(0);
  fileAllocationMonitor->showProgress();
  Time cp;
  for(int32_t i = 0; i < numChecksum; ++i) {
    _validator->validateChunk();
    if(cp.elapsedInMillis(500)) {
      fileAllocationMonitor->setCurrentValue(i+1);
      fileAllocationMonitor->showProgress();
      cp.reset();
    }
  }
  fileAllocationMonitor->setCurrentValue(numChecksum);
  fileAllocationMonitor->showProgress();
*/
}
