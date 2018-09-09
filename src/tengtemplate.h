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

#include <utility>
#include <string>

#include "tengcache.h"
#include "tengdictionary.h"
#include "tengprogram.h"
#include "tengparsercontext.h"
#include "tengconfiguration.h"

namespace Teng {

/** @short Cache of dictionaries.
 */
using DictionaryCache_t = Cache_t<Dictionary_t>;

/** @short Cache of configurations.
 */
using ConfigurationCache_t = Cache_t<Configuration_t>;

/** @short Cache of dictionaries.
 */
using ProgramCache_t = Cache_t<Program_t>;

class TemplateCache_t;

/** @short Teng template.
 *  Made up from program, laguage, config dictionary and data
 *  definition. It's able to check change of source files.
 */
class Template_t {
public:
    /** @short Create new template.
     *  @param program byte compiled program
     *  @param langDictionary cache entry pointing to language
     *         dictionary
     *  @param paramDictionary cache entry pointing to config
     *         dictionary
     *  @param owner owning cache
     */
    Template_t(const Program_t *program, const Dictionary_t *langDictionary,
               const Configuration_t *paramDictionary,
               TemplateCache_t *owner)
        : program(program), langDictionary(langDictionary),
          paramDictionary(paramDictionary), owner(owner)
    {}

    /** @short Destroy template.
     */
    ~Template_t();

    /** @short Byte compiled program.
     */
    const Program_t *program;

    /** @short Language dictionary.
     */
    const Dictionary_t *langDictionary;

    /** @short Config.
     */
    const Configuration_t *paramDictionary;

private:
    Template_t(const Template_t&);
    Template_t operator=(const Template_t&);

    TemplateCache_t *owner;
};

/** @short Cache of templates.
 */
class TemplateCache_t {
public:
    /** @short Create new cache.
     *  @param root root dir for relative paths
     *  @param programCacheSize maximal number of programs in the cache
     *  @param dictCacheSizemaximal number of dictionaries in the cache
     */
    TemplateCache_t(const std::string &root, unsigned int programCacheSize = 0,
                    unsigned int dictCacheSize = 0);

    ~TemplateCache_t();

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
     *  @return created template (borrowed pointer!!!)
     */
    Template_t* createTemplate(const std::string &source,
                               const std::string &langFilename,
                               const std::string &paramFilename,
                               SourceType_t sourceType);

    /** @short Create dictionary from given files.
     *
     *  @param configFilename file with configuration
     *  @param dictFilename file with dictionary
     *  @return created dictionary (borrowed pointer!!!)
     */
    const Dictionary_t *
    createDictionary(const std::string &configFilename,
                     const std::string &dictFilename)
    {
        return getConfigAndDict(configFilename, dictFilename).second;
    }

    /** @short Release program.
     *  @param program released program
     *  @return 0 OK, !0 error
     */
    inline int release(const Program_t *program) {
        return programCache->release(program);
    }

    /** @short Release dictionary.
     *  @param dictionary released dictionary
     *  @return 0 OK, !0 error
     */
    inline int release(const Dictionary_t *dictionary) {
        return dictCache->release(dictionary);
    }

    /** @short Release configuration.
     *  @param config released configuration
     *  @return 0 OK, !0 error
     */
    inline int release(const Configuration_t *config) {
        return configCache->release(config);
    }

private:
    /** @short Copy constructor intentionally private -- copying
     *         disabled.
     */
    TemplateCache_t(const TemplateCache_t&);

    /** @short Assignment operator intentionally private -- assignment
     *         disabled.
     */
    TemplateCache_t operator=(const TemplateCache_t&);

    typedef std::pair<
        const Configuration_t *,
        const Dictionary_t *
    > ConfigAndDict_t;

    /** @short Get configuration and dictionary from given files.
     *
     *  @param configFilename file with configuration
     *  @param dictFilename file with dictionary
     *  @param serial serial number of cached data (output)
     *  @return configuration and dictionary (borrowed pointers!!!)
     */
    ConfigAndDict_t getConfigAndDict(const std::string &configFilename,
                                     const std::string &dictFilename,
                                     unsigned long int *serial = 0);

    /** @short Root for relativa paths.
     */
    std::string root;

    /** @short Cache of templates.
     */
    ProgramCache_t *programCache;

    /** @short Cache of dictionaries.
     */
    DictionaryCache_t *dictCache;

    /** @short Cache of dictionaries.
     */
    ConfigurationCache_t *configCache;
};

} // namespace Teng

#endif // TENGTEMPLATE_H

