/*
 * Teng -- a general purpose templating engine.
 * Copyright (C) 2004  Seznam.cz, a.s.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Seznam.cz, a.s.
 * Naskove 1, Praha 5, 15000, Czech Republic
 * http://www.seznam.cz, mailto:teng@firma.seznam.cz
 *
 *
 * $Id: time.cpp,v 1.1 2005-06-22 07:16:15 romanmarek Exp $
 *
 * DESCRIPTION
 * Substitution for unix time functions.
 *
 * AUTHORS
 * Roman Marek <roman.marek@firma.seznam.cz>
 *
 * HISTORY
 * 2005-06-16  (roman)
 *             Created.
 */


#include <stdafx.h>
#include <time.h>
#include "sys/time.h"

int gettimeofday(timeval *tv, struct timezone *tz)
{
	if (tz)
	{
		tz->tz_minuteswest = 0;
		tz->tz_dsttime = _timezone;
	}

	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	time(&tv->tv_sec);
	tv->tv_usec = (ft.dwLowDateTime / 10) % 1000000;

	return 0;
}

struct tm *localtime_r(const time_t *timep, struct tm *result)
{
	struct tm * tmRet = localtime(timep);

	if (result)
		try{
			memcpy (result, tmRet, sizeof(tm));
		}catch(...){
			return 0;
		}

	return tmRet;
}