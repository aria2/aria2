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
#include "MetalinkHelper.h"
#include "Option.h"
#include "MetalinkEntry.h"
#include "Xml2MetalinkProcessor.h"
#include "Metalinker.h"
#include "prefs.h"

MetalinkHelper::MetalinkHelper() {}

MetalinkHelper::~MetalinkHelper() {}

MetalinkEntries MetalinkHelper::parseAndQuery(const string& filename, const Option* option)
{
  Xml2MetalinkProcessor proc;

  MetalinkerHandle metalinker = proc.parseFile(filename);
  if(metalinker->entries.empty()) {
    throw new DlAbortEx("No file entry found. Probably, the metalink file is not configured properly or broken.");
  }
  MetalinkEntries entries =
    metalinker->queryEntry(option->get(PREF_METALINK_VERSION),
			   option->get(PREF_METALINK_LANGUAGE),
			   option->get(PREF_METALINK_OS));
  return entries;
}
