/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2007 Tatsuhiro Tsujikawa
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

#include "localtime_r.h"

#ifdef __MINGW32__

static CRITICAL_SECTION localtime_r_cs;

static void localtime_r_atexit()
{
	DeleteCriticalSection(&localtime_r_cs);
}

struct tm * localtime_r(const time_t *clock, struct tm *result)
{
	static struct tm *local_tm;
	static int initialized = 0;

	if (!initialized) {
		++initialized;
		InitializeCriticalSection(&localtime_r_cs);
		atexit(localtime_r_atexit);
	}

	EnterCriticalSection(&localtime_r_cs);
	local_tm = localtime(clock);
	memcpy(result, local_tm, sizeof(struct tm));
	LeaveCriticalSection(&localtime_r_cs);
	return result;
};

#endif // __MINGW32__
