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
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-17  (vasek)
 *             Created.
 * 2005-06-21  (roman)
 *             Win32 support.
 * 2006-06-21  (sten__)
 *             Commented out error reporting of exist function.
 * 2018-07-07  (burlog)
 *             Extracted from tengparsercontext.cc.
 */

#include <algorithm>

#include "tenglex1.h"
#include "tengprogram.h"
#include "tengplatform.h"
#include "tenglogging.h"
#include "tengsyntax.hh"
#include "tengcontenttype.h"
#include "tengparsercontext.h"

namespace Teng {
namespace Parser {
namespace {
} // namespace

std::string Context_t::FragmentContext_t::fullname() const {
    return Parser::fullname(name);
}

bool Context_t::pushFragment(const Position_t &pos,
                             const IdentifierName_t &name,
                             Identifier_t &id)
{
    auto &err = program->getErrors();

    // check for bad name
    if (name.empty()) {
        logError(err, pos,
                 "Invalid fragment identifier; "
                 "discarding fragment block content");
        return false;
    }

    if (name.size() == 1) {
        // top-level fragment -- context change

        std::cerr << "OPEN NEW ctx!!!!!!!!!!!! " << fullname(name) << std::endl;

        // push new fragment context
        fragContext.push_back(FragmentContext_t());

        // mark context change
        id.context = 1;
    } else {
        FragmentContext_t &fc = fragContext.back();
        // check for name prefix match
        if ((name.size() != (fc.name.size() + 1))
            || !std::equal(name.begin(), name.end() - 1, fc.name.begin())) {
            logError(err, pos,
                     "Fragment '" + fullname(name) +
                     "' badly nested into context '" + fc.fullname() +
                     "'; discarding fragment block content");
            return false;
        }

        std::cerr << "KEEEEEP ctx!!!!!!!!!!!! " << fullname(name) << std::endl;

        // no context change
        id.context = 0;
    }

    // set fragment's name
    id.name = name.back();

    // remember this new fragment
    fragContext.back().push_back(id.name, program->size());

    std::cerr << "##############################################" << std::endl;
    for (auto i: fragContext)
        std::cerr << "******** " << i.fullname() << std::endl;

    // OK we have new fragment
    return true;
}

void Context_t::popFragment(unsigned int fragmentProgramStart) {
    // get current fragment
    FragmentContext_t &context = fragContext.back();
    unsigned int address = context.addresses.back();

    // update context
    context.pop_back();

    // if we are leaving non-default context remove it
    if ((fragContext.size() > 1) && context.empty())
        fragContext.pop_back();

    // calculate fragment's jumps
    (*program)[address].value = program->size() - address - 1;
    program->back().value = - (program->size() - address - 1);

    // no print-values join below following address
    lowestValPrintAddress = program->size();
}

void Context_t::cropCode(unsigned int size) {
    // we have just to forget program generated so far
    program->erase(program->begin() + size, program->end());
}

bool Context_t::findFragmentForVar(const Position_t &pos,
                                   const IdentifierName_t &name,
                                   Identifier_t &id) const
{
    // process all contexts and try to find varible's prefix (fragment name)
    for (auto ifragctx = fragContext.rbegin();
         ifragctx != fragContext.rend(); ++ifragctx)
    {
        if ((name.size() - 1) > ifragctx->size()) continue;
        if (std::equal(name.begin(), name.end() - 1, ifragctx->name.begin())) {
            // set variables name
            id.name = name.back();
            // set context (how much to go from root)
            id.context = (fragContext.rend() - ifragctx - 1);
            // set fragment depth
            id.depth = name.size() - 1;
            return true;
        }
    }

    // log error
    logError(program->getErrors(), pos,
             "Variable '" + fullname(name) +
             "' doesn't match any fragment in any context; "
             "replacing variable with undefined value.");

    // not found
    return false;
}

Context_t::FR
Context_t::findFragment(const Position_t *pos,
                        const IdentifierName_t &name,
                        Identifier_t &id,
                        bool parentIsOK) const
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
        return FR::FOUND;
    }

    // process all contexts and try to find varible's prefix (fragment name)
    for (auto ifragctx = fragContext.rbegin();
         ifragctx != fragContext.rend(); ++ifragctx)
    {
        if ((name.size() <= ifragctx->size())
            && std::equal(name.begin(), name.end(),
                          ifragctx->name.begin()))
        {
            // set object name
            id.name = name.back();
            // set context (how much to go from the root context)
            id.context = (fragContext.rend() - ifragctx - 1);
            // set fragment depth
            id.depth = name.size();
            return FR::FOUND;

        } else if (parentIsOK // fragment name cannot be empty!
                   && ((name.size() - 1) <= ifragctx->size())
                   && std::equal(name.begin(), name.end() - 1,
                                 ifragctx->name.begin()))
        {
            // we have found parent of requested fragment

            // set object name
            id.name = name.back();
            // set context (how much to go from the root context)
            id.context = (fragContext.rend() - ifragctx - 1);
            // set fragment depth
            id.depth = name.size() - 1;
            return FR::PARENT_FOUND;
        }
    }

    // log error (only when we are allowed to do so)
    if (pos) {
        logError(program->getErrors(), *pos,
                 "Fragment '" + fullname(name) + "' not found in any context.");
    }

    // not found
    return FR::NOT_FOUND;
}

Context_t::ER
Context_t::exists(const Position_t &pos,
                  const IdentifierName_t &name,
                  Identifier_t &id,
                  bool mustBeOpen)
    const
{
    // try to find fragment -- fragment's parent is sufficient
    auto fr = findFragment(0, name, id, !mustBeOpen);

    // determine object's existence
    switch (fr) {
    case FR::FOUND:
        // we have found fragment => exist is always true
        return ER::FOUND;
    case FR::PARENT_FOUND:
        // we have found parent fragment of identifier =>
        // resolution must be made in runtime
        return ER::RUNTIME;
    case FR::NOT_FOUND:
        // not found => this object couldn't exist
        break;
    }
    return ER::NOT_FOUND;
}

int Context_t::getFragmentAddress(const Position_t &pos,
                                  const IdentifierName_t &name,
                                  Identifier_t &id) const
{
    int address = fragContext.back().getAddress(name);
    if (address < 0) {
        // not found => log error and return bad address
        logError(program->getErrors(), pos,
                 "Fragment '" + fullname(name)
                 + "' not found in current context.");
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

int
Context_t
::FragmentContext_t::getAddress(const IdentifierName_t &id) const {
    // match id in name and return associated address
    if (id.size() <= name.size())
        if (std::equal(id.begin(), id.end(), name.begin()))
            return addresses[id.size() - 1];
    return -1;
}

} // namespace Parser
} // namespace Teng

