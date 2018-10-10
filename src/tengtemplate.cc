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
                                 unsigned int programCacheSize,
                                 unsigned int dictCacheSize)
    : root(root),
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

Template_t *
TemplateCache_t::createTemplate(Error_t &err,
                                const std::string &source,
                                const std::string &langFilename,
                                const std::string &configFilename,
                                const std::string &encoding,
                                const std::string &ctype,
                                SourceType_t sourceType)
{
    unsigned long int configSerial;

    // get configuration and dictionary from cache
    const Dictionary_t *dict = nullptr;
    const Configuration_t *params = nullptr;
    std::tie(params, dict)
        = getConfigAndDict(err, configFilename, langFilename, &configSerial);

    // create key from source file names
    std::vector<std::string> key;
    if (sourceType == SRC_STRING)
        key.push_back(createCacheKeyForString(source));
    else key.push_back(createCacheKeyForFilename(root, source));
    key.push_back(createCacheKeyForFilename(root, langFilename));
    key.push_back(createCacheKeyForFilename(root, configFilename));

    // cached program
    unsigned long int programSerial;
    unsigned long int dataSerial;
    const Program_t *cached
        = programCache->find(key, dataSerial, &programSerial);

    // determine whether we have to reload program
    bool reload = !cached
        || (configSerial != dataSerial)
        || (params->isWatchFilesEnabled() && cached->isChanged());

    if (reload) {
        // create new program
        auto program = (sourceType == SRC_STRING)
            ? compile_string(err, dict, params, root, {source}, encoding, ctype)
            : compile_file(err, dict, params, root, source, encoding, ctype);

        // add program into cache
        // TODO(burlog): cached = programCache->add(key, std::move(program), configSerial);
        cached = programCache->add(key, program.release(), configSerial);
    }

    // create template with cached sources
    return new Template_t(cached, dict, params, this);
}

TemplateCache_t::ConfigAndDict_t
TemplateCache_t::getConfigAndDict(
    Error_t &err,
    const std::string &configFilename,
    const std::string &dictFilename,
    unsigned long int *serial)
{
    // find or create configuration
    std::vector<std::string> key;

    unsigned long int configSerial = 0;
    unsigned long int configDependSerial = 0;
    auto *cachedCfg = configCache->find(key, configDependSerial, &configSerial);
    bool reloadCfg = !cachedCfg
        || (cachedCfg->isWatchFilesEnabled() && cachedCfg->isChanged());

    // find or create dictionary
    unsigned long int dictSerial = 0;
    unsigned long int dictDependSerial = 0;
    auto *cachedDict = dictCache->find(key, dictDependSerial, &dictSerial);
    bool reloadDict = !cachedDict
        || (dictDependSerial != configSerial)
        || (cachedCfg->isWatchFilesEnabled() && cachedDict->isChanged());

    // reuse key for config
    key.push_back(createCacheKeyForFilename(root, configFilename));

    // reload params if needed
    if (reloadCfg) {
        // not found or changed -> create new configionary
        Configuration_t *config = new Configuration_t(err, root);
        // parse file
        if (!configFilename.empty()) config->parse(configFilename);
        // add configionary to cache and return it
        cachedCfg = configCache->add(key, config, 0, &configSerial);
    }

    // reuse key for dictionary
    key.push_back(createCacheKeyForFilename(root, dictFilename));

    // reload lang dict if needed
    if (reloadDict) {
        // not found or changed -> create new dictionary
        Dictionary_t *dict = new Dictionary_t(err, root);
        // parse file
        if (!dictFilename.empty()) dict->parse(dictFilename);
        // add dictionary to cache and return it
        // (dict depends on config serial number)
        cachedDict = dictCache->add(key, dict, configSerial, &dictSerial);
    }

    // set config-dict serial number (it's dict's serial number)
    if (serial) *serial = dictSerial;

    // return data
    return ConfigAndDict_t(cachedCfg, cachedDict);
}

} // namespace Teng

