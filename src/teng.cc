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
 * $Id: teng.cc,v 1.1 2004-07-28 11:36:55 solamyl Exp $
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
 */


#include <unistd.h>

#include <stdexcept>

#include "teng.h"
#include "tengstructs.h"
#include "tengprocessor.h"
#include "tengcontenttype.h"
#include "tengtemplate.h"
#include "tengformatter.h"

using namespace std;

using namespace Teng;

extern "C" int __teng() {
    return 0;
}

static int logErrors(const ContentType_t *contentType,
                     Writer_t &writer, Error_t &err)
{
    if (!err) return 0;
    bool useLineComment = false;
    if (contentType->blockComment.first.empty()) {
        useLineComment = true;
        if (!contentType->lineComment.empty())
            writer.write(contentType->lineComment + ' ');
    } else {
        if (!contentType->blockComment.first.empty())
            writer.write(contentType->blockComment.first + ' ');
    }
    writer.write("Error log:\n");
    const vector<Error_t::Entry_t> &errorLog = err.getEntries();
    for (vector<Error_t::Entry_t>::const_iterator
             ierrorLog = errorLog.begin();
         ierrorLog != errorLog.end(); ++ierrorLog) {
        if (useLineComment && !contentType->lineComment.empty())
            writer.write(contentType->lineComment + ' ');
        writer.write(ierrorLog->getLogLine());
    }
    if (!useLineComment)
        writer.write(contentType->blockComment.second + '\n');

    return 0;
}

Teng_t::Teng_t(const string &root, const Teng_t::Settings_t &settings)
    : root(root), logMode(settings.logMode), validate(settings.validate),
      templateCache(0), err()
{
    init(settings);
}

Teng_t::Teng_t(const string &root, int logMode,
               bool validate)
    : root(root), logMode(logMode), validate(validate),
      templateCache(0), err()
{
    init(Settings_t());
}

void Teng_t::init(const Settings_t &settings) {
    // if not absolute path, prepend current working directory
    if (root.empty() || root[0] != '/') {
        char cwd[2048];
        if (!getcwd(cwd, sizeof(cwd))) {
            Error_t::Position_t pos;
            err.logSyscallError(Error_t::LL_FATAL, pos, "Cannot get cwd");
            throw runtime_error("Cannot get cwd.");
        }
        root = string(cwd) + '/' + root;
    }
    // create template cache
    templateCache = new TemplateCache_t(root, settings.programCacheSize,
                                            settings.dictCacheSize);
}

Teng_t::~Teng_t() {
    delete templateCache;
}

static string prependBeforeExt(const string &str, const string &prep) {
    // no prep or no str -> return str
    if (prep.empty()) return str;
    if (str.empty()) return str;
    // find the last dot and the last slash
    string::size_type dot = str.rfind('.');
    string::size_type slash = str.rfind('/');
    // if last slash exists and slash after dot or no dot
    // append prep at the end
    if (((slash != string::npos) && (slash > dot)) ||
        (dot == string::npos)) {
        return str + '.' + prep;
    } else {
        // else prepend prep before the last dot
        return str.substr(0, dot) + '.' + prep + str.substr(dot);
    }
}

int Teng_t::generatePage(const std::string &templateFilename,
                         const std::string &skin,
                         const std::string &dataDefinition,
                         const std::string &_dict, const std::string &lang,
                         const std::string &param, const std::string &_contentType,
                         const std::string &encoding,
                         const Fragment_t &data,
                         Writer_t &writer, Error_t &err)
{
    // find contentType desciptor for given contentType
    const ContentType_t *contentType =
        ContentType_t::findContentType(_contentType);

    // make proper filename for language dictionary
    string langDictFilename = prependBeforeExt(_dict, lang);
    
    Template_t *templ =
        templateCache->
        createTemplate(prependBeforeExt(templateFilename, skin),
                       langDictFilename, param, dataDefinition,
                       validate, TemplateCache_t::SRC_FILE);
    
    // append error logs of dicts and program
    err.append(templ->langDictionary->getErrors());
    err.append(templ->paramDictionary->getErrors());
    err.append(templ->dataDefinition->getErrors());
    err.append(templ->program->getErrors());

    // check data validity
    if (validate)
        tengCheckData(data, *templ->dataDefinition, err);
    
    // if program is valid (not empty) execute it
    if (!templ->program->empty()) {
        Formatter_t output(writer);
        
        Processor_t(templ->program, templ->langDictionary,
                    templ->paramDictionary, encoding,
                    *contentType, logMode & LM_ERROR_FRAGMENT)
            .run(&data, &output, &err);
    }

    // log error into log, if said
    if (logMode & LM_LOG_TO_OUTPUT)
        logErrors(contentType, writer, err);

    // flush writer to output
    writer.flush();

    // destroy template => it will release resources
    delete templ;

    // append writer errors
    err.append(writer.getErrors());

    // return error level from error log
    return err.getLevel();
}

int Teng_t::generatePage(const std::string &templateString,
                         const std::string &dataDefinition,
                         const std::string &_dict, const std::string &lang,
                         const std::string &param,
                         const std::string &_contentType,
                         const std::string &encoding,
                         const Fragment_t &data,
                         Writer_t &writer, Error_t &err)
{
    // find contentType desciptor for given contentType
    const ContentType_t *contentType =
        ContentType_t::findContentType(_contentType);

    // make proper filename for language dictionary
    string langDictFilename = prependBeforeExt(_dict, lang);
    
    Template_t *templ =
        templateCache->createTemplate(templateString, langDictFilename, param,
                                      dataDefinition, validate,
                                      TemplateCache_t::SRC_STRING);
    
    // append error logs of dicts and program
    err.append(templ->langDictionary->getErrors());
    err.append(templ->paramDictionary->getErrors());
    err.append(templ->dataDefinition->getErrors());
    err.append(templ->program->getErrors());
    
    // check data validity
    if (validate)
        tengCheckData(data, *templ->dataDefinition, err);

    // if program is valid (not empty) execute it
    if (!templ->program->empty()) {
        // create formatter for writer
        Formatter_t output(writer);

        // execute byte code
        Processor_t(templ->program, templ->langDictionary,
                    templ->paramDictionary, encoding,
                    *contentType, logMode & LM_ERROR_FRAGMENT)
            .run(&data, &output, &err);
    }

    // log error into log, if said
    if (logMode & LM_LOG_TO_OUTPUT)
        logErrors(contentType, writer, err);

    // flush writer to output
    writer.flush();

    // destroy template => it will release resources
    delete templ;

    // append writer errors
    err.append(writer.getErrors());

    // return error level from error log
    return err.getLevel();
}

int Teng_t::dictionaryLookup(const std::string &dict, const std::string &lang,
                             const std::string &key, std::string &value)
{
    // make proper filename for language dictionary
    string langDictFilename = prependBeforeExt(dict, lang);
    const Dictionary_t *languageDict =
        templateCache->dictFromFile<Dictionary_t>(langDictFilename);
    // stop on invalid dictionary
    if (!languageDict) return -1;
    // find value for key
    const string *foundValue = languageDict->lookup(key);
    if (!foundValue) {
        // not fount => error
        value.erase();
        return -1;
    }
    // found => assign
    value = *foundValue;
    // OK
    return 0;
}

void Teng_t::listSupportedContentTypes(std::vector<std::pair<std::string,
                                       std::string> > &supported)
{
    // retrieve supported content types
    ContentType_t::listSupported(supported);
}
