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
 * $Id: tengparsercontext.h,v 1.2 2004-12-30 12:42:02 vasek Exp $
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
#include "tengprogram.h"
#include "tengprocessor.h"

using namespace std;

namespace Teng {

/** Parser context contains all necessary parsing-time data. */
struct ParserContext_t {

    /** Fragment context type used in parse-time. */
    typedef vector<string> FragmentContext_t;
    
    /** Temporary variable list used in parse-time. */
    typedef vector<string> VariableList_t;

    /** Var/frag identifier. */
    typedef vector<string> IdentifierName_t;
    
    /** Initialize.
     * @param langDictionary Language-dependent dictionary.
     * @param paramDictionary Language-independent dictionary (param.conf).
     * @param root Application's root path for teng files. */
    ParserContext_t(const Dictionary_t *langDictionary,
                    const Configuration_t *paramDictionary,
                    const string &root);

    /** Delete lexical analyzer objects left on the stack. */
    ~ParserContext_t();
    
    /** Try to find template-created variable in variable list.
      * @return 0=not found, 1=found.
      * @param name Fully qualified name of the variable. */ 
    int tmpVariableLookup(const string &name);

    /** Compile file template into a program.
      * @return Pointer to program compiled within this context.
      * @param filename Template filename. */
    Program_t* createProgramFromFile(const string &filename);
    
    /** Compile string template into a program.
      * @return Pointer to program compiled within this context.
      * @param str Whole template is stored in this string. */
    Program_t* createProgramFromString(const string &str);

    bool pushFragment(const Error_t::Position_t &pos,
                      const IdentifierName_t &name, const string &fullName,
                      Identifier_t &id);

    void ParserContext_t::popFragment(unsigned int fragmentProgramStart);

    void ParserContext_t::cropCode(unsigned int size);

    bool findFragmentForVariable(const Error_t::Position_t &pos,
                                 const IdentifierName_t &name,
                                 const string &fullName,
                                 Identifier_t &id) const;

    enum FragmentResolution_t {
        FR_NOT_FOUND = 0,
        FR_FOUND = 1,
        FR_PARENT_FOUND = 2,
    };

    FragmentResolution_t findFragment(const Error_t::Position_t *pos,
                                      const IdentifierName_t &name,
                                      const string &fullName,
                                      Identifier_t &id,
                                      bool parentIsOK = false) const;
    
    enum ExistResolution_t {
        ER_NOT_FOUND,
        ER_FOUND,
        ER_RUNTIME,
    };

    ExistResolution_t exists(const Error_t::Position_t &pos,
                             const IdentifierName_t &name,
                             const string &fullName, Identifier_t &id,
                             bool mustBeOpen = false) const;
    
    /** Language dictionary. */
    const Dictionary_t *langDictionary;
    
    /** Language-independent dictionary (param.conf). */
    const Configuration_t *paramDictionary;
    
    /** Application root path (templates and dictionaries) */
    string root;
    
    /** Lexical analyzer (level 1) object. */
    stack<Lex1_t *> lex1;
    /** Source index relevant to the currently processed source by lex1. */
    stack<int> sourceIndex;
    
    /** Flag of using lexical analyzer (level2). */
    int lex2;
    
    /** Actual position in input stream.
     * Value is periodicaly updated by yylex(). */
    Error_t::Position_t position;
    
    /** Actual fragment context when parsing a template. */
    vector<FragmentContext_t> fragContext;
    
    /** List of temporary variables created in each fragment contexts. */
    vector<VariableList_t> variableList;
    
    /** Program created by parser.
     * Temporary value used when parsing. */
    Program_t *program;
    
    /** Lowest possible address, at which the sequence of
      * VAL, PRINT, VAL, PRINT, ... instructions can be joined
      * into the single VAL, PRINT pair. */
    unsigned int lowestValPrintAddress;
    
    /** Processor unit used for evaluation of constant expressions. */
    Processor_t *evalProcessor;
};

} // namespace Teng

#endif // TENGPARSERCONTEXT_H
