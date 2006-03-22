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
#ifndef _D_SEGMENT_H_
#define _D_SEGMENT_H_

#include "common.h"
#include <deque>

using namespace std;

/**
 * Segment represents a download segment.
 * sp, ep is a offset from a begining of a file.
 * Therefore, if a file size is x, then 0 <= sp <= ep <= x-1.
 * sp, ep is used in Http Range header.
 * e.g. Range: bytes=sp-ep
 * ds is downloaded bytes.
 * If a download of this segement is complete, finish must be set to true.
 */
typedef struct {
  int cuid;
  long long int sp;
  long long int ep;
  long long int ds;
  int speed;
  bool finish;
} Segment;

typedef deque<Segment> Segments;

#define SEGMENT_EQUAL(X, Y) (X.cuid == Y.cuid && X.sp == Y.sp && X.ep == Y.ep && X.ds == Y.ds && X.finish == Y.finish ? true : false)

#endif // _D_SEGMENT_H_

