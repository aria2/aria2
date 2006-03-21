/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#ifndef _D_PRE_ALLOCATION_DISK_WRITER_H_
#define _D_PRE_ALLOCATION_DISK_WRITER_H_

#include "AbstractDiskWriter.h"

class PreAllocationDiskWriter:public AbstractDiskWriter {
private:
  long long int totalLength;
public:
  PreAllocationDiskWriter(long long int totalLength);
  ~PreAllocationDiskWriter();

  void initAndOpenFile(string filename);
};

#endif // _D_PRE_ALLOCATION_DISK_WRITER_H_

