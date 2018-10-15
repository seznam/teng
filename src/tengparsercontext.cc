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
 * $Id: tengparsercontext.cc,v 1.6 2006-06-21 14:13:59 sten__ Exp $
 *
 * DESCRIPTION
 * Teng parser context -- implementation.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-17  (vasek)
 *             Created.
 * 2005-06-21  (roman)
 *             Win32 support.
 * 2006-06-21  (sten__)
 *             Commented out error reporting of exist function.
 */

#include <algorithm>

#include "tengparsercontext.h"
#include "tenglex1.h"
#include "tengprogram.h"
#include "tengplatform.h"


namespace Teng {

extern int tengSyntax_debug;
extern int tengSyntax_parse(void *context);
extern std::string tengSyntax_lastErrorMessage;

/** Initialize.
  * Also creates some dynamic objects (fragment stack and error-log). */
ParserContext_t::ParserContext_t(const Dictionary_t *langDictionary,
                                 const Configuration_t *paramDictionary,
                                 const FilesystemInterface_t *filesystem)
    : langDictionary(langDictionary), paramDictionary(paramDictionary),
      filesystem(filesystem), lex2(0), program(0),
      lowestValPrintAddress(0), evalProcessor(0)
{
}


/** Delete lexical analyzer objects left on the stack. */
ParserContext_t::~ParserContext_t()
{
    // delete lex1 objects
    while (!lex1.empty()) {
        delete lex1.top();
        lex1.pop();
    }
    // delete eval proc
    delete evalProcessor;
}


/** Compile file template into a program.
  * @return Pointer to program compiled within this context.
  * @param filename Template's filename. */
Program_t* ParserContext_t::createProgramFromFile(
        const std::string &filename)
{
    // empty previous stacked values
    fragContext.clear(); // delete all fragment contexts
    fragContext.reserve(20);

    while (!sourceIndex.empty())
        sourceIndex.pop(); //delete all source indexes
    while (!lex1.empty()) {
        delete lex1.top(); //delete all lex1 objects
        lex1.pop();
    }

    // create (empty) program
    program = new Program_t();

    // create old eval processor
    delete evalProcessor;
    // create new eval processor
    evalProcessor = new Processor_t(*program,
            *langDictionary, *paramDictionary,
            "", //encoding (program is invariant to it)
            ContentType_t::getDefault()->contentType); //content-type (invariant)

    // add source to source list -- main template will always have index 0
    // and also remember returned sourceindex
    sourceIndex.push(program->addSource(filesystem, filename, Error_t::Position_t()));

    // create first level-1 lexical analyzer (from file)
    lex1.push(new Lex1_t(filesystem, filename, Error_t::Position_t("", 0, 0),
                         program->getErrors()));
    // reset lex2
    lex2 = 0;

    // create first (empty) fragment context
    fragContext.push_back(ParserContext_t::FragmentContext_t());
    fragContext.back().reserve(100);

    // no print-values join below following address
    lowestValPrintAddress = 0;

    // parse input and create program
    if (tengSyntax_parse(this)) {
        // compilation error, destroy whole code
        program->erase(program->begin(), program->end());
        // if some uncaught error
        if (tengSyntax_lastErrorMessage.length() > 0) {
            // append message into error log post-mortem
            program->getErrors().logError(Error_t::LL_FATAL,
                                        Error_t::Position_t(),
                    "Parser crash: " + tengSyntax_lastErrorMessage);
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    }

    // return program
    return program;
}


/** Compile string template into a program.
  * @return Pointer to program compiled within this context.
  * @param str Whole template is stored in this string. */
Program_t* ParserContext_t::createProgramFromString(const std::string &str)
{
    // empty previous stacked values
    fragContext.clear(); //delete all fragment contexts
    fragContext.reserve(20);

    while (!sourceIndex.empty())
        sourceIndex.pop(); //delete all source indexes
    while (!lex1.empty()) {
        delete lex1.top(); //delete all lex1 objects
        lex1.pop();
    }

    // create (empty) program
    program = new Program_t;

    // delete old eval processor
    delete evalProcessor;
    // create new eval processor
    evalProcessor = new Processor_t(*program,
            *langDictionary, *paramDictionary,
            "", //encoding (program is invariant to it)
            ContentType_t::getDefault()->contentType); //content-type (invariant)

    // create first level-1 lexical analyzer (from file)
    sourceIndex.push(-1);
    lex1.push(new Lex1_t(str, "")); //no filename spec
    // reset lex2
    lex2 = 0;

    // create first (empty) fragment context
    fragContext.push_back(ParserContext_t::FragmentContext_t());
    fragContext.back().reserve(100);

    // no print-values join below following address
    lowestValPrintAddress = 0;

    // parse input and create program
    if (tengSyntax_parse(this)) {
        // compilation error, destroy whole code
        program->erase(program->begin(), program->end());
        // if some uncaught error
        if (tengSyntax_lastErrorMessage.length() > 0) {
            // append message into error log post-mortem
            program->getErrors().logError(Error_t::LL_FATAL,
                                        Error_t::Position_t(),
                    "Parser crash: " + tengSyntax_lastErrorMessage);
            tengSyntax_lastErrorMessage.erase(); //clear error
        }
    }

    // return program
    return program;
}

bool ParserContext_t::pushFragment(const Error_t::Position_t &pos,
                                   const IdentifierName_t &name,
                                   const std::string &fullName,
                                   Identifier_t &id)
{
    // check for bad name
    if (name.empty()) {
        program->getErrors().
            logError(Error_t::LL_ERROR, pos,
                     ("Invalid fragment identifier; "
                      "discarding fragment block content"));
        return false;
    }

    if (name.size() == 1) {
        // top-level fragment -- context change

//         // check for duplicity -- NOT USED
//         for (vector<FragmentContext_t>::reverse_iterator
//                  ifragContext = fragContext.rbegin();
//              ifragContext != fragContext.rend(); ++ifragContext) {
//             if (!ifragContext->empty()
//                 && (ifragContext->front() == name.back())) {
//                 program->getErrors().
//                     logError(Error_t::LL_ERROR, pos,
//                              ("Fragment context '" + fullName +
//                               "' already opened, "
//                               "you cannot open it more than once; "
//                               "discarding fragment block content"));
//                 return false;
//             }
//         }

        // push new fragment context
        fragContext.push_back(FragmentContext_t());

        // mark context change
        id.context = 1;
    } else {
        FragmentContext_t &fc = fragContext.back();
        // check for name prefix match
        if ((name.size() != (fc.name.size() + 1))
            || !std::equal(name.begin(), name.end() - 1, fc.name.begin())) {
            program->getErrors().
                logError(Error_t::LL_ERROR, pos,
                         ("Fragment '" + fullName +
                          "' badly nested into context '" + fc.fullname() +
                          "'; discarding fragment block content"));
            return false;
        }

        // no context change
        id.context = 0;
    }

    // set fragment's name
    id.name = name.back();

    // remember this new fragment
    fragContext.back().push_back(id.name, program->size());

    // OK we have new fragment
    return true;
}

void ParserContext_t::popFragment(unsigned int fragmentProgramStart) {
    // we have to forget the fragment

    // get current fragment
    FragmentContext_t &context(fragContext.back());

    const std::string name = context.name.back();
    unsigned int address = context.addresses.back();

    // update context
    context.pop_back();

    // if we are leaving non-default context remove it
    if ((fragContext.size() > 1) && context.empty())
        fragContext.pop_back();

    // calculate fragment's jumps
    (*program)[address].value.integerValue = program->size() - address - 1;
    program->back().value.integerValue = - (program->size() - address - 1);

    // no print-values join below following address
    lowestValPrintAddress = program->size();
}

void ParserContext_t::cropCode(unsigned int size) {
    // we have just to forget program generated so far
    program->erase(program->begin() + size, program->end());
}

bool ParserContext_t::findFragmentForVariable(const Error_t::Position_t &pos,
                                              const IdentifierName_t &name,
                                              const std::string &fullName,
                                              Identifier_t &id)
    const
{
    // process all contexts and try to find varible's prefix (fragment
    // name)
    for (std::vector<FragmentContext_t>::const_reverse_iterator
             ifragContext = fragContext.rbegin();
         ifragContext != fragContext.rend(); ++ifragContext) {
        if (((name.size() - 1) <= ifragContext->size())
            && std::equal(name.begin(), name.end() - 1,
                          ifragContext->name.begin())) {
            // set variables name
            id.name = name.back();
            // set context (how much to go from root)
            id.context = (fragContext.rend() - ifragContext - 1);
            // set fragment depth
            id.depth = name.size() - 1;
            return true;
        }
    }

    // log error
    program->getErrors().
        logError(Error_t::LL_ERROR, pos,
                 ("Variable '" + fullName +
                  "' doesn't match any fragment in any context; "
                  "replacing variable with undefined value."));

    // not found
    return false;
}

ParserContext_t::FragmentResolution_t
ParserContext_t::findFragment(const Error_t::Position_t *pos,
                              const IdentifierName_t &name,
                              const std::string &fullName,
                              Identifier_t &id, bool parentIsOK)
    const
{
    // handle root fragment
    if (name.empty()) {
        // name is empty
        id.name = std::string();
        // current context (root is in all contexts)
        id.context = fragContext.size() - 1;
        // root is first
        id.depth = 0;

        // OK, found
        return FR_FOUND;
    }

    // process all contexts and try to find varible's prefix (fragment
    // name)
    for (std::vector<FragmentContext_t>::const_reverse_iterator
             ifragContext = fragContext.rbegin();
         ifragContext != fragContext.rend(); ++ifragContext) {
        if ((name.size() <= ifragContext->size())
            && std::equal(name.begin(), name.end(),
                          ifragContext->name.begin())) {
            // set object name
            id.name = name.back();
            // set context (how much to go from the root context)
            id.context = (fragContext.rend() - ifragContext - 1);
            // set fragment depth
            id.depth = name.size();
            return FR_FOUND;
        } else if (parentIsOK // fragment name cannot be empty!
                   && ((name.size() - 1) <= ifragContext->size())
                   && std::equal(name.begin(), name.end() - 1,
                                 ifragContext->name.begin())) {
            // we have found parent of requested fragment

            // set object name
            id.name = name.back();
            // set context (how much to go from the root context)
            id.context = (fragContext.rend() - ifragContext - 1);
            // set fragment depth
            id.depth = name.size() - 1;
            return FR_PARENT_FOUND;
        }
    }

    // log error (only when we are allowed to do so
    if (pos)
        program->getErrors().
            logError(Error_t::LL_ERROR, *pos,
                     ("Fragment '" + fullName + "' not found in any context."));

    // not found
    return FR_NOT_FOUND;
}

ParserContext_t::ExistResolution_t
ParserContext_t::exists(const Error_t::Position_t &pos,
                        const IdentifierName_t &name,
                        const std::string &fullName, Identifier_t &id,
                        bool mustBeOpen)
    const
{
    // try to find fragment -- fragment's parent is sufficient
    FragmentResolution_t fr = findFragment(0, name, fullName, id, !mustBeOpen);

    // determine object's existence
    switch (fr) {
    case FR_FOUND:
        // we have found fragment => exist is always true
        return ER_FOUND;
    case FR_PARENT_FOUND:
        // we have found parent fragment of identifier =>
        // resolution must be made in runtime
        return ER_RUNTIME;
    case FR_NOT_FOUND:
        // not found => this object couldn't exist
        break;
    }

    // log error
    /*program->getErrors().
        logError(Error_t::LL_ERROR, pos,
                 ("Object '" + fullName + "' not found in any context."));*/

    return ER_NOT_FOUND;
}

int ParserContext_t::getFragmentAddress(const Error_t::Position_t &pos,
                                        const IdentifierName_t &name,
                                        const std::string &fullName,
                                        Identifier_t &id) const
{
    int address = fragContext.back().getAddress(name);
    if (address < 0) {
        // not found => log error and return bad address
        program->getErrors().logError
            (Error_t::LL_ERROR, pos, ("Fragment '" + fullName +
                                      "' not found in current contenxt."));
        return -1;
    }

    // set id
    id.name = name.back();
    // set context (how much to go from the root context)
    id.context = fragContext.size() - 1;
    // set fragment depth
    id.depth = name.size();

    return address;
}

int ParserContext_t::FragmentContext_t::
getAddress(const IdentifierName_t &id) const
{
    // match id in name and return associated address
    if ((id.size() <= name.size())
        && std::equal(id.begin(), id.end(), name.begin())) {

        // return address
        return addresses[id.size() - 1];
    }

    // not found
    return -1;
}

} // namespace Teng

