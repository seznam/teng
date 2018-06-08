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
 * $Id: tengparsercontext.h,v 1.5 2006-10-18 08:31:09 vasek Exp $
 *
 * DESCRIPTION
 * Teng parser context.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-17  (vasek)
 *             Created.
 */

#ifndef TENGPARSERCONTEXT_H
#define TENGPARSERCONTEXT_H

#include <string>
#include <vector>
#include <stack>

#include "tengdictionary.h"
#include "tengconfiguration.h"
#include "tenglex1.h"
#include "tenglex2.h"
#include "tengprogram.h"
#include "tengprocessor.h"

namespace Teng {

/** Parser context contains all necessary parsing-time data. */
struct ParserContext_t {
    /** Var/frag identifier. */
    using IdentifierName_t = std::vector<std::string>;
    /** Position_t in template. */
    using Position_t = Error_t::Position_t;

    /** Initialize.
     * @param langDictionary Language-dependent dictionary.
     * @param paramDictionary Language-independent dictionary (param.conf).
     * @param root Application's root path for teng files. */
    ParserContext_t(const Dictionary_t *langDictionary,
                    const Configuration_t *paramDictionary,
                    const std::string &root);

    /** Delete lexical analyzer objects left on the stack. */
    ~ParserContext_t();

    /** Compile file template into a program.
      * @return Pointer to program compiled within this context.
      * @param filename Template filename. */
    Program_t* createProgramFromFile(const std::string &filename);

    /** Compile string template into a program.
      * @return Pointer to program compiled within this context.
      * @param str Whole template is stored in this string. */
    Program_t* createProgramFromString(const std::string &str);

    bool pushFragment(const Position_t &pos,
                      const IdentifierName_t &name, const std::string &fullName,
                      Identifier_t &id);

    void popFragment(unsigned int fragmentProgramStart);

    void cropCode(unsigned int size);

    bool findFragmentForVariable(const Position_t &pos,
                                 const IdentifierName_t &name,
                                 const std::string &fullName,
                                 Identifier_t &id) const;

    enum FragmentResolution_t {
        FR_NOT_FOUND = 0,
        FR_FOUND = 1,
        FR_PARENT_FOUND = 2,
    };

    FragmentResolution_t findFragment(const Position_t *pos,
                                      const IdentifierName_t &name,
                                      const std::string &fullName,
                                      Identifier_t &id,
                                      bool parentIsOK = false) const;

    enum ExistResolution_t {
        ER_NOT_FOUND,
        ER_FOUND,
        ER_RUNTIME,
    };

    ExistResolution_t exists(const Position_t &pos,
                             const IdentifierName_t &name,
                             const std::string &fullName, Identifier_t &id,
                             bool mustBeOpen = false) const;

    int getFragmentAddress(const Position_t &pos,
                           const IdentifierName_t &name,
                           const std::string &fullName,
                           Identifier_t &id) const;

    /** Language dictionary. */
    const Dictionary_t *langDictionary;

    /** Language-independent dictionary (param.conf). */
    const Configuration_t *paramDictionary;

    /** Application root path (templates and dictionaries) */
    std::string root;

    /** Lexical analyzer (level 1) object. */
    std::stack<Lex1_t *> lex1;
    /** Source index relevant to the currently processed source by lex1. */
    std::stack<int> sourceIndex;

    /** Flag of using lexical analyzer (level2). */
    bool lex2InUse;

    /** Actual position in input stream.
     * Value is periodicaly updated by yylex(). */
    Position_t position;

    struct FragmentContext_t {
        void reserve(unsigned int n) {
            name.reserve(n);
            addresses.reserve(n);
        }

        std::string fullname() const {
            std::string fn;
            for (IdentifierName_t::const_iterator iname = name.begin();
                 iname != name.end(); ++iname) {
                fn.push_back('.');
                fn.append(*iname);
            }
            return fn;
        }

        void push_back(const std::string &n, int a) {
            name.push_back(n);
            addresses.push_back(a);
        }

        void pop_back() {
            name.pop_back();
            addresses.pop_back();
        }

        bool empty() const {
            return name.empty();
        }

        unsigned int size() const {
            return name.size();
        }

        operator const IdentifierName_t&() const {
            return name;
        }

        int getAddress(const IdentifierName_t &id) const;

        IdentifierName_t name;
        std::vector<int> addresses;
    };

    /** Actual fragment context when parsing a template. */
    std::vector<FragmentContext_t> fragContext;

    /** Program created by parser.
     * Temporary value used when parsing. */
    Program_t *program;

    /** Lowest possible address, at which the sequence of
      * VAL, PRINT, VAL, PRINT, ... instructions can be joined
      * into the single VAL, PRINT pair. */
    unsigned int lowestValPrintAddress;

    /** Processor unit used for evaluation of constant expressions. */
    Processor_t *evalProcessor;

    /** lex2 scanner instance */
    Lex2_t lex2;

    // for error handling positions
    Position_t lex1Pos; //start pos of current lex1 element
    Position_t lex2Pos; //actual position in lex2 stream



    void logWarning(const Position_t &pos, const std::string &msg) const {
        program->getErrors().logError(Error_t::LL_WARNING, pos, msg);
    }

    void logError(const Position_t &pos, const std::string &msg) const {
        program->getErrors().logError(Error_t::LL_ERROR, pos, msg);
    }

    void logFatal(const Position_t &pos, const std::string &msg) const {
        program->getErrors().logError(Error_t::LL_FATAL, pos, msg);
    }

    std::string lastErrorMessage; // last error message of the parser
};

} // namespace Teng

#endif // TENGPARSERCONTEXT_H

