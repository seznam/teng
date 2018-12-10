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

#include "lex1.h"
#include "lex2.h"
#include "yystype.h"
#include "processor.h"
#include "parserfrag.h"
#include "parserdiag.h"
#include "contenttype.h"
#include "overriddenblocks.h"
#include "teng/error.h"

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
    Error_t &err,
    const Dictionary_t *dict,
    const Configuration_t *params,
    const std::string &fs_root,
    const std::string &filename,
    const std::string &encoding,
    const std::string &contentType
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
    Error_t &err,
    const Dictionary_t *dict,
    const Configuration_t *params,
    const std::string &fs_root,
    const std::string &source,
    const std::string &encoding,
    const std::string &contentType
);

namespace Parser {

/** Parser context contains all necessary parsing-time data.
 */
struct Context_t {
    /** C'tor.
     */
    Context_t(
        Error_t &err,
        const Dictionary_t *dict,
        const Configuration_t *params,
        const std::string &fs_root,
        const std::string &encoding,
        const std::string &contentType
    );

    /** D'tor.
     */
    ~Context_t();

    /** Returns next token parsed from source code.
     *
     * Source code parsing algorithm has three layers:
     * - level 1 lexer: tenglex1.h/cc,
     * - level 2 lexer: tenglex2(impl).h/ll,
     * - syntactic parser: tengsyntax.yy,
     * - semanthic actions: semanthic*.h/cc.
     *
     * This method returns directly some level 1 tokens that can be passed to
     * syntactic parser or it uses level 2 lexer to parse complex level 1
     * tokens and then returns such resulting level 2 tokens.
     */
    Token_t next_token();

    /** Returns level 1 lexer that is on the top of level 1 lexers stack.
     */
    Lex1_t &lex1() {return lex1_stack.top().lexer;}

    /** Returns level 2 lexer.
     */
    Lex2_t &lex2() {return lex2_value;}

    /** Return the number of current includes.
     */
    std::size_t include_level() const {return lex1_stack.size();}

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

    /** Stack of positions where rtvars starts.
     */
    using rtvar_strings_t = std::vector<string_view_t>;

    /** Stack of instruction addresses. The stack is needed due to nested if
     * statements.
     */
    struct addrs_stack_t {
        struct entry_t {
            using const_iterator = std::vector<int64_t>::const_iterator;
            int64_t top() {return stack.back();}
            int64_t pop() {auto t = stack.back(); stack.pop_back(); return t;}
            void push(int64_t v) {stack.push_back(v);}
            bool empty() const {return stack.empty();}
            const_iterator begin() const {return stack.begin();}
            const_iterator end() const {return stack.begin();}
            std::vector<int64_t> stack;
        };

        void push() {addrs.push({});}
        void pop() {addrs.pop();}
        bool empty() {return addrs.empty();}
        entry_t &top() {return addrs.top();}
        const entry_t &top() const {return addrs.top();}

        std::stack<entry_t> addrs;
    };

    /** The pair of instruction address and source code position. It's used to
     * remember where expression begins.
     */
    struct expr_start_t {Pos_t pos; int64_t addr; bool update_allowed;};
    using expr_starts_t = std::stack<expr_start_t>;

    /** The pair of instruction address and optimizable flag. It's used to note
     * where begins subprogram representing the expression that should be
     * processed by the optimizer. The optimizable flag is used by optimizer
     * to skip subprograms that is not optimizable.
     */
    struct optimization_point_t {int64_t addr; bool optimizable;};

    /** Stack of expression begins.
     */
    struct optim_points_t: public std::stack<optimization_point_t> {
        void clear() {while (!empty()) pop();}
    };

    /** Load source code from file.
     *
     * @param ctx Parser context.
     * @param path Template filename (absolute path).
     * @param incl_pos The include directive position.
     */
    void load_file(const string_view_t &path, const Pos_t &incl_pos);

    /** Load source code from string.
     *
     * @param ctx Parser context.
     * @param source Template source code in string.
     * @param incl_pos The include directive position.
     */
    void load_source(const std::string &source, const Pos_t *inc_pos = nullptr);

    /** Returns current address of the unfinished JMP instruction.
     */
    addrs_stack_t::entry_t &curr_branch_addrs() {return branch_addrs.top();}

    /** Returns current address of the unfinished JMP instruction.
     */
    auto &curr_if_start_point() {return if_start_points.top();}

    bool utf8;                          //!< true if templates are in utf-8
    std::unique_ptr<Program_t> program; //!< program created by parser
    const Dictionary_t *dict;           //!< language dictionary
    const Configuration_t *params;      //!< config dictionary (param.conf)
    const std::string fs_root;          //!< application root path
    SourceCodes_t source_codes;         //!< parsed/compiled source codes
    Lex1Stack_t lex1_stack;             //!< lexical analyzer (level 1)
    Lex2_t lex2_value;                  //!< lexical analyzer (level 2)
    Error_t coproc_err;                 //!< error log for optimalizer
    Processor_t coproc;                 //!< coprocessor for expr optimizer
    OpenFrames_t open_frames;           //!< stack of open fragment frames
    Variable_t var_sym;                 //!< used to build variable
    Options_t opts_sym;                 //!< used to build directive options
    bool error_occurred;                //!< used to turn off consequent errors
    Token_t unexpected_token;           //!< the last unexpected token
    expr_start_t expr_start_point;      //!< address and pos where exprs starts
    expr_starts_t if_start_points;      //!< addresses where if stmnts start
    rtvar_strings_t rtvar_strings;      //!< positions where rtvar starts
    addrs_stack_t branch_addrs;         //!< addresses of unfinished jumps
    addrs_stack_t case_option_addrs;    //!< the list of addrs of case options
    optim_points_t optimization_points; //!< adresses of "value generators"
    ExprDiag_t expr_diag;               //!< list of expression diagnostic codes
    Escaper_t escaper;                  //!< open content types / escaper
    ExtendsBlock_t extends_block;       //!< stack of open 'extends' block
    OverriddenBlocks_t overridden_blocks;//!< used to impl. template inheritance
};

} // namespace Parser
} // namespace Teng

#endif /* TENGPARSERCONTEXT_H */

