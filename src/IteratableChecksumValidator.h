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
#ifndef _D_ITERATABLE_CHECKSUM_VALIDATOR_H_
#define _D_ITERATABLE_CHECKSUM_VALIDATOR_H_

#include "common.h"
#include "BitfieldMan.h"
#include "Checksum.h"
#include "DiskWriter.h"
#include "LogFactory.h"
#include "messageDigest.h"

class IteratableChecksumValidator
{
private:
  DiskWriterHandle _diskWriter;
  BitfieldMan* _bitfield;
  int64_t _currentOffset;
  ChecksumHandle _checksum;
  const Logger* logger;
  MessageDigestContextHandle _ctx;

  string calculateActualChecksum();
public:
  IteratableChecksumValidator():_diskWriter(0), _bitfield(0), _currentOffset(0), _checksum(0), logger(LogFactory::getInstance()), _ctx(0) {}

  bool canValidate() const;

  void init();

  void validateChunk();

  bool finished() const
  {
    return _currentOffset >= _bitfield->getTotalLength();
  }

  void setDiskWriter(const DiskWriterHandle& diskWriter)
  {
    _diskWriter = diskWriter;
  }

  void setBitfield(BitfieldMan* bitfield)
  {
    _bitfield = bitfield;
  }

  void setChecksum(const ChecksumHandle& checksum)
  {
    _checksum = checksum;
  }

  int64_t getCurrentOffset() const
  {
    return _currentOffset;
  }
};

typedef SharedHandle<IteratableChecksumValidator> IteratableChecksumValidatorHandle;

#endif // _D_ITERATABLE_CHECKSUM_VALIDATOR_H_
