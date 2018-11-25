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
 * $Id: tengtemplate.h,v 1.2 2004-12-30 12:42:02 vasek Exp $
 *
 * DESCRIPTION
 * Teng template and cache of templates.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-25  (vasek)
 *             Created.
 */

#ifndef TENGTEMPLATE_H
#define TENGTEMPLATE_H

#include <tuple>
#include <memory>
#include <utility>
#include <string>

#include "cache.h"
#include "dictionary.h"
#include "program.h"
#include "parsercontext.h"
#include "configuration.h"

namespace Teng {

/** @short Teng template.
 *
 *  Made up from program, laguage, config dictionary and data
 *  definition. It's able to check change of source files.
 */
class Template_t {
public:
    std::shared_ptr<const Program_t> program;      //!< bytecode of the template
    std::shared_ptr<const Dictionary_t> dict;      //!< language dictionary
    std::shared_ptr<const Configuration_t> params; //!< config dictionary
};

/** @short Cache of templates.
 */
class TemplateCache_t {
public:
    /** @short Cache of dictionaries.
     */
    using DictionaryCache_t = Cache_t<Dictionary_t>;

    /** @short Cache of configurations.
     */
    using ConfigurationCache_t = Cache_t<Configuration_t>;

    /** @short Cache of dictionaries.
     */
    using ProgramCache_t = Cache_t<Program_t>;

    /** @short Create new cache.
     *
     *  @param fs_root root dir for relative paths
     *  @param programCacheSize maximal number of programs in the cache
     *  @param dictCacheSizemaximal number of dictionaries in the cache
     */
    TemplateCache_t(
        const std::string &fs_root,
        unsigned int programCacheSize = 0,
        unsigned int dictCacheSize = 0
    );

    /** @short Type of source.
     */
    enum SourceType_t {
        SRC_FILE,   /**< source is filename */
        SRC_STRING, /**< source is template */
    };

    /** @short Create template from given data.
     *  @param templateSource source of template
     *  @param langFilename file with language dictionary
     *  @param paramFilename file with config
     *  @param sourceType type of template source
     *  @return created template
     */
    Template_t
    createTemplate(
        Error_t &err,
        const std::string &source,
        const std::string &langFilename,
        const std::string &paramFilename,
        const std::string &encoding,
        const std::string &ctype,
        SourceType_t sourceType
    );

    /** @short Create dictionary from given files.
     *
     *  @param configFilename file with configuration
     *  @param dictFilename file with dictionary
     *  @return created dictionary
     */
    std::shared_ptr<const Dictionary_t>
    createDictionary(
        Error_t &err,
        const std::string &configFilename,
        const std::string &dictFilename
    ) {
        return std::get<1>(getConfigAndDict(err, configFilename, dictFilename));
    }

private:
    // don't copy
    TemplateCache_t(const TemplateCache_t &) = delete;
    TemplateCache_t &operator=(const TemplateCache_t &) = delete;

    /** @short Get configuration and dictionary from given files.
     *
     *  @param configFilename file with configuration
     *  @param dictFilename file with dictionary
     *  @param serial serial number of cached data (output)
     *  @return configuration and dictionary
     */
    std::tuple<
        std::shared_ptr<Configuration_t>,
        std::shared_ptr<Dictionary_t>,
        uint64_t
    > getConfigAndDict(
        Error_t &err,
        const std::string &configFilename,
        const std::string &dictFilename,
        unsigned long int *serial = 0
    );

    std::string fs_root;              //!< root for relativa paths
    ProgramCache_t programCache;      //!< cache of compiled templates
    DictionaryCache_t dictCache;      //!< cache of parsed language dictionaries
    ConfigurationCache_t paramsCache; //!< cahce of parsed config dictionaries
};

} // namespace Teng

#endif // TENGTEMPLATE_H

