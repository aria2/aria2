/* List of exported symbols of libintl on Cygwin.
   Copyright (C) 2006 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published
   by the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.  */

 /* IMP(x) is a symbol that contains the address of x.  */
#define IMP(x) _imp__##x

 /* Ensure that the variable x is exported from the library, and that a
    pseudo-variable IMP(x) is available.  */
#define VARIABLE(x) \
 /* Export x without redefining x.  This code was found by compiling a	\
    snippet:								\
      extern __declspec(dllexport) int x; int x = 42;  */		\
 asm (".section .drectve\n");						\
 asm (".ascii \" -export:" #x ",data\"\n");				\
 asm (".data\n");							\
 /* Allocate a pseudo-variable IMP(x).  */				\
 extern int x;								\
 void * IMP(x) = &x;

VARIABLE(libintl_version)
