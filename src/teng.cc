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
 * $Id: teng.cc,v 1.4 2005-06-22 07:16:07 romanmarek Exp $
 *
 * DESCRIPTION
 * Teng engine -- implementation.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-17  (vasek)
 *             Created.
 * 2005-06-21  (roman)
 *             Win32 support.
 */

#include <unistd.h>

#include <stdexcept>
#include <memory>

#include "tengutil.h"
#include "tengplatform.h"
#include "tenglogging.h"
#include "tengstructs.h"
#include "tengprocessor.h"
#include "tengtemplate.h"
#include "teng.h"

extern "C" int teng_library_present() {return 0;}

namespace Teng {
namespace {

std::string prependBeforeExt(const std::string &str, const std::string &prep) {
    // no prep or no str -> return str
    if (prep.empty() || str.empty()) return str;

    // find the last dot and the last slash
    std::string::size_type dot = str.rfind('.');
    std::string::size_type slash = str.rfind('/');
#ifdef WIN32
    std::string::size_type bslash = str.rfind('\\');
    if (bslash > slash)
        slash = bslash;
#endif //WIN32

    // if last slash exists and slash after dot or no dot
    // append prep at the end
    if (((slash != std::string::npos) && (slash > dot)) ||
        (dot == std::string::npos)) {
        return str + '.' + prep;

    } else {
        // else prepend prep before the last dot
        return str.substr(0, dot) + '.' + prep + str.substr(dot);
    }
}

int gen_page(
    std::unique_ptr<Template_t> templ,
    const std::string &contentType,
    const std::string &encoding,
    const FragmentValue_t &data,
    Writer_t &writer,
    Error_t &err
) {
    // propage error log
    writer.setError(&err);

    // if program is valid (not empty) execute it
    if (!templ->program->empty()) {
        Processor_t(
            err,
            *templ->program,
            *templ->langDictionary,
            *templ->paramDictionary,
            encoding,
            contentType
        ).run(data, writer);
    }

    // flush writer to output
    writer.flush();

    // return error level from error log
    return err.max_level;
}

std::string normalize_root(std::string root) {
    // if not absolute path, prepend current working directory
    if (root.empty() || !ISROOT(root)) {
        char cwd[2048];
        if (!getcwd(cwd, sizeof(cwd)))
            throw std::runtime_error("cannot get cwd");
        root = std::string(cwd) + '/' + root;
    }
    normalizeFilename(root);
    return root;
}

} // namespace

Teng_t::Teng_t(const std::string &root, const Teng_t::Settings_t &settings)
    : root(normalize_root(root)),
      templateCache(std::make_unique<TemplateCache_t>(
          this->root, settings.programCacheSize, settings.dictCacheSize
      ))
{}

Teng_t::~Teng_t() = default;

int Teng_t::generatePage(
    const std::string &templateFilename,
    const std::string &skin,
    const std::string &dict,
    const std::string &lang,
    const std::string &param,
    const std::string &contentType,
    const std::string &encoding,
    const Fragment_t &data,
    Writer_t &writer,
    Error_t &err
) const {
    std::string encoding_lowerized = tolower(encoding);
    std::unique_ptr<Template_t> templ(templateCache->createTemplate(
        err,
        prependBeforeExt(templateFilename, skin),
        prependBeforeExt(dict, lang),
        param,
        encoding_lowerized,
        contentType,
        TemplateCache_t::SRC_FILE
    ));
    return gen_page(
        std::move(templ),
        contentType,
        encoding,
        FragmentValue_t(&data),
        writer,
        err
    );
}

int Teng_t::generatePage(
    const std::string &templateString,
    const std::string &dict,
    const std::string &lang,
    const std::string &param,
    const std::string &contentType,
    const std::string &encoding,
    const Fragment_t &data,
    Writer_t &writer,
    Error_t &err
) const {
    std::string encoding_lowerized = tolower(encoding);
    std::unique_ptr<Template_t> templ(templateCache->createTemplate(
        err,
        templateString,
        prependBeforeExt(dict, lang),
        param,
        encoding_lowerized,
        contentType,
        TemplateCache_t::SRC_STRING
    ));
    return gen_page(
        std::move(templ),
        contentType,
        encoding,
        FragmentValue_t(&data),
        writer,
        err
    );
}

int Teng_t::dictionaryLookup(
    const std::string &config,
    const std::string &dict,
    const std::string &lang,
    const std::string &key,
    std::string &value
) const {
    // find value for key
    Error_t err;
    auto path = prependBeforeExt(dict, lang);
    auto *dictionary = templateCache->createDictionary(err, config, path);
    auto *foundValue = dictionary->lookup(key);
    if (!foundValue) {
        // not fount => error
        value.erase();
        return -1;
    }
    // found => assign
    value = *foundValue;
    return 0;
}

std::vector<std::pair<std::string, std::string>>
Teng_t::listSupportedContentTypes() {
    return ContentType_t::listSupported();
}

} // namespace Teng

