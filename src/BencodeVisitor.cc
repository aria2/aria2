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
#include "BencodeVisitor.h"
#include "Data.h"
#include "List.h"
#include "Dictionary.h"
#include "Util.h"

BencodeVisitor::BencodeVisitor() {}

BencodeVisitor::~BencodeVisitor() {}

void BencodeVisitor::visit(const Data* d)
{
  if(d->isNumber()) {
    _bencodedData += "i"+d->toString()+"e";
  } else {
    _bencodedData += Util::itos(d->getLen())+":"+d->toString();
  }
}

void BencodeVisitor::visit(const List* l)
{
  _bencodedData += "l";
  for_each(l->getList().begin(), l->getList().end(),
	   bind2nd(mem_fun(&MetaEntry::accept), this));
  _bencodedData += "e";
}

void BencodeVisitor::visit(const Dictionary* d)
{
  _bencodedData += "d";

  for(Order::const_iterator itr = d->getOrder().begin(); itr != d->getOrder().end(); ++itr) {
    _bencodedData += Util::itos((int32_t)(*itr).size());
    _bencodedData += ":";
    _bencodedData += *itr;
    d->get(*itr)->accept(this);
  }
  _bencodedData += "e";
}

void BencodeVisitor::visit(const MetaEntry* e)
{
  if(dynamic_cast<const Data*>(e) != 0) {
    visit(reinterpret_cast<const Data*>(e));
  } else if(dynamic_cast<const List*>(e) != 0) {
    visit(reinterpret_cast<const List*>(e));
  } else if(dynamic_cast<const Dictionary*>(e) != 0) {
    visit(reinterpret_cast<const Dictionary*>(e));
  }
}
