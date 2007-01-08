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
#include "DefaultDiskWriter.h"
#include "DlAbortEx.h"
#include "message.h"
#include "FileAllocator.h"
#include "prefs.h"
#include "Util.h"
#include <errno.h>
#include <unistd.h>

DefaultDiskWriter::DefaultDiskWriter():AbstractDiskWriter() {}

DefaultDiskWriter::~DefaultDiskWriter() {}

void DefaultDiskWriter::initAndOpenFile(const string& filename,
					uint64_t totalLength)
{
  createFile(filename);
  try {
    if(totalLength > 0) {
      if(fileAllocator.isNull()) {
	ftruncate(fd, totalLength);
      } else {
	logger->notice("Allocating file %s, %s bytes",
		       filename.c_str(),
		       Util::ullitos(totalLength).c_str());
	fileAllocator->allocate(fd, totalLength);
      }
    }
  } catch(Exception *e) {
    throw new DlAbortEx(e, EX_FILE_WRITE, filename.c_str(), strerror(errno));
  }
}

DefaultDiskWriter* DefaultDiskWriter::createNewDiskWriter(const Option* option)
{
  DefaultDiskWriter* diskWriter = new DefaultDiskWriter();
  if(option->get(PREF_FILE_ALLOCATION) == V_PREALLOC) {
    FileAllocatorHandle allocator = new FileAllocator();
    allocator->setFileAllocationMonitor(FileAllocationMonitorFactory::getFactory()->createNewMonitor());
    diskWriter->setFileAllocator(allocator);
  } else {
    diskWriter->setFileAllocator(0);
  }
  return diskWriter;
}
