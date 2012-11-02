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
 * $Id: tengconfiguration.h,v 1.4 2010-06-11 07:46:26 burlog Exp $
 *
 * DESCRIPTION
 * Teng configuration dictionary.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2004-09-18  (vasek)
 *             Created.
 */


#ifndef TENGCONFIGURATION_H
#define TENGCONFIGURATION_H

#include <iosfwd>

#include "tengdictionary.h"

using namespace std;

namespace Teng {

/**
 * @short Configuration -- language independed dictionary and
 *        configuration placeholder.
 */
class Configuration_t : public Dictionary_t {
public:
    /**
     * @short Creates configuration object.
     *
     * @param root path of root for locating files
     */
    Configuration_t(const string &root = string());

    /**
     * @short Destroy dictionary object.
     */
    virtual ~Configuration_t();

    /**
     * @short Parses and processes processing directive.
     *
     * @param directive whole directive string
     * @param param parameter to directive
     * @param pos position in current file
     * @return 0 OK !0 error
     */
    virtual int processDirective(const string &directive,
                                 const string &param,
                                 Error_t::Position_t &pos);

    inline bool isDebugEnabled() const {
        return debug;
    }

    inline bool isErrorFragmentEnabled() const {
        return errorFragment;
    }

    inline bool isLogToOutputEnabled() const {
        return logToOutput;
    }

    inline bool isBytecodeEnabled() const {
        return bytecode;
    }

    inline bool isWatchFilesEnabled() const {
        return watchFiles;
    }

    inline unsigned int getMaxIncludeDepth() const {
        return maxIncludeDepth;
    }

    inline unsigned int getMaxDebugValLength() const {
        return maxDebugValLength;
    }

    inline bool isFormatEnabled() const {
        return format;
    }

    inline bool isAlwaysEscapeEnabled() const {
        return alwaysEscape;
    }
    
    inline bool isShortTagEnabled() const {
        return shortTag;
    }

    int isEnabled(const string &feature, bool &enabled) const;

    friend std::ostream& operator<<(std::ostream &o, const Configuration_t &c);

private:
    bool debug;           //!< <?teng debug?> works. (false)
    bool errorFragment;   //!< <?teng frag ._error?> works. (false)
    bool logToOutput;     //!< Log error goes to ouput. (false)
    bool bytecode;        //!< <?teng bytecode?> works. (false)
    bool watchFiles;      //!< Cached templates are checked for change. (true)
    bool alwaysEscape;    //!< Escape always (true)
    bool shortTag;         //!< Short tags <? ?> enabled (false)

    unsigned int maxIncludeDepth; //!< Maximal template include depth.

    bool format;          //!< enabled <?tenf formag ...?> (true)
    unsigned short int maxDebugValLength; //!< Maximal length of variable value length
};

} // namespace Teng

#endif // TENGCONFIGURATION_H
