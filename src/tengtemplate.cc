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
 * $Id: tengtemplate.cc,v 1.1 2004-07-28 11:36:55 solamyl Exp $
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
#include "tengdatadefinition.h"

using namespace std;

using namespace Teng;

Template_t::~Template_t() {
    if (owner) {
        owner->release(program);
        owner->release(langDictionary);
        owner->release(paramDictionary);
        owner->release(dataDefinition);
    }
}

TemplateCache_t::TemplateCache_t(const string &root,
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
                 : ProgramCache_t::DEFAULT_MAXIMAL_SIZE))
{}

TemplateCache_t::~TemplateCache_t() {
    delete programCache;
    delete dictCache;
}

Template_t*
TemplateCache_t::createTemplate(const string &templateSource,
                                const string &langFilename,
                                const string &paramFilename,
                                const string &dataDefFilename,
                                bool validate,
                                SourceType_t sourceType)
{
    // create key from source file names
    vector<string> key;
    if (sourceType == SRC_STRING)
        tengCreateStringKey(templateSource, key);
    else tengCreateKey(root, templateSource, key);
    tengCreateKey(root, langFilename, key);
    tengCreateKey(root, paramFilename, key);
    
    // indicates that some dictionary has been reloaded
    bool dictionariesReloaded = false;
    
    // create lang dictionary
    const Dictionary_t *langDictionary = dictFromFile<Dictionary_t>
        (langFilename, &dictionariesReloaded);
    if (!langDictionary) return 0;
    
    // create config dictionary
    const Dictionary_t *paramDictionary = dictFromFile<Dictionary_t>
        (paramFilename, &dictionariesReloaded);
    if (!paramDictionary) return 0;
    
    // create data definition dictionary
    const Dictionary_t *dataDefinition = dictFromFile<DataDefinition_t>
        (dataDefFilename, &dictionariesReloaded);
    if (!dataDefinition) return 0;
    
    // cached program
    const Program_t *cachedProgram = 0;
    // search for program in cache
    programCache->find(key, cachedProgram);
    // if priogram not found, sources changed or dictionaried reloaded
    // create new program
    if (dictionariesReloaded || (!cachedProgram || cachedProgram->check())) {
        // create new program
        Program_t *program = (sourceType == SRC_STRING)
            ?
            ParserContext_t(langDictionary,
                            paramDictionary,
                            validate ? dataDefinition : 0, root)
            .createProgramFromString(templateSource)
            :
            ParserContext_t(langDictionary,
                            paramDictionary,
                            validate ? dataDefinition : 0, root)
            .createProgramFromFile(templateSource);
        
        // add program into cache
        cachedProgram = programCache->add(key, program);
    }
    if (!cachedProgram) return 0;
    
    // create template with cached sources
    Template_t *templ =
        new Template_t(cachedProgram, langDictionary,
                       paramDictionary, dataDefinition, this);
    // return template
    return templ;
}

int TemplateCache_t::release(const Program_t *program) {
    return programCache->release(program);
}

int TemplateCache_t::release(const Dictionary_t *dictionary) {
    return dictCache->release(dictionary);
}

