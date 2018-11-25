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

#include "template.h"

namespace Teng {

TemplateCache_t::TemplateCache_t(
    const std::string &fs_root,
    unsigned int programCacheSize,
    unsigned int dictCacheSize
): fs_root(fs_root), programCache(programCacheSize),
   dictCache(dictCacheSize), paramsCache(dictCacheSize)
{}

Template_t
TemplateCache_t::createTemplate(
    Error_t &err,
    const std::string &source,
    const std::string &langFilename,
    const std::string &configFilename,
    const std::string &encoding,
    const std::string &ctype,
    SourceType_t sourceType
) {
    // get configuration and dictionary from cache
    uint64_t configSerial;
    std::shared_ptr<Dictionary_t> dict;
    std::shared_ptr<Configuration_t> params;
    std::tie(params, dict, configSerial)
        = getConfigAndDict(err, configFilename, langFilename);

    // create key from source file names
    std::vector<std::string> key;
    if (sourceType == SRC_STRING)
        key.push_back(createCacheKeyForString(source));
    else key.push_back(createCacheKeyForFilename(fs_root, source));
    key.push_back(createCacheKeyForFilename(fs_root, langFilename));
    key.push_back(createCacheKeyForFilename(fs_root, configFilename));

    // cached program
    uint64_t dependSerial;
    std::shared_ptr<Program_t> program;
    std::tie(program, dependSerial, std::ignore) = programCache.find(key);

    // determine whether we have to reload program
    bool reload = !program
        || (configSerial != dependSerial)
        || (params->isWatchFilesEnabled() && program->isChanged());

    // create new program if reload requested
    if (reload) {
        auto *d = &*dict;
        auto *p = &*params;
        program = (sourceType == SRC_STRING)
            ? compile_string(err, d, p, fs_root, {source}, encoding, ctype)
            : compile_file(err, d, p, fs_root, source, encoding, ctype);
        programCache.add(key, program, configSerial);
    }

    // create template with cached sources
    return {std::move(program), std::move(dict), std::move(params)};
}

std::tuple<
    std::shared_ptr<Configuration_t>,
    std::shared_ptr<Dictionary_t>,
    uint64_t
> TemplateCache_t::getConfigAndDict(
    Error_t &err,
    const std::string &configFilename,
    const std::string &dictFilename,
    unsigned long int *serial)
{
    // key for config
    std::vector<std::string> key;
    key.push_back(createCacheKeyForFilename(fs_root, configFilename));

    // find or create configuration
    uint64_t configSerial;
    std::shared_ptr<Configuration_t> params;
    std::tie(params, std::ignore, configSerial) = paramsCache.find(key);

    // determine whether we have to reload params
    bool reload_params = !params
        || (params->isWatchFilesEnabled() && params->isChanged());

    // reload params if needed
    if (reload_params) {
        params = std::make_shared<Configuration_t>(err, fs_root);
        if (!configFilename.empty()) params->parse(configFilename);
        configSerial = paramsCache.add(key, params);
    }

    // reuse key for dictionary
    key.push_back(createCacheKeyForFilename(fs_root, dictFilename));

    // find or create dictionary
    uint64_t dictSerial = 0;
    uint64_t dependSerial = 0;
    std::shared_ptr<Dictionary_t> dict;
    std::tie(dict, dependSerial, dictSerial) = dictCache.find(key);

    // determine whether we have to reload dict
    bool reload_dict = !dict
        || (configSerial != dependSerial)
        || (params->isWatchFilesEnabled() && dict->isChanged());

    // reload lang dict if needed
    if (reload_dict) {
        dict = std::make_shared<Dictionary_t>(err, fs_root);
        if (!dictFilename.empty()) dict->parse(dictFilename);
        dictCache.add(key, dict, configSerial);
    }

    // return data
    return {std::move(params), std::move(dict), configSerial};
}

} // namespace Teng

