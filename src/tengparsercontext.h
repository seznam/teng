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
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz
 *
 * HISTORY
 * 2003-09-17  (vasek)
 *             Created.
 * 2018-07-07  (burlog)
 *             Cleaned.
 */

#ifndef TENGPARSERCONTEXT_H
#define TENGPARSERCONTEXT_H

#include <stack>
#include <string>
#include <memory>

#include "tenglex1.h"
#include "tenglex2.h"
#include "tengerror.h"
#include "tengprocessor.h"
#include "tengparserfrag.h"

namespace Teng {

/** The list of source codes.
 */
using SourceCodes_t = std::vector<Parser::flex_string_value_t>;

/** Compile file template into a program.
 *
 * @param dict Language-dependent dictionary.
 * @param params Language-independent dictionary (param.conf).
 * @param fs_root Application's root path for teng files.
 * @param filename Template filename (relative to fs_root).
 *
 * @return Pointer to program compiled within this context.
 */
std::unique_ptr<Program_t>
compile_file(
    const Dictionary_t *dict,
    const Configuration_t *params,
    const std::string &fs_root,
    const std::string &filename
);

/** Compile string template into a program.
 *
 * @param dict Language-dependent dictionary.
 * @param params Language-independent dictionary (param.conf).
 * @param fs_root Application's root path for teng files.
 * @param source Whole template is stored in this string.
 *
 * @return Pointer to program compiled within this context.
 */
std::unique_ptr<Program_t>
compile_string(
    const Dictionary_t *dict,
    const Configuration_t *params,
    const std::string &fs_root,
    const std::string &source
);

namespace Parser {

/** Parser context contains all necessary parsing-time data.
 */
struct Context_t {
    /** C'tor.
     */
    Context_t(
        const Dictionary_t *dict,
        const Configuration_t *params,
        const std::string &fs_root
    );

    /** D'tor.
     */
    ~Context_t();

    /** Returns current position in source.
     */
    const Pos_t &position() {
        // TODO(burlog): depends on fullness of lex1 stack
        return lex2.in_use()? lex2.position(): lex1_stack.top().position();
    }

    /** Returns level 1 lexer on top of stack of lexers.
     */
    Lex1_t &lex1() {return lex1_stack.top();}

    std::unique_ptr<Program_t> program; //!< program created by parser
    const Dictionary_t *dict;           //!< language dictionary
    const Configuration_t *params;      //!< config dictionary (param.conf)
    std::string fs_root;                //!< application root path
    SourceCodes_t source_codes;         //!< parsed/compiled source codes
    std::stack<Lex1_t> lex1_stack;      //!< lexical analyzer (level 1)
    Lex2_t lex2;                        //!< lexical analyzer (level 2)
    Error_t coproc_err;                 //!< error log for optimalizer
    Processor_t coproc;                 //!< auxiliary processor for optimalizer
    IdentifierPath_t ident;             //!< temporary for building variables
    FragmentContexts_t frag_ctxs;       //!< stack of frag contexts
};

/** Compile file template into a program.
 *
 * @param ctx Parser context.
 * @param filename Template filename (absolute path).
 */
void compile_file(Context_t *ctx, const std::string &filename);

} // namespace Parser
} // namespace Teng

#endif // TENGPARSERCONTEXT_H

