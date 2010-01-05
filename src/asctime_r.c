/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2008 Tatsuhiro Tsujikawa
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#include <time.h>
#include <stdlib.h>

#ifdef __MINGW32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // __MINGW32__

#include "asctime_r.h"

#ifdef __MINGW32__

static CRITICAL_SECTION asctime_r_cs;

static void asctime_r_atexit()
{
	DeleteCriticalSection(&asctime_r_cs);
}

char * asctime_r (const struct tm *tyme, char *buf)
{
	static char *p;
	static int initialized = 0;

	if (!initialized) {
		++initialized;
		InitializeCriticalSection(&asctime_r_cs);
		atexit(asctime_r_atexit);
	}

	EnterCriticalSection(&asctime_r_cs);
	p = asctime(tyme);
	memcpy(buf, p, 26);
	LeaveCriticalSection(&asctime_r_cs);
	return buf;
};

#endif // __MINGW32__
