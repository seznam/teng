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
 * $Id: tengfragmentstack.h,v 1.3 2004-12-30 12:42:01 vasek Exp $
 *
 * DESCRIPTION
 * Teng stack fragment frame.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2004-05-10  (vasek)
 *             Created.
 */

#ifndef TENGFRAGEMENTSTACK_H
#define TENGFRAGEMENTSTACK_H

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <iostream>

#include "tengparservalue.h"
#include "tenginstruction.h"

using namespace std;

namespace Teng {

const string ERROR_FRAG_NAME("_error");

const string FILENAME("filename");

const string NO_FILE("(no file)");

const string LINE("line");

const string COLUMN("column");

const string LEVEL("level");

const string MESSAGE("message");

enum Status_t {
    S_OK =               0,
    S_NOT_FOUND =       -1,
    S_BAD =             -2,
    S_OUT_OF_CONTEXT =  -3,
    S_ALREADY_DEFINED = -4,
    S_TYPE_MISMATCH =   -5,
    S_NO_ITERATIONS =   -6,
};

class FragmentFrame_t {
public:
    inline FragmentFrame_t() {
    }

    inline virtual ~FragmentFrame_t() {
        // no-op
    }

    virtual const FragmentList_t* findSubFragment(const string &name)
        const = 0;

    virtual Status_t findVariable(const string &name, ParserValue_t &var)
        const = 0;

    virtual bool nextIteration() = 0;

    virtual bool overflown() = 0;

    virtual unsigned int size() const = 0;

    virtual unsigned int iteration() const = 0;

    virtual bool exists(const string &name, bool onlyData = false) const = 0;

    bool localExists(const string &name) const {
        return (locals.find(name) != locals.end());
    }

    inline Status_t findLocalVariable(const string &name, ParserValue_t &var,
                                      bool testExistence = false)
        const
    {
        // try to find variable
        map<string, ParserValue_t>::const_iterator flocals
            = locals.find(name);
        if (flocals == locals.end()) return S_NOT_FOUND;

        // check whether we are only checking for existence
        if (testExistence) return S_OK;

        // found => assing
        var = flocals->second;

        // OK
        return S_OK;
    }

    inline Status_t setVariable(const string &name, const ParserValue_t &var) {
        // check whether there is non-local variable with given name
        // -- we are not allowed to override data from template
        if (exists(name, true)) return S_ALREADY_DEFINED;

        // try to insert value to local variables 
        pair<map<string, ParserValue_t>::iterator, bool> insertResult
            = locals.insert(make_pair(name, var));
        if (!insertResult.second) {
            // unsuccesful (already set) -- change its value
            insertResult.first->second = var;
        }

        // OK
        return S_OK;
    }

    inline void resetLocals() {
        // remove all local variables (only when non empty)
        if (!locals.empty())
            locals = map<string, ParserValue_t>();
    }

private:
    map<string, ParserValue_t> locals;
};

class RegularFragmentFrame_t : public FragmentFrame_t {
public:
    RegularFragmentFrame_t(const FragmentList_t *fragmentList = 0)
        : FragmentFrame_t(),
          fragment(fragmentList
                   ? (fragmentList->empty() ? 0 : *fragmentList->begin())
                   : 0),
          data(fragmentList ? fragmentList->begin()
               : FragmentList_t::const_iterator()),
          dataEnd(fragmentList ? fragmentList->end()
                  : FragmentList_t::const_iterator()),
          dataSize(fragmentList ? fragmentList->size() : 0),
          index(0)
    {
        // no-op
    }
    
    RegularFragmentFrame_t(const Fragment_t *fragment)
        : FragmentFrame_t(),
          fragment(fragment),
          data(FragmentList_t::const_iterator()),
          dataEnd(FragmentList_t::const_iterator()),
          dataSize(1), index(0)
    {
        // no-op
    }

    virtual bool exists(const string &name, bool onlyData = false) const {
        Fragment_t::const_iterator ffragment = fragment->find(name);
        if (ffragment != fragment->end()) {
            // we have found identifier in data
            if (ffragment->second->nestedFragments) {
                // identifier is fragment -- we have test whether it has any
                // iteration
                if (!ffragment->second->nestedFragments->empty())
                    return true;
                // fragment is empty => there can be local variable of
                // this name
            } else return true; // identifier is variable => exists
        }
        return onlyData ? false : localExists(name);
    }

    virtual const FragmentList_t* findSubFragment(const string &name) const {
        map<string, FragmentValue_t*>::const_iterator subFragment
            = fragment->find(name);
        return ((subFragment == fragment->end()) ? 0
                : subFragment->second->nestedFragments);
    }

    virtual Status_t findVariable(const string &name, ParserValue_t &var)
        const
    {
        // try to find variable in the associated fragment
        map<string, FragmentValue_t*>::const_iterator element
            = fragment->find(name);

        // when not found => try to find local variable
        if (element == fragment->end())
            return findLocalVariable(name, var);

        // check whether found element is value (has no nested fragments)
        if (element->second->nestedFragments)
            return S_TYPE_MISMATCH;

        // OK we have variable's value from data tree!
        var.setString(element->second->value);
        return S_OK;
    }

    virtual bool nextIteration() {
        // check for end
        if (data == dataEnd) return false;

        // increment index
        ++index;

        // increment data iterator and check for overflow
        if (++data == dataEnd) return false;

        // set current fragment (dereference data)
        fragment = *data;

        // reset local data;
        resetLocals();

        // ok, we have next iteration
        return true;
    }

    virtual bool overflown() {
        // we have only one iteration
        return data == dataEnd;
    }

    virtual unsigned int size() const {
        return dataSize;
    }

    virtual unsigned int iteration() const {
        return index;
    }

private:
    const Fragment_t *fragment;
    FragmentList_t::const_iterator data;
    FragmentList_t::const_iterator dataEnd;

    unsigned int dataSize;
    unsigned int index;
};

class ErrorFragmentFrame_t : public FragmentFrame_t {
public:
    inline ErrorFragmentFrame_t(const Error_t &error)
        : FragmentFrame_t(), errors(error.getEntries()),
          errorSize(error.count()), index(0)
        
    {
        // no-op
    }

    virtual bool exists(const string &name, bool onlyData = false) const {
        if ((name == FILENAME) || (name == LINE)
             || (name == COLUMN) || (name == LEVEL)
            || (name == MESSAGE)) return true;
        return onlyData ? false : localExists(name);
    }

    virtual const FragmentList_t* findSubFragment(const string &name) const {
        // error fragment has no descendants
        return 0;
    }

    virtual Status_t findVariable(const string &name, ParserValue_t &var)
        const
    {
        // try to match variable names
        if (name == FILENAME) {
            const string &filename = errors[index].pos.filename;
            var.setString(filename.empty() ? NO_FILE : filename);
            return S_OK;
        } else if (name == LINE) {
            var.setInteger(errors[index].pos.lineno);
            return S_OK;
        } else if (name == COLUMN) {
            var.setInteger(errors[index].pos.col);
            return S_OK;
        } else if (name == LEVEL) {
            var.setInteger(errors[index].level);
            return S_OK;
        } else if (name == MESSAGE) {
            var.setString(errors[index].message);
            return S_OK;
        }
        
        // nothing matched, return local variable
        return findLocalVariable(name, var);
    }

    virtual bool nextIteration() {
        // check for end
        if (index == errorSize) return false;

        // reset local variables;
        resetLocals();

        // increment data iterator and return whether we have not runaway
        return (++index != errorSize);
    }

    virtual bool overflown() {
        // we have only one iteration
        return index == errorSize;
    }

    virtual unsigned int size() const {
        return errorSize;
    }

    virtual unsigned int iteration() const {
        return index;
    }

private:
    const std::vector<Error_t::Entry_t> &errors;
    unsigned int errorSize;
    unsigned int index;
};

class FragmentChain_t {
public:
    inline FragmentChain_t(FragmentFrame_t *rootFrame) {
        frames.reserve(100);
        frames.push_back(rootFrame);
    }

    inline ~FragmentChain_t() {
        // get rid of all remaining frames
        for (vector<FragmentFrame_t*>::iterator iframes = frames.begin();
             iframes != frames.end(); ++iframes)
            if (iframes != frames.begin())
                delete *iframes;
    }

    inline void pushFrame(const string &name, FragmentFrame_t *frame) {
        path.push_back(name);
        frames.push_back(frame);
    }

    inline Status_t popFrame() {
        // check for underflow
        if (path.empty()) return S_OUT_OF_CONTEXT;

        // remove last path part
        path.pop_back();

        // remove and delete last frame
        delete frames.back();
        frames.pop_back();

        // OK
        return S_OK;
    }

    inline const FragmentList_t* findSubFragment(const string &name) const {
        return frames.back()->findSubFragment(name);
    }

    inline Status_t findVariable(const Identifier_t &name, ParserValue_t &var)
        const
    {
        // check for range
        if (name.depth > frames.size()) return S_OUT_OF_CONTEXT;
        return (*(frames.begin() + name.depth))->findVariable(name.name, var);
    }

    inline Status_t setVariable(const Identifier_t &name,
                                const ParserValue_t &var)
    {
        // check for range
        if (name.depth > frames.size()) return S_OUT_OF_CONTEXT;
        return (*(frames.begin() + name.depth))->setVariable(name.name, var);
    }

    inline Status_t getFragmentSize(const Identifier_t &name,
                               unsigned int &fragmentSize)
        const
    {
        // check for range
        if (name.depth > path.size()) return S_OUT_OF_CONTEXT;

        // get size
        fragmentSize = (*(frames.begin() + name.depth))->size();
        return S_OK;
    }

    inline Status_t getSubFragmentSize(const Identifier_t &name,
                                       unsigned int &fragmentSize)
        const
    {
        // check for range
        if (name.depth > path.size()) return S_OUT_OF_CONTEXT;
        
        // find subfragment by name
        const FragmentList_t *subFragment = findSubFragment(name.name);
        if (subFragment) {
            // get size
            fragmentSize = subFragment->size();
        } else {
            // subfragment not present, treating as zero size
            fragmentSize = 0;
        }
        
        // OK
        return S_OK;
    }

    inline Status_t getFragmentIteration(const Identifier_t &name,
                                         unsigned int &fragmentIteration)
        const 
    {
        // check for range
        if (name.depth > path.size()) return S_OUT_OF_CONTEXT;

        // get iteration
        fragmentIteration = (*(frames.begin() + name.depth))->iteration();
        return S_OK;
    }

    inline bool exists(const Identifier_t &name) const {
        // check for range
        if (name.depth > path.size()) return false;

        // test for existence
        return (*(frames.begin() + name.depth))->exists(name.name);
    };

    inline bool empty() const {
        // check for empty path
        return path.empty();
    }

    inline unsigned int size() const {
        // check for empty path
        return path.size();
    }

    inline bool nextIteration() {
        return frames.back()->nextIteration();
    }

    inline bool overflown() {
        return frames.back()->overflown();
    }

private:
    vector<string> path;

    vector<FragmentFrame_t*> frames;
};


class FragmentStack_t {
public:
    FragmentStack_t(const Fragment_t *data, Error_t &error,
                    bool enableErrorFragment = false)
        : data(data), error(error), enableErrorFragment(enableErrorFragment),
          root(data)
    {
        // reserve some space and create new fragment chain for whole
        // page
        chains.reserve(20);
        chains.push_back(&root);
    }

    ~FragmentStack_t() {
    }

    inline Status_t pushFrame(const Identifier_t &name) {
        if (name.context) {
            // check whether context has been changed
            chains.push_back(&root);
        }

        // get current chain
        FragmentChain_t &chain = chains.back();

         // create fragment frame
        FragmentFrame_t *frame;

        // create new frame
        if (enableErrorFragment && chain.empty()
            && (name.name == ERROR_FRAG_NAME)) {
            // error fragment
            frame = new ErrorFragmentFrame_t(error);
        } else {
            // regular fragment
            frame = new RegularFragmentFrame_t
                (chain.findSubFragment(name.name));
        }

        // check for empty fragment
        if (frame->overflown()) {
            // we have to remove new context (if just created)
            if (name.context) chains.pop_back();

            // get rid of frame
            delete frame;
            return S_NO_ITERATIONS;
        }

        // ok we have at least one iteration
        chain.pushFrame(name.name, frame);
        return S_OK;
    }

    inline bool nextIteration() {
        // process next iteration of current fragment
        return chains.back().nextIteration();
    }

    inline Status_t popFrame() {
        // get current chain
        FragmentChain_t &chain = chains.back();

        // pop frame from last frame chain
        if (Status_t s = chain.popFrame())
            return s;
        // if chain is empty and is not first root remove it
        if ((chains.size() > 1) && (chain.empty()))
            chains.pop_back();
        return S_OK;
    }

    inline Status_t findVariable(const Identifier_t &name, ParserValue_t &var)
        const
    {
        // test whether we are in range
        if (chains.size() > name.context) {
            // get context's chain and try to find variable in it
            return (chains.begin() + name.context)->findVariable(name, var);
        }
        // not found (invalid context position) => probably badly composed
        // bytecode
        return S_OUT_OF_CONTEXT;
    };

    inline Status_t setVariable(const Identifier_t &name,
                                const ParserValue_t &var)
    {
        // test whether we are in range
        if (chains.size() > name.context) {
            // get context's chain and try to set variable in it
            return (chains.begin() + name.context)->setVariable(name, var);
        }

        // not found (invalid context position) => probably badly composed
        // bytecode
        return S_OUT_OF_CONTEXT;
    };

    inline Status_t getFragmentSize(const Identifier_t &name,
                                    unsigned int &fragmentSize)
        const
    {
        // test whether we are in range
        if (chains.size() > name.context) {
            // get context's chain and try to find variable in it
            return (chains.begin() + name.context)->
                getFragmentSize(name, fragmentSize);
        }
        // not found (invalid context position) => probably badly composed
        // bytecode
        return S_OUT_OF_CONTEXT;
    }

    inline Status_t getSubFragmentSize(const Identifier_t &name,
                                       unsigned int &fragmentSize)
        const
    {
        // test whether we are in range
        if (chains.size() > name.context) {
            // check for error fragment;
            if (enableErrorFragment && !name.depth
                && (name.name == ERROR_FRAG_NAME)) {
                fragmentSize = error.count();
                return S_OK;
            }

            // get context's chain and try to find variable in it
            return (chains.begin() + name.context)->
                getSubFragmentSize(name, fragmentSize);
        }
        // not found (invalid context position) => probably badly composed
        // bytecode
        return S_OUT_OF_CONTEXT;
    }

    inline Status_t getFragmentIteration(const Identifier_t &name,
                                         unsigned int &fragmentIteration)
        const
    {
        // test whether we are in range
        if (chains.size() > name.context) {
            // get context's chain and try to find variable in it
            return (chains.begin() + name.context)->
                getFragmentIteration(name, fragmentIteration);
        }
        // not found (invalid context position) => probably badly composed
        // bytecode
        return S_OUT_OF_CONTEXT;
    }

    inline Status_t exists(const Identifier_t &name) const {
        // test whether we are in range
        if (chains.size() > name.context) {
            // get context's chain and try to find variable in it
            return ((chains.begin() + name.context)->exists(name)
                    ? S_OK : S_NOT_FOUND);
        }
        // not found (invalid context position) => probably badly composed
        // bytecode
        return S_OUT_OF_CONTEXT;
    };


private:
    FragmentStack_t(const FragmentStack_t&);
    FragmentStack_t operator= (const FragmentStack_t&);

    const Fragment_t *data;
    Error_t &error;
    bool enableErrorFragment;
    
    RegularFragmentFrame_t root;
    vector<FragmentChain_t> chains;
};

} // namespace Teng

#endif // TENGFRAGEMENTSTACK_H
