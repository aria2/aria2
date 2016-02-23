SUMMARY
=======
This is a port of expat for AmigaOS 4.x which includes the
SDK, some XML tools and the libraries.

Four library flavours are supported:
1. static clib2 (libexpat.a)
2. static newlib (libexpat.a)
3. AmigaOS library (expat.library)
4. AmigaOS shared object library (libexpat.so)

The AmigaOS library version is based on the work of Fredrik Wikstrom.


BUILDING
========
To build all the library flavours, all the tools, examples and run the
test suite, simply type 'make all' in the amiga subdirectory.


INSTALLATION
============
To install expat into the standard AmigaOS SDK type 'make install'
in the amiga subdirectory.


CONFIGURATION
=============
You may want to edit the lib/amigaconfig.h file to remove
DTD and/or XML namespace support if they are not required by your
specific application for a smaller and faster implementation.


SOURCE CODE
===========
The source code is actively maintained and merged with the official
Expat repository available at http://expat.sourceforge.net/


HISTORY
=======
53.1 - bumped version to match AmigaOS streaming
     - modified to remove all global variables (except INewLib)
     - removed replacements for malloc(), etc. which are now
       handled by the respective C library
     - compiled with the latest binutils which bumps the
       AMIGAOS_DYNVERSION to 2 for the libexpat.so target
     - now strips the expat.library binary

5.2  - fixed XML_Parse 68k stub which enables xmlviewer to work
       without crashing
     - added some new functions to the 68k jump table available
       in the latest expat.library for AmigaOS 3.x
     - patches provided by Fredrik Wikstrom

5.1  - fixed package archive which was missing libexpat.so
     - fixed library protection bits
     - fixed up copyright notices

5.0  - integrated 68k patches from Fredrik Wikstrom which means
       expat.library is now callable from 68k code
     - bumped version for the addition of the 68k interface so
       executables can explicitly ask for version 5 and know
       it includes the 68k interface
     - refactored Makefile to avoid recursive make calls and
       build all the library flavours
     - added static newlib version
     - added shared objects version
     - added package target to Makefile
     - compiled with SDK 53.13 (GCC 4.2.4) at -O3

4.2  - updated to correspond to Expat 2.0.1 release
     - bumped copyright banners and versions
     - simplified amigaconfig.h
     - updated include/libraries/expat.h file
     - modified launch.c to use contructor/deconstructor
     - removed need for amiga_main() from expat utilities

4.1  - fixed memory freeing bug in shared library version
     - now allocates shared memory

4.0  - updated for corresponding Expat 2.0 release
     - some minor CVS related changes

3.1  - removed obsolete sfd file
     - added library description xml file
     - refactored Makefile
     - removed extraneous VARARGS68K keywords
     - reworked default memory handling functions in shared lib
     - updated amigaconfig.h

3.0  - initial release
     - based on expat 1.95.8


TO DO
=====
- wide character support (UTF-16)
