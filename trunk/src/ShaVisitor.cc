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

ShaVisitor::ShaVisitor() {
#ifdef ENABLE_SHA1DIGEST
  sha1DigestInit(ctx);
  sha1DigestReset(ctx);
#endif // ENABLE_SHA1DIGEST
}

ShaVisitor::~ShaVisitor() {
#ifdef ENABLE_SHA1DIGEST
  sha1DigestFree(ctx);
#endif // ENABLE_SHA1DIGEST
}

void ShaVisitor::visit(const Data* d) {
#ifdef ENABLE_SHA1DIGEST
  if(d->isNumber()) {
    sha1DigestUpdate(ctx, "i", 1);
  } else {
    string lenStr = Util::llitos(d->getLen());
    sha1DigestUpdate(ctx, lenStr.c_str(), lenStr.size());
    sha1DigestUpdate(ctx, ":", 1);
  }
  sha1DigestUpdate(ctx, d->getData(), d->getLen());
  if(d->isNumber()) {
    sha1DigestUpdate(ctx, "e", 1);
  }
#endif // ENABLE_SHA1DIGEST
}

void ShaVisitor::visit(const Dictionary* d) {
#ifdef ENABLE_SHA1DIGEST
  sha1DigestUpdate(ctx, "d", 1);
  const Order& v = d->getOrder();
  for(Order::const_iterator itr = v.begin(); itr != v.end(); itr++) {
    string lenStr = Util::llitos(itr->size());
    sha1DigestUpdate(ctx, lenStr.c_str(), lenStr.size());
    sha1DigestUpdate(ctx, ":", 1);
    sha1DigestUpdate(ctx, itr->c_str(), itr->size());
    const MetaEntry* e = d->get(*itr);
    this->visit(e);
  }
  sha1DigestUpdate(ctx, "e", 1);
#endif // ENABLE_SHA1DIGEST
}

void ShaVisitor::visit(const List* l) {
#ifdef ENABLE_SHA1DIGEST
  sha1DigestUpdate(ctx, "l", 1);
  for(MetaList::const_iterator itr = l->getList().begin(); itr != l->getList().end(); itr++) {
    this->visit(*itr);
  }
  sha1DigestUpdate(ctx, "e", 1);
#endif // ENABLE_SHA1DIGEST
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
#ifdef ENABLE_SHA1DIGEST
  sha1DigestFinal(ctx, hashValue);
  len = 20;
#endif // ENABLE_SHA1DIGEST
}
