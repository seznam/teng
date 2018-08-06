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
#include <fstream>

#include "tenglex1.h"
#include "tengprogram.h"
#include "tengplatform.h"
#include "tenglogging.h"
#include "tengsyntax.hh"
#include "tengcontenttype.h"
#include "tengparsercontext.h"

namespace Teng {
namespace {

// forwards
using flex_string_value_t = Parser::flex_string_value_t;

/** Compose absolute filename.
 */
std::string absfile(const std::string &fs_root, const std::string &filename) {
    return !fs_root.empty() && !filename.empty() && !ISROOT(filename)
         ? fs_root + "/" + filename
         : filename;
}

/** Concats name segments into one string.
 */
std::string fullname(const std::vector<std::string> &id) {
    std::string fn;
    for (auto &segment: id)
        fn.append({'.'}).append(segment);
    return fn;
}

/** Reads file content into string.
 */
flex_string_value_t
read_file(Parser::Context_t *ctx, const std::string &filename) {
    // open file
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        logError(ctx, "Cannot open input file '" + filename + "'");
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

void
compile(
    Parser::Context_t *ctx,
    flex_string_value_t &source_code,
    const std::string &path = {},
    const Pos_t &include_pos = {}
) {
    // register source into program sources list
    // the registration routine returns program lifetime durable pointer to
    // filename that can be used as pointer to filename in Pos_t instancies
    auto *source_path = path.empty()
        ? nullptr
        : ctx->program->addSource(path, include_pos).first;

    // create level 1 lexer for given file (the level 2 lexer should be off)
    ctx->lex1_stack.emplace(source_code, source_path);

    // TODO(burlog): vvvvv fragContext musi uz tak vzniknout,
    // lowestValPrintAddress bude udelany pomoci spec instr, evaluator uvidime
    // eval processor
    // Processor_t evaluator(*err, *ctx->program, *ctx->dict, *ctx->params);
    // // create first (empty) fragment context
    // fragContext.push_back(Context_t::FragmentContext_t());
    // fragContext.back().reserve(100);
    // // create first (empty) fragment context
    // fragContext.push_back(Context_t::FragmentContext_t());
    // fragContext.back().reserve(100);
    // no print-values join below following address
    // ctx.lowestValPrintAddress = 0;
    // no print-values join below following address
    // lowestValPrintAddress = 0;

    // parse input by bison generated parser and compile program
    Parser::parser parser(ctx);
    if (parser.parse() != 0) {
        ctx->program->clear();
        logFatal(ctx, "Parser has crashed: syntax error");
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
    std::string path = absfile(ctx.fs_root, filename);
    ctx.source_codes.push_back(read_file(&ctx, path));
    compile(&ctx, ctx.source_codes.back(), path);
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
    flex_string_value_t source_code(source.size());
    std::copy(source.begin(), source.end(), source_code.data());
    ctx.source_codes.push_back(std::move(source_code));
    compile(&ctx, ctx.source_codes.back());
    return std::move(ctx.program);
}

namespace Parser {

Context_t::Context_t(
    const Dictionary_t *dict,
    const Configuration_t *params,
    const std::string &fs_root
): program(std::make_unique<Program_t>()), dict(dict), params(params),
   fs_root(fs_root), source_codes(), lex1_stack(), lex2(program->getErrors()),
   coproc_err(), coproc(coproc_err, *program, *dict, *params), ident(),
   frag_ctxs()
{}

Context_t::~Context_t() {}

void compile_file(Context_t *ctx, const std::string &filename) {
    if (ctx->lex2.in_use()) ctx->lex2.finish_scanning();
    std::string path = absfile(ctx->fs_root, filename);
    ctx->source_codes.push_back(read_file(ctx, path));
    compile(ctx, ctx->source_codes.back(), path, ctx->position());
}

void parser::error(const std::string &msg) {
#if YYDEBUG
    // if debug enabled
    if (debug_level())
        debug_stream() << "\n*** " << msg << " ***\n" << std::endl;
#endif /* YYDEBUG */
}

// // TODO(burlog): vvvv these should be removed?
// // TODO(burlog): lokalni promena?
// Processor_t *evalProcessor; //!< Processor unit used for evaluation of constant expressions
//
// // TODO(burlog): zase do lexeru a vracet jako funkci?
// Position_t position;                     //!< current position in input stream (updated by parser)
//
// // TODO(burlog): should go to lexX
// Position_t lex1Pos; //!< start pos of current lex1 element
// Position_t lex2Pos; //!< actual position in lex2 stream
//
// // TODO(burlog):  should go to lex1?
// std::stack<int> sourceIndex;             //!< Source index relevant to the currently processed source by lex1
//
// unsigned int lowestValPrintAddress;      //!< Lowest possible address, at which the sequence of VAL, PRINT, VAL, PRINT, ... instructions can be joined into the single VAL, PRINT pair.
// std::string lastErrorMessage; //!< last error message of the parser
// // TODO(burlog): there is no helpfull message from bison - it say's "syntax
// // error" for all cases (can be turned to more descriptive mode, but it can
// // be wrong)

} // namespace Parser
} // namespace Teng

