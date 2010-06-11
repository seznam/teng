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
 * $Id: tengutil.h,v 1.3 2010-06-11 07:46:26 burlog Exp $
 *
 * DESCRIPTION
 * Teng utilities.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-18  (vasek)
 *             Created.
 */


#ifndef TENGUTIL_H
#define TENGUTIL_H

#include <string>
#include <map>

using namespace std;

#include "tengstructs.h"
#include "tengerror.h"
#include "tengdictionary.h"

namespace Teng {

/**
 * @short Normalizes filename.
 *
 * Removes multiple '/', removes '.', resolves '..'.  Path must be
 * absolute.
 *
 * @param filename normalized file (result)
 * @return 0 OK !0 error
 */
int tengNormalizeFilename(string &filename);

/** @short Compute MD5 hexdigest of data; 
 *  @param data input data
 *  @param hexdigest resulting hex digest
 *  @return 0 OK, !0 error
 */
int tengMD5Hexdigest(const string &data, string &hexdigest);

/** @short Reads all data from application and checks if it is in
 *         data definition (slow -> for debug only)
 *  @param root application data
 *  @param dataDefinition dictionary
 *  @param error log 
 */
void tengCheckData(const Fragment_t &root,
                   const Dictionary_t &dataDefinition,
                   Error_t &error);


/** @short Clip string to specified length and append "..." string
 *         to end of clipped string (utf-8 safe)
 *  @param str string to clip
 *  @param len maximal length of str after clipping
 *  @param error log
 */
void clipString(std::string &str, unsigned int len);

} // namespace Teng

#endif // TENGUTIL_H

