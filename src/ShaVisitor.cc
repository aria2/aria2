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
#include "ShaVisitor.h"
#include "Util.h"

ShaVisitor::ShaVisitor():
  ctx(DIGEST_ALGO_SHA1) {
  ctx.digestInit();
}

ShaVisitor::~ShaVisitor() {
  ctx.digestFree();
}

void ShaVisitor::visit(const Data* d) {
  if(d->isNumber()) {
    ctx.digestUpdate("i", 1);
  } else {
    string lenStr = Util::llitos(d->getLen());
    ctx.digestUpdate(lenStr.c_str(), lenStr.size());
    ctx.digestUpdate(":", 1);
  }
  ctx.digestUpdate(d->getData(), d->getLen());
  if(d->isNumber()) {
    ctx.digestUpdate("e", 1);
  }
}

void ShaVisitor::visit(const Dictionary* d) {
  ctx.digestUpdate("d", 1);
  const Order& v = d->getOrder();
  for(Order::const_iterator itr = v.begin(); itr != v.end(); itr++) {
    string lenStr = Util::llitos(itr->size());
    ctx.digestUpdate(lenStr.c_str(), lenStr.size());
    ctx.digestUpdate(":", 1);
    ctx.digestUpdate(itr->c_str(), itr->size());
    const MetaEntry* e = d->get(*itr);
    this->visit(e);
  }
  ctx.digestUpdate("e", 1);
}

void ShaVisitor::visit(const List* l) {
  ctx.digestUpdate("l", 1);
  for(MetaList::const_iterator itr = l->getList().begin(); itr != l->getList().end(); itr++) {
    this->visit(*itr);
  }
  ctx.digestUpdate("e", 1);
}

void ShaVisitor::visit(const MetaEntry* e) {
  if(dynamic_cast<const Data*>(e) != NULL) {
    visit((const Data*)e);
  } else if(dynamic_cast<const Dictionary*>(e) != NULL) {
    visit((const Dictionary*)e);
  } else if(dynamic_cast<const List*>(e) != NULL) {
    visit((const List*)e);
  }
}

void ShaVisitor::getHash(unsigned char* hashValue, int& len) {
  len = ctx.digestLength();
  ctx.digestFinal(hashValue);
}
