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
#include "tengyystype.h"
#include "tengprocessor.h"
#include "tengparserfrag.h"
#include "tengparserdiag.h"

namespace Teng {

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

    /** Returns next token parsed from source code.
     *
     * Source code parsing algorithm has three layers:
     * - level 1 lexer: tenglex1.h/cc,
     * - level 2 lexer: tenglex2(impl).h/ll,
     * - syntactic parser: tengsyntax.yy.
     *
     * This method returns directly some level 1 tokens that can be passed to
     * syntactic parser or it uses level 2 lexer to parse complex level 1
     * tokens and then returns such resulting level 2 tokens.
     */
    Token_t next_token();

    /** Returns level 1 lexer that is on the top of level 1 lexers stack.
     */
    Lex1_t &lex1() {return lex1_stack.top();}

    /** Returns level 2 lexer.
     */
    Lex2_t &lex2() {return lex2_value;}

    /** Returns current position in source.
     */
    const Pos_t &pos() {
        static const Pos_t nowhere;
        return lex2().in_use()
            ? lex2().position()
            : (lex1_stack.empty()? nowhere: lex1().position());
    }

    /** The list of source codes.
    */
    using SourceCodes_t = std::vector<flex_string_value_t>;

    /** Stack of instruction addresses.
     */
    class addrs_stack_t: public std::vector<int32_t> {
    public:
        using std::vector<int32_t>::vector;
        int32_t pop() {auto t = back(); pop_back(); return t;}
        void push(int32_t v) {push_back(v);}
    };

    /** The pair of instruction address and source code position. It's used to
     * note where expression begins.
     */
    struct expr_start_t {Pos_t pos; int32_t addr;};

    /** The pair of instruction address and optimizable flag. It's used to note
     * where begins subprogram representing the expression that should be
     * processed by the optimizer. The optimizable flag is used by optimizer
     * to skip subprograms that is not optimizable.
     */
    struct optimization_point_t {int32_t addr; bool optimizable;};

    /** Stack of expression begins.
     */
    struct optim_points_t: public std::stack<optimization_point_t> {
        void clear() {while (!empty()) pop();}
    };

    /** Load source code from file.
     *
     * @param ctx Parser context.
     * @param filename Template filename (absolute path).
     * @param include_pos The include directive position.
     */
    void load_file(const std::string &filename, const Pos_t *include_pos = {});

    /** Load source code from string.
     *
     * @param ctx Parser context.
     * @param source Template source code in string.
     */
    void load_source(const std::string &source);

    std::unique_ptr<Program_t> program; //!< program created by parser
    const Dictionary_t *dict;           //!< language dictionary
    const Configuration_t *params;      //!< config dictionary (param.conf)
    std::string fs_root;                //!< application root path
    SourceCodes_t source_codes;         //!< parsed/compiled source codes
    std::stack<Lex1_t> lex1_stack;      //!< lexical analyzer (level 1)
    Lex2_t lex2_value;                  //!< lexical analyzer (level 2)
    Error_t coproc_err;                 //!< error log for optimalizer
    Processor_t coproc;                 //!< coprocessor for expr optimizer
    OpenFrames_t open_frames;           //!< stack of open fragment frames
    Variable_t var_sym;                 //!< used to build variable
    Options_t opts_sym;                 //!< used to build directive options
    bool error_occurred;                //!< used to turn off consequent errors
    Token_t unexpected_token;           //!< the last unexpected token
    expr_start_t expr_start_point;      //!< address and pos where expr starts
    addrs_stack_t branch_start_addrs;   //!< addresses of unfinished jumps
    optim_points_t optimization_points; //!< adresses of "value generators"
    addrs_stack_t case_option_addrs;    //!< the list of addrs of case options
    ExprDiag_t expr_diag;               //!< list of expression diagnostic codes
};

} // namespace Parser
} // namespace Teng

#endif /* TENGPARSERCONTEXT_H */

