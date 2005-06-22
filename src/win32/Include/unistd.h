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
 * $Id: unistd.h,v 1.1 2005-06-22 07:16:15 romanmarek Exp $
 *
 * DESCRIPTION
 * Unix std fnc on WIN32 platform.
 *
 * AUTHORS
 * Roman Marek <roman.marek@firma.seznam.cz>
 *
 * HISTORY
 * 2005-06-16  (roman)
 *             Created.
 */

#ifndef TENG_UNISTD_H
#define TENG_UNISTD_H

#include <stdio.h>
#include <direct.h>
#include <process.h>

#define getcwd _getcwd
#define getpid _getpid

#define S_ISDIR(x) ((x) & _S_IFDIR)

#endif //TENG_UNISTD_H