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
#ifndef _D_A2IO_H_
#define _D_A2IO_H_
#include "common.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_IO_H
# include <io.h>
#endif // HAVE_IO_H

// in some platforms following definitions are missing:
#ifndef EINPROGRESS
# define EINPROGRESS (WSAEINPROGRESS)
#endif // EINPROGRESS

#ifndef O_NONBLOCK
# define O_NONBLOCK (O_NDELAY)
#endif // O_NONBLOCK

#ifndef O_BINARY
# define O_BINARY (0)
#endif // O_BINARY

// st_mode flags
#ifndef S_IRUSR
# define		S_IRUSR	0000400	/* read permission, owner */
#endif /* S_IRUSR	*/
#ifndef S_IWUSR
# define		S_IWUSR	0000200	/* write permission, owner */
#endif /* S_IWUSR	*/
#ifndef S_IXUSR
# define		S_IXUSR 0000100/* execute/search permission, owner */
#endif /* S_IXUSR */
#ifndef S_IRWXU
# define	S_IRWXU 	(S_IRUSR | S_IWUSR | S_IXUSR)
#endif /* S_IRWXU 	*/
#ifndef S_IRGRP
# define		S_IRGRP	0000040	/* read permission, group */
#endif /* S_IRGRP	*/
#ifndef S_IWGRP
# define		S_IWGRP	0000020	/* write permission, grougroup */
#endif /* S_IWGRP	*/
#ifndef S_IXGRP
# define		S_IXGRP 0000010/* execute/search permission, group */
#endif /* S_IXGRP */
#ifndef S_IRWXG
# define	S_IRWXG		(S_IRGRP | S_IWGRP | S_IXGRP)
#endif /* S_IRWXG		*/
#ifndef S_IROTH
# define		S_IROTH	0000004	/* read permission, other */
#endif /* S_IROTH	*/
#ifndef S_IWOTH
# define		S_IWOTH	0000002	/* write permission, other */
#endif /* S_IWOTH	*/
#ifndef S_IXOTH
# define		S_IXOTH 0000001/* execute/search permission, other */
#endif /* S_IXOTH */
#ifndef S_IRWXO
# define	S_IRWXO		(S_IROTH | S_IWOTH | S_IXOTH)
#endif /* S_IRWXO		*/

// Use 'null' instead of /dev/null in win32.
#ifdef HAVE_WINSOCK2_H
# define DEV_NULL "nul"
#else
# define DEV_NULL "/dev/null"
#endif // HAVE_WINSOCK2_H

// Use 'con' instead of '/dev/stdout' in win32.
#ifdef HAVE_WINSOCK2_H
# define DEV_STDOUT "con"
#else
# define DEV_STDOUT "/dev/stdout"
#endif // HAVE_WINSOCK2_H

#ifdef __MINGW32__
# define lseek(a, b, c) _lseeki64((a), (b), (c))
# define a2mkdir(path, openMode) mkdir(path)
#else
# define a2mkdir(path, openMode) mkdir(path, openMode)
#endif // __MINGW32__

#if defined HAVE_POSIX_MEMALIGN && O_DIRECT
# define ENABLE_DIRECT_IO 1
#endif // HAVE_POSIX_MEMALIGN && O_DIRECT

#define OPEN_MODE S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH
#define DIR_OPEN_MODE S_IRWXU|S_IRWXG|S_IRWXO

#endif // _D_A2IO_H_
