/* Copyright 2000, Clark Cooper
   All rights reserved.

   This is free software. You are permitted to copy, distribute, or modify
   it under the terms of the MIT/X license (contained in the COPYING file
   with this distribution.)
*/

/* Define to empty if the keyword does not work.  */
#undef const

/* Define if you have a working `mmap' system call.  */
#undef HAVE_MMAP

/* Define to `long' if <sys/types.h> doesn't define.  */
#undef off_t

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
#undef size_t

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
#undef WORDS_BIGENDIAN

/* Define if you have the bcopy function.  */
#undef HAVE_BCOPY

/* Define if you have the memmove function.  */
#define HAVE_MEMMOVE 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

#define XML_NS
#define XML_DTD

#ifdef WORDS_BIGENDIAN
#define XML_BYTE_ORDER 21
#else
#define XML_BYTE_ORDER 12
#endif

#define XML_CONTEXT_BYTES 1024

#ifndef HAVE_MEMMOVE
#ifdef HAVE_BCOPY
#define memmove(d,s,l) bcopy((s),(d),(l))
#else
#define memmove(d,s,l) ;punting on memmove;
#endif

#endif
