/* __gmp_rands -- global random state for old-style random functions.

   EVERYTHING IN THIS FILE IS FOR INTERNAL USE ONLY.  IT'S ALMOST CERTAIN TO
   BE SUBJECT TO INCOMPATIBLE CHANGES OR DISAPPEAR COMPLETELY IN FUTURE GNU
   MP RELEASES.  */

/*
Copyright 2001 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

The GNU MP Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.  */

#include "gmp.h"
#include "gmp-impl.h"


/* Use this via the RANDS macro in gmp-impl.h */
char             __gmp_rands_initialized = 0;
gmp_randstate_t  __gmp_rands;
