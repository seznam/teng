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

#include "tenglex1.h"
#include "tengprogram.h"
#include "tengplatform.h"
#include "tenglogging.h"
#include "tengsyntax.hh"
#include "tengcontenttype.h"
#include "tengconfiguration.h"
#include "tengparsercontext.h"

namespace Teng {
namespace {

// forwards
using flex_string_value_t = Parser::flex_string_value_t;
using flex_string_view_t = Parser::flex_string_view_t;

/** Compose absolute filename.
 */
std::string absfile(const std::string &fs_root, const std::string &filename) {
    return !fs_root.empty() && !filename.empty() && !ISROOT(filename)
         ? fs_root + "/" + filename
         : filename;
}

/** Reads file content into string.
 */
flex_string_value_t
read_file(
    Parser::Context_t *ctx,
    const Pos_t *include_pos,
    const std::string &filename
) {
    // open file
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        Pos_t pos = include_pos? *include_pos: Pos_t();
        logError(ctx, pos, "Cannot open input file '" + filename + "'");
        return flex_string_value_t(0);
    }

    // read whole file in one buffer
    file.seekg(0, std::ios::end);
    std::size_t size = file.tellg();
    flex_string_value_t source_code(size);
    file.seekg(0, std::ios::beg);
    file.read(source_code.data(), size);
    return source_code;
}

void compile(Parser::Context_t *ctx) {
    // parse input by bison generated parser and compile program
    Parser::Parser_t parser(ctx);
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

} // namespace

std::unique_ptr<Program_t>
compile_file(
    const Dictionary_t *dict,
    const Configuration_t *params,
    const std::string &fs_root,
    const std::string &filename
) {
    Parser::Context_t ctx(dict, params, fs_root);
    ctx.load_file(filename);
    compile(&ctx);
    return std::move(ctx.program);
}

std::unique_ptr<Program_t>
compile_string(
    const Dictionary_t *dict,
    const Configuration_t *params,
    const std::string &fs_root,
    const std::string &source
) {
    Parser::Context_t ctx(dict, params, fs_root);
    ctx.load_source(source);
    compile(&ctx);
    return std::move(ctx.program);
}

namespace Parser {

Context_t::Context_t(
    const Dictionary_t *dict,
    const Configuration_t *params,
    const std::string &fs_root
): program(std::make_unique<Program_t>()), dict(dict), params(params),
   fs_root(fs_root), source_codes(), lex1_stack(),
   lex2_value(program->getErrors()), coproc_err(),
   coproc(coproc_err, *program, *dict, *params),
   open_frames(*program), var_sym(), opts_sym(),
   error_occurred(false), unexpected_token{LEX2::INVALID, {}, {}},
   expr_start_point{{}, -1}, branch_start_addrs(), optimization_points()
{}
// TODO(burlog): zvazit predalokaci stacku na nejakou "velikost" v 2.0 byla na 100

Context_t::~Context_t() = default;

void
Context_t::load_file(const std::string &filename, const Pos_t *include_pos) {
    // load source code from file
    std::string path = absfile(fs_root, filename);
    source_codes.push_back(read_file(this, include_pos, path));
    auto &source_code = source_codes.back();

    // register source into program sources list
    // the registration routine returns program lifetime durable pointer to
    // filename that can be used as pointer to filename in Pos_t instancies
    auto *source_path = program->addSource(path, {}).first;

    // create the level 1 lexer for given source code
    lex1_stack.emplace(flex_string_view_t(source_code), source_path);
}

void Context_t::load_source(const std::string &source) {
    // copy source code to flex parsable string
    flex_string_value_t source_code(source.size());
    std::copy(source.begin(), source.end(), source_code.data());
    source_codes.push_back(std::move(source_code));

    // create the level 1 lexer for given source code
    lex1_stack.emplace(flex_string_view_t(source_codes.back()));
}

#ifdef DEBUG
#define DBG(...) __VA_ARGS__
#else /* DEBUG */
#define DBG()
#endif /* DEBUG */

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
            case LEX2::INVALID:
                DBG(std::cerr << "---- " << token << std::endl);
                logError(this, token.pos, "Invalid lexical element");
                continue;
            case LEX2_EOF:
                DBG(std::cerr << "---- " << token << std::endl);
                lex2().finish_scanning();
                break;
            }
        }

        // get next L1 token and process it
        using LEX1 = Lex1_t::LEX1;
        bool accept_short_directives = params->isShortTagEnabled();
        switch (auto token = lex1().next(accept_short_directives)) {
        case LEX1::TENG: case LEX1::EXPR:
        case LEX1::DICT: case LEX1::TENG_SHORT:
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
            break;
        }
    }

    // there is no work anymore
    return {LEX2_EOF, last_valid_pos, {}}; // EOF
}

#undef DBG

void Parser_t::error(const std::string &) {
    /* ignore all errors from bison parser since they are useless */
}

} // namespace Parser
} // namespace Teng

