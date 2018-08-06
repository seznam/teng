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
#include <string>

#include "tengdictionary.h"

namespace Teng {

/**
 * @short Language independed dictionary and configuration placeholder.
 */
class Configuration_t: public Dictionary_t {
public:
    /**
     * @short Creates configuration object.
     *
     * @param root path of root for locating files
     */
    Configuration_t(const std::string &root = std::string());

    /**
     * @short Parses and processes processing directive.
     *
     * @param directive whole directive string
     * @param param parameter to directive
     *
     * @return 0 OK !0 error
     */
    int processDirective(string_view_t directive, string_view_t param) override;

    // @{ shortcuts to query configuration
    bool isDebugEnabled() const {return debug;}
    bool isErrorFragmentEnabled() const {return errorFragment;}
    bool isLogToOutputEnabled() const {return logToOutput;}
    bool isBytecodeEnabled() const {return bytecode;}
    bool isWatchFilesEnabled() const {return watchFiles;}
    unsigned int getMaxIncludeDepth() const {return maxIncludeDepth;}
    unsigned int getMaxDebugValLength() const {return maxDebugValLength;}
    bool isFormatEnabled() const {return format;}
    bool isAlwaysEscapeEnabled() const {return alwaysEscape;}
    bool isShortTagEnabled() const {return shortTag;}
    // @}

    /** Sets enabled to true if feature is enabled.
     */
    int isEnabled(const std::string &feature, bool &enabled) const;

    /** Dumps configuration to stream.
     */
    friend std::ostream &operator<<(std::ostream &o, const Configuration_t &c);

private:
    bool debug;           //!< <?teng debug?> works. (false)
    bool errorFragment;   //!< <?teng frag ._error?> works. (false)
    bool logToOutput;     //!< Log error goes to ouput. (false)
    bool bytecode;        //!< <?teng bytecode?> works. (false)
    bool watchFiles;      //!< Cached templates are checked for change. (true)
    bool alwaysEscape;    //!< Escape always (true)
    bool shortTag;        //!< Short tags <? ?> enabled (false)
    bool format;          //!< enabled <?tenf formag ...?> (true)
    uint32_t maxIncludeDepth;   //!< Maximal template include depth.
    uint16_t maxDebugValLength; //!< Maximal length of variable value length
};

} // namespace Teng

#endif // TENGCONFIGURATION_H

