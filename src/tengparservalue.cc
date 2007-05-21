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
 * $Id: tengparservalue.cc,v 1.3 2007-05-21 15:43:28 vasek Exp $
 *
 * DESCRIPTION
 * Teng parser value data type.
 *
 * AUTHORS
 * Jan Nemec <jan.nemec@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-22  (jan)
 *             Created.
 * 2005-06-21  (roman)
 *             Win32 support.
*/

#include <stdio.h>
#include <stdlib.h>

#include <sstream>

#include "tengparservalue.h"
#include "tengplatform.h"

using namespace std;

using namespace Teng;

void ParserValue_t::setString(const string &val) {
    const char *str;
    char *parseStr;
    
    stringValue = val;
    if (val.size()) {
        str = val.c_str();
        integerValue = strtol(str, &parseStr, 10);
        if (!*parseStr) {
            realValue = integerValue;
            type = TYPE_INT;
            return;
        }
        realValue = strtod(str, &parseStr);
        if (!*parseStr) {
            integerValue = (int_t)realValue;
            type = TYPE_REAL;
            return;
        }
    }
    type = TYPE_STRING;
    realValue = 0.0;
    integerValue = 0;
}

void ParserValue_t::setInteger(int_t val) {
    std::ostringstream os;
    os << val;
    stringValue = os.str();
    integerValue = val;
    realValue = val;
    type = TYPE_INT;
}

void ParserValue_t::setReal(double val) {
    char str[64];
    
    int l = snprintf(str, sizeof(str), "%f", val);
    if (!strstr(str, ".")) strcpy(str + l,".0");
    else {
        l--;
        while (1) {
            if (str[l-1] == '.' || str[l] != '0')
                break;
            str[l] = 0;
            l--;
        }
    }
    stringValue = str;
    integerValue = (int_t)val;
    realValue = val;
    type = TYPE_REAL;
}

void ParserValue_t::setReal(double val, int prec) {
    char str[64];
    
    snprintf(str, sizeof(str), "%.*f", prec, val);
    stringValue = str;
    integerValue = (int_t)val;
    realValue = val;
    type = TYPE_REAL;
}

ParserValue_t ParserValue_t::validate() const {
    ParserValue_t r(*this);
    
    r.validateThis();
    return r;
}

void ParserValue_t::validateThis() {
    if (type == TYPE_STRING) {
        const char *str;
        char *parseStr;
        
        if (stringValue.size()) {
            str = stringValue.c_str();
            integerValue = strtol(str, &parseStr, 10);
            if (!*parseStr) {
                realValue = integerValue;
                type = TYPE_INT;
                return;
            }
            realValue = strtod(str, &parseStr);
            if (!*parseStr) {
                integerValue = (int_t)realValue;
                type = TYPE_REAL;
                return;
            }
            type = TYPE_STRING;
            realValue = 0.0;
            integerValue = 0;
        } else {
            // empty string
            setInteger(0);
            return;
        }
    }
}

