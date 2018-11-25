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

#include "util.h"
#include "platform.h"
#include "logging.h"
#include "processor.h"
#include "template.h"
#include "teng/structs.h"
#include "teng/teng.h"

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

std::string normalize_root(std::string fs_root) {
    // if not absolute path, prepend current working directory
    if (fs_root.empty() || !ISROOT(fs_root)) {
        char cwd[2048];
        if (!getcwd(cwd, sizeof(cwd)))
            throw std::runtime_error("cannot get cwd");
        fs_root = std::string(cwd) + '/' + fs_root;
    }
    normalizeFilename(fs_root);
    return fs_root;
}

} // namespace

Teng_t::Teng_t(const std::string &fs_root, const Teng_t::Settings_t &settings)
    : fs_root(normalize_root(fs_root)),
      templateCache(std::make_unique<TemplateCache_t>(
          this->fs_root, settings.programCacheSize, settings.dictCacheSize
      ))
{}

Teng_t::~Teng_t() = default;

int Teng_t::generatePage(
    const GenPageArgs_t &args,
    const Fragment_t &data,
    Writer_t &writer,
    Error_t &err
) const {
    std::string encoding_lowerized = tolower(args.encoding);

    // prepare template
    std::string template_arg = args.templateFilename.empty()
        ? args.templateString
        : prependBeforeExt(args.templateFilename, args.skin);

    // create template
    auto templ = templateCache->createTemplate(
        err,
        template_arg,
        prependBeforeExt(args.dictFilename, args.lang),
        args.paramsFilename,
        encoding_lowerized,
        args.contentType,
        args.templateFilename.empty()
            ? TemplateCache_t::SRC_STRING
            : TemplateCache_t::SRC_FILE
    );

    // propage error log
    writer.setError(&err);

    // if program is valid (not empty) execute it
    if (!templ.program->empty()) {
        Processor_t(
            err,
            *templ.program,
            *templ.dict,
            *templ.params,
            encoding_lowerized,
            args.contentType
        ).run(FragmentValue_t(&data), writer);
    }

    // flush writer to output
    writer.flush();

    // return error level from error log
    return err.max_level;
}

const std::string *Teng_t::dictionaryLookup(
    const std::string &config,
    const std::string &dict,
    const std::string &lang,
    const std::string &key
) const {
    Error_t err;
    auto path = prependBeforeExt(dict, lang);
    auto dictionary = templateCache->createDictionary(err, config, path);
    return dictionary->lookup(key);
}

std::vector<std::pair<std::string, std::string>>
Teng_t::listSupportedContentTypes() {
    return ContentType_t::listSupported();
}

int
Teng_t::dictionaryLookup(
    const std::string &config,
    const std::string &dict,
    const std::string &lang,
    const std::string &key,
    std::string &result
) const {
    if (auto *value = dictionaryLookup(config, dict, lang, key)) {
        result = *value;
        return 0;
    }
    result.clear();
    return -1;
}

/** Deprecated version of listSupportedContentTypes().
 */
void
Teng_t::listSupportedContentTypes(
    std::vector<std::pair<std::string, std::string>> &s
) {for (auto &e: listSupportedContentTypes()) s.push_back(e);};

} // namespace Teng

