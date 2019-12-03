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
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz
 *
 * HISTORY
 * 2003-09-17  (vasek)
 *             Created.
 * 2005-06-21  (roman)
 *             Win32 support.
 * 2006-06-21  (sten__)
 *             Commented out error reporting of exist function.
 * 2018-07-07  (burlog)
 *             Cleaned.
 */

#include <algorithm>
#include <fstream>

#include "lex1.h"
#include "program.h"
#include "platform.h"
#include "logging.h"
#include "syntax.hh"
#include "contenttype.h"
#include "configuration.h"
#include "parsercontext.h"

#ifdef DEBUG
#define DBG(...) __VA_ARGS__
#else /* DEBUG */
#define DBG(...)
#endif /* DEBUG */

namespace Teng {
namespace {

// forwards
using flex_string_value_t = Parser::flex_string_value_t;
using flex_string_view_t = Parser::flex_string_view_t;

void compile(Parser::Context_t *ctx) {
    // parse input by bison generated parser and compile program
    Parser::Parser_t parser(ctx);
    // parser.set_debug_level(10);
    if (parser.parse() != 0) {
        // write diagnostic messages if any
        ctx->expr_diag.unwind(ctx, ctx->unexpected_token);
        // destroy invalid program and report final fatal message
        ctx->program->clear();
        logFatal(
            ctx,
            {/*use default pos, all level 1 lexers might be gone*/},
            "Unrecoverable syntax error; discarding whole program"
        );
    }
}

/** If the last instruction of program is a PRINT then it is marked as
 * unoptimizable.
 */
void make_last_print_unoptimalizable(Parser::Context_t *ctx) {
    if (ctx->program->empty()) return;
    if (ctx->program->back().opcode() != OPCODE::PRINT) return;
    ctx->program->back().as<Print_t>().unoptimizable = true;
}

/** Resets level 2 lexer.
 */
void finish_level_2_scanning(Parser::Context_t *ctx) {
    if (ctx->lex2_value.in_use()) {
        auto token = ctx->lex2_value.next();
        if (token) {
            logWarning(
                ctx,
                ctx->lex2_value.position(),
                "Level 2 lexer contains unprocessed data and it is requested "
                "to load new data from other source: first-unprocessed-token="
                + token.token_name()
            );
        }
        ctx->lex2_value.finish_scanning();
    }
}

} // namespace

std::unique_ptr<Program_t>
compile_file(
    Error_t &err,
    const Dictionary_t *dict,
    const Configuration_t *params,
    const FilesystemInterface_t *filesystem,
    const std::string &filename,
    const std::string &encoding,
    const std::string &contentType
) {
    Parser::Context_t ctx(err, dict, params, filesystem, encoding, contentType);
    ctx.load_file(filename, Pos_t(/*base level, no include reference*/));
    compile(&ctx);
    return std::move(ctx.program);
}

std::unique_ptr<Program_t>
compile_string(
    Error_t &err,
    const Dictionary_t *dict,
    const Configuration_t *params,
    const FilesystemInterface_t *filesystem,
    const std::string &source,
    const std::string &encoding,
    const std::string &contentType
) {
    Parser::Context_t ctx(err, dict, params, filesystem, encoding, contentType);
    ctx.load_source(source);
    compile(&ctx);
    return std::move(ctx.program);
}

namespace Parser {

Context_t::Context_t(
    Error_t &err,
    const Dictionary_t *dict,
    const Configuration_t *params,
    const FilesystemInterface_t* filesystem,
    const std::string &encoding,
    const std::string &contentType
): utf8(encoding == "utf-8"),
   program(std::make_unique<Program_t>(err)), dict(dict), params(params),
   filesystem(filesystem), source_codes(), lex1_stack(),
   lex2_value(params, utf8, err),
   coproc_err(), coproc(coproc_err, *program, *dict, *params),
   open_frames(*program), var_sym(), opts_sym(),
   error_occurred(false), unexpected_token{LEX2::INV, {}, {}},
   expr_start_point{{}, -1, true}, if_start_points(),
   branch_addrs(), case_option_addrs(), optimization_points(),
   escaper(ContentType_t::find(contentType))
{}

Context_t::~Context_t() = default;

void Context_t::load_file(const string_view_t &path, const Pos_t &incl_pos) {
    // register source into program sources list
    // the registration routine returns program lifetime durable pointer to
    // filename that can be used as pointer to filename in Pos_t instancies
    std::string filename = path.str();
    try {
        // load source code from file
        source_codes.push_back(flex_string_value_t(filesystem->read(filename)));
        auto &source_code = source_codes.back();

        auto *source_path = program->addSource(filesystem, filename).first;

        // create the level 1 lexer for given source code
        lex1_stack.emplace(source_code, utf8, params, source_path);

        // drop excessive tokens in level 2 lexers
        finish_level_2_scanning(this);

    } catch (const std::exception &e) {
        logError(
            this,
            incl_pos,
            "Error reading file '" + filename + "' " + "(" + e.what() + ")"
        );
    }
}

void Context_t::load_source(const std::string &source, const Pos_t *pos) {
    // copy source code to flex parsable string
    flex_string_value_t source_code(source.size());
    std::copy(source.begin(), source.end(), source_code.data());
    source_codes.push_back(std::move(source_code));

    // create the level 1 lexer for given source code
    pos != nullptr
        ? lex1_stack.emplace(source_codes.back(), utf8, params, *pos)
        : lex1_stack.emplace(source_codes.back(), utf8, params);

    // drop excessive tokens in level 2 lexers
    finish_level_2_scanning(this);
}

Token_t Context_t::next_token() {
    Pos_t last_valid_pos;

    // until there is some source code to parse
    while (!lex1_stack.empty()) {
        // if level 2 lexer is currently in use get next L2 token and process it
        if (lex2().in_use()) {
            switch (auto token = lex2().next()) {
            default:
                DBG(std::cerr << "**** " << token << std::endl);
                return token;
            case LEX2::INV:
                DBG(std::cerr << "!!!! " << token << std::endl);
                return token;
            case LEX2_EOF:
                DBG(std::cerr << "---- " << token << std::endl);
                lex2().finish_scanning();
                break;
            }
        }

        // get next L1 token and process it
        using LEX1 = Lex1_t::LEX1;
        switch (auto token = lex1().next()) {
        case LEX1::DICT:
        case LEX1::TENG: case LEX1::TENG_SHORT:
        case LEX1::ESC_EXPR: case LEX1::RAW_EXPR:
            DBG(std::cerr << "* " << token << std::endl);
            lex2().start_scanning(std::move(token.flex_view()), token.pos);
            continue; // parse token value by level 2 lexer in next loop

        case LEX1::TEXT:
            DBG(std::cerr << "**** " << token << std::endl);
            return {LEX2::TEXT, token.pos, token.string_view()};

        case LEX1::ERROR:
            DBG(std::cerr << "- " << token << std::endl);
            logError(this, token.pos, token.string_view());
            continue; // ignore bad element

        case LEX1::END_OF_INPUT:
            DBG(std::cerr << "* " << token << std::endl);
            // EOF from current level 1 lexer so pop it and try next
            last_valid_pos = lex1().position();
            lex1_stack.pop();
            make_last_print_unoptimalizable(this);
            break;
        }
    }

    // there is no work anymore
    return {LEX2_EOF, last_valid_pos, {}}; // EOF
}

void Parser_t::error(__attribute__((unused)) const std::string &s) {
    DBG(std::cerr << "!!!! Syntax error occurred: " << s << std::endl);
}

} // namespace Parser
} // namespace Teng

