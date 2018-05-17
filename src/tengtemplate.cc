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
 * $Id: tengtemplate.cc,v 1.4 2005-04-11 17:14:02 solamyl Exp $
 *
 * DESCRIPTION
 * Teng template and cache of templates -- implementation.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-25  (vasek)
 *             Created.
 */

#include "tengtemplate.h"

namespace Teng {

Template_t::~Template_t() {
    if (owner) {
        owner->release(program);
        owner->release(langDictionary);
        owner->release(paramDictionary);
    }
}

TemplateCache_t::TemplateCache_t(const std::string &root,
                                 const FilesystemInterface_t *filesystem,
                                 unsigned int programCacheSize,
                                 unsigned int dictCacheSize)
    : root(root),
      filesystem(filesystem),
      programCache(new ProgramCache_t
                   (programCacheSize
                    ? programCacheSize
                    : ProgramCache_t::DEFAULT_MAXIMAL_SIZE)),
      dictCache(new DictionaryCache_t
                (dictCacheSize
                 ? dictCacheSize
                 : DictionaryCache_t::DEFAULT_MAXIMAL_SIZE)),
      configCache(new ConfigurationCache_t
                (dictCacheSize
                 ? dictCacheSize
                 : ConfigurationCache_t::DEFAULT_MAXIMAL_SIZE))
{}

TemplateCache_t::~TemplateCache_t() {
    delete programCache;
    delete dictCache;
    delete configCache;
}

Template_t*
TemplateCache_t::createTemplate(const std::string &templateSource,
                                const std::string &langFilename,
                                const std::string &configFilename,
                                SourceType_t sourceType)
{
    unsigned long int configSerial;

    // get configuration and dictionary from cache
    ConfigAndDict_t configAndDict
        = getConfigAndDict(configFilename, langFilename,
                           &configSerial);

    // create key from source file names
    std::vector<std::string> key;
    if (sourceType == SRC_STRING) tengCreateStringKey(templateSource, key);
    else tengCreateKey(root, templateSource, key);
    tengCreateKey(root, langFilename, key);
    tengCreateKey(root, configFilename, key);

    // cached program
    unsigned long int programSerial;
    unsigned long int programDependSerial;
    const Program_t *cachedProgram
        = programCache->find(key, programDependSerial, &programSerial);

    // determine whether we have to reload program
    bool reload = (!cachedProgram
                   || (configSerial != programDependSerial)
                   || (configAndDict.first->isWatchFilesEnabled()
                       && cachedProgram->check()));

    if (reload) {
        // create new program
        Program_t *program = (sourceType == SRC_STRING)
            ?
            ParserContext_t(configAndDict.second, configAndDict.first, filesystem, root)
            .createProgramFromString(templateSource)
            :
            ParserContext_t(configAndDict.second, configAndDict.first, filesystem, root)
            .createProgramFromFile(templateSource);

        // add program into cache
        cachedProgram = programCache->add(key, program, configSerial);
    }

    // create template with cached sources
    // cannot return value directly, because of g++ 2.95 warnings
    Template_t *retval = new Template_t(cachedProgram,
            configAndDict.second, configAndDict.first, this);
    return retval;
}

TemplateCache_t::ConfigAndDict_t
TemplateCache_t::getConfigAndDict(const std::string &configFilename,
                                  const std::string &dictFilename,
                                  unsigned long int *serial)
{
    // find or create configuration
    std::vector<std::string> key;
    tengCreateKey(root, configFilename, key);

    unsigned long int configSerial = 0;
    unsigned long int configDependSerial = 0;
    const Configuration_t *cachedConfig
        = configCache->find(key, configDependSerial, &configSerial);
    if (!cachedConfig
        || (cachedConfig->isWatchFilesEnabled() && cachedConfig->check())) {
        // not found or changed -> create new configionary
        Configuration_t *config = new Configuration_t(root);
        // parse file
        if (!configFilename.empty()) config->parse(filesystem, configFilename);
        // add configionary to cache and return it
        cachedConfig = configCache->add(key, config, 0, &configSerial);
    }

    // reuse key for dictionary
    tengCreateKey(root, dictFilename, key);

    // find or create dictionary
    unsigned long int dictSerial = 0;
    unsigned long int dictDependSerial = 0;
    const Dictionary_t *cachedDict = dictCache->find(key, dictDependSerial,
                                                     &dictSerial);

    if (!cachedDict || (dictDependSerial != configSerial)
        || (cachedConfig->isWatchFilesEnabled() && cachedDict->check())) {
        // not found or changed -> create new dictionary
        Dictionary_t *dict = new Dictionary_t(root);
        // parse file
        if (!dictFilename.empty()) dict->parse(filesystem, dictFilename);
        // add dictionary to cache and return it
        // (dict depends on config serial number)
        cachedDict = dictCache->add(key, dict, configSerial, &dictSerial);
    }

    // set config-dict serial number (it's dict's serial number)
    if (serial) *serial = dictSerial;

    // return data
    return ConfigAndDict_t(cachedConfig, cachedDict);
}

} // namespace Teng

