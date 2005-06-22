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
 * $Id: tengplatform.h,v 1.1 2005-06-22 07:16:12 romanmarek Exp $
 *
 * DESCRIPTION
 * Define platform specific symbols.
 *
 * AUTHORS
 * Roman Marek <roman.marek@firma.seznam.cz>
 *
 * HISTORY
 * 2005-06-11  (roman)
 *             Created.
 */

#ifndef TENGPLATFORM_H
#define TENGPLATFORM_H

#ifdef WIN32

#ifndef snprintf

#ifndef _snprintf
#include <stdio.h>
#endif //_snprintf

#define snprintf _snprintf
#endif //snprintf

#endif //WIN32

#ifndef WIN32
#define ISROOT(path) ((path)[0] == '/')
#else
#define ISROOT(path) ((((path)[0] == '\\') && ((path)[1] == '\\')) || ((path)[1] == ':'))
#endif //WIN32

#ifdef WIN32
#define CONVERTNAMEBYPLATFORM(path)\
	string::size_type CNBP_slash = 0; \
	string::size_type CNBP_nextslash = 0; \
	static const basic_string <char>::size_type CNBP_npos = -1; \
		while((CNBP_nextslash = (path).find('\\', CNBP_slash)) != CNBP_npos) { \
			(path).replace(CNBP_nextslash, 1, string("/")); CNBP_slash = CNBP_nextslash; \
		}
#else
#define CONVERTNAMEBYPLATFORM(path)
#endif //WIN32

#endif //TENGPLATFORM_H