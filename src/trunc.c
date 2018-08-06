/*
 * Teng -- a general purpose templating engine.
 * Copyright (C) 2005  Seznam.cz, a.s.
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
 * $Id: trunc.c,v 1.1 2007-11-26 17:33:24 vasek Exp $
 *
 * DESCRIPTION
 * Emulation of trunc() math function.
 *
 * AUTHORS
 * Stepan Skrob <stepan@firma.seznam.cz>
 *
 * HISTORY
 * 2005-04-11  (stepan)
 *             Created.
 */

#include <math.h>

/**
  * This function round x to the nearest integer not larger in absolute value.
  * @return The rounded integer value.
  */
double trunc(double x) {return x >= 0.0? floor(x): ceil(x);}

