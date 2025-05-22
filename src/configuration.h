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
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2004-09-18  (vasek)
 *             Created.
 */

#ifndef TENGCONFIGURATION_H
#define TENGCONFIGURATION_H

#include <cstdint>
#include <iosfwd>
#include <string>

#include "dictionary.h"

namespace Teng {

/** The result type of isEnabled query.
 */
enum class teng_feature {unknown = 0, enabled = 1, disabled = 2};

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
    Configuration_t(Error_t &err, std::shared_ptr<const FilesystemInterface_t> filesystem);

    // @{ shortcuts to query configuration
    bool isDebugEnabled() const {return debug;}
    bool isErrorFragmentEnabled() const {return errorFragment;}
    bool isLogToOutputEnabled() const {return logToOutput;}
    bool isBytecodeEnabled() const {return bytecode;}
    bool isWatchFilesEnabled() const {return watchFiles;}
    uint32_t getMaxIncludeDepth() const {return maxIncludeDepth;}
    uint16_t getMaxDebugValLength() const {return maxDebugValLength;}
    bool isFormatEnabled() const {return format;}
    bool isAlwaysEscapeEnabled() const {return alwaysEscape;}
    bool isPrintEscapeEnabled() const {return printEscape;}
    bool isShortTagEnabled() const {return shortTag;}
    // @}

    /** Sets enabled to true if feature is enabled.
     */
    teng_feature isEnabled(const string_view_t &name) const;

    /** Dumps configuration to stream.
     */
    friend std::ostream &operator<<(std::ostream &o, const Configuration_t &c);

protected:
    /** Called wheb new directive parsed.
     */
    error_code
    new_directive(
        const char *name_ptr, std::size_t name_len,
        const char *value_ptr, std::size_t value_len
    ) override;

    bool debug;         //!< the <?teng debug?> enabled (false)
    bool errorFragment; //!< the <?teng frag ._error?> enabled (false)
    bool logToOutput;   //!< log error goes to output (false)
    bool bytecode;      //!< the <?teng bytecode?> enabled (false)
    bool watchFiles;    //!< cached templates are checked for change (true)
    bool alwaysEscape;  //!< always escape regardless on next instr (true)
    bool shortTag;      //!< short tags <? ?> enabled (false)
    bool format;        //!< the <?tenf formag ...?> enabled (true)
    uint32_t maxIncludeDepth;   //!< maximal template include depth
    uint16_t maxDebugValLength; //!< maximal length of variable value length
    bool printEscape;  //!< use escaping only if values are printed
};

} // namespace Teng

#endif // TENGCONFIGURATION_H
