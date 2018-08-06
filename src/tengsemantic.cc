/* !don't remove! -*- C++ -*-
 *
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
 * $Id: tengsyntax.yy,v 1.14 2010-06-11 08:25:35 burlog Exp $
 *
 * DESCRIPTION
 * Teng grammar semantic actions.
 *
 * AUTHORS
 * Stepan Skrob <stepan@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-07  (burlog)
 *             Moved from syntax.yy.
 */

#include <cstdio>
#include <string>
#include <algorithm>

#include "tengyystype.h"
#include "tengerror.h"
#include "tenglogging.h"
#include "tenginstruction.h"
#include "tengconfiguration.h"
#include "tengparsercontext.h"
#include "tengparservalue.h"
#include "tengformatter.h"
#include "tengcode.h"
#include "tenglex2.h"
#include "tengaux.h"
#include "tengsyntax.hh"
#include "tengcode.h"
#include "tengyylex.h"

namespace Teng {
namespace Parser {
namespace {

/** If variable starts with underscore then warning message is appended into
 * log.
 */
void
warn_if_variable_name_is_reserved(Context_t *ctx, const Symbol_t &name) {
    // switch (name.symbol_id) {
    // case LEX2::BUILTIN_THIS:
    //     logWarning(ctx, name.pos, "The _this variable name is reserved");
    //     break;
    // case LEX2::BUILTIN_PARENT:
    //     logWarning(ctx, name.pos, "The _parent variable name is reserved");
    //     break;
    // case LEX2::IDENT:
    //     if (name.val.as_str().empty()) {
    //         logWarning(ctx, name.pos, "The variable name is empty");
    //     } else if (name.val.as_str().front() == '_') {
    //         logWarning(
    //             ctx,
    //             name.pos,
    //             "The variable names starting with underscore are reserved"
    //         );
    //     }
    //     break;
    // }
}

} // namespace

void
codeForVariable(Context_t *ctx, Symbol_t &result, const Symbol_t &name) {
    // // if identifier of variable si invalid
    // if (name.id.empty()) {
    //     auto &n = name.val.as_str();
    //     result.val = Undefined_t{};
    //     generateCode(ctx, Instruction_t::VAL, result.val);
    //     logError(ctx, name.pos, "Variable identifier '" + n + "' is invalid");
    //     return;
    // }
    //
    // // algorithm for common builtin vars
    // auto generate_builtin_var = [&] (Instruction_t::OpCode_t opcode) {
    //     // omit builtin variable name
    //     result.set_ident({name.id.begin(), std::prev(name.id.end())});
    //
    //     // try to find fragment
    //     Identifier_t id;
    //     switch (ctx->findFragment(&name.pos, result.id, id)) {
    //     case Context_t::FR::FOUND:
    //         generateCode(ctx, opcode, result.val);
    //         ctx->program->back().identifier = id;
    //         return true;
    //     default:
    //         return false;
    //     }
    // };
    //
    // // algorithm for builtin variable count
    // auto generate_builtin_count_var = [&] {
    //     // omit builtin variable name
    //     result.set_ident({name.id.begin(), std::prev(name.id.end())});
    //
    //     // // try to find fragment
    //     // Identifier_t id;
    //     // switch (ctx->findFragment(&name.pos, result.id, id, true)) {
    //     // case Context_t::FR::NOT_FOUND:
    //     //     // no-op -- we have nothing found
    //     //     return false;
    //     // case Context_t::FR::FOUND:
    //     //     generateCode(ctx, Instruction_t::FRAGCNT, result.val);
    //     //     ctx->program->back().identifier = id;
    //     //     return true;
    //     // case Context_t::FR::PARENT_FOUND:
    //     //     // parent fragment found -- this fragment is not open
    //     //     // and we must have some runtime overhead to find count of it
    //     //     generateCode(ctx, Instruction_t::NESTED_FRAGCNT, result.val);
    //     //     ctx->program->back().identifier = id;
    //         return true;
    //     // }
    // };
    //
    // // algorithm for regular vars
    // auto generate_regular_var = [&] {
    //     // Identifier_t id;
    //     // result.id = name.id;
    //     // result.val = name.val;
    //     // if (ctx->findFragmentForVar(name.pos, result.id, id)) {
    //     //     result.val = 1; // do escape
    //     //     generateCode(ctx, Instruction_t::VAR, result.val);
    //     //     ctx->program->back().identifier = id;
    //     //     return true;
    //     // }
    //     return false;
    // };
    //
    // // handle regular and builtin variables
    // switch (name.symbol_id) {
    // case LEX2::BUILTIN_FIRST:
    //     if (generate_builtin_var(Instruction_t::FRAGFIRST))
    //         return;
    //     break;
    // case LEX2::BUILTIN_INNER:
    //     if (generate_builtin_var(Instruction_t::FRAGINNER))
    //         return;
    //     break;
    // case LEX2::BUILTIN_LAST:
    //     if (generate_builtin_var(Instruction_t::FRAGLAST))
    //         return;
    //     break;
    // case LEX2::BUILTIN_INDEX:
    //     if (generate_builtin_var(Instruction_t::FRAGINDEX))
    //         return;
    //     break;
    // case LEX2::BUILTIN_COUNT:
    //     if (generate_builtin_count_var())
    //         return;
    //     break;
    // case LEX2::BUILTIN_PARENT:
    // case LEX2::BUILTIN_THIS:
    // case LEX2::SELECTOR:
    // case LEX2::IDENT:
    // case LEX2::VAR:
    //     if (generate_regular_var())
    //         return;
    //     break;
    // default:
    //     logError(
    //         ctx,
    //         name.pos,
    //         "Unexpected token: " + std::to_string(name.symbol_id)
    //     );
    //     break;
    // }
    //
    // // if variable is not found then it is undefined
    // result.val = Undefined_t{};
    // generateCode(ctx, Instruction_t::VAL, result.val);
}

void
buildLocVar(
    Context_t *ctx,
    Symbol_t &result,
    Symbol_t &name
) {
    // warn_if_variable_name_is_reserved(ctx, name);
    // result.pos = name.pos;
    // result.symbol_id = name.symbol_id;
    // // result.set_ident(ctx->fragContext.back(), std::move(name.val.as_str()));
    // std::cerr << ">>>>>>>>>>>>>>>>>>>> loc.name: " << name.val << std::endl;
}

void
buildAbsVar(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &statement
) {
    result.pos = statement.pos;
    // result.set_ident(std::move(ctx->ident));
    result.id = statement.id;
}

void popAbsVarSegment(Context_t *ctx, const Symbol_t &segment) {
    // if (ctx->ident.empty())
    //     logWarning(ctx, segment.pos, "The _parent violates the root boundary");
    // else ctx->ident.pop_back();
}

void pushAbsVarSegment(Context_t *ctx, const Symbol_t &segment) {
    // ctx->ident.push_back(std::move(segment.val.as_str()));
}

void
pushVarName(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &statement,
    const Symbol_t &name
) {
    result.pos = statement.pos;
    result.id = name.id;
    warn_if_variable_name_is_reserved(ctx, name);
    // ctx->ident.push_back(std::move(name.val.as_str()));
}

void
buildRelVar(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &statement
) {
    result.pos = statement.pos;

    // // take iter to path segments (omit the last one which is var name)
    // auto irpath = ++ctx->ident.rbegin(), erpath = ctx->ident.rend();
    //
    // // search open fragments stack in reverse order for this path
    // auto &open_fragments = ctx->fragContext.back().name;
    // auto iropen_fragment = std::search(
    //     open_fragments.rbegin(), open_fragments.rend(),
    //     irpath, erpath
    // );
    //
    // // if path has been found in open fragments then build ident and return it
    // if (iropen_fragment != open_fragments.rend()) {
    //     result.set_ident(
    //         {open_fragments.begin(), iropen_fragment.base()},
    //         ctx->ident.back()
    //     );
    //     std::cerr << ">>>>>>>>>>>>>>>>>>>> rel.name: " << result.val << std::endl;
    //     return;
    // }
    //
    //     std::cerr << ">>>>>>>>>>>>>>>>>>>> no.name: " << std::endl;
    //
    // // path doesn't match
    // std::string var;
    // for (auto &segment: ctx->ident) var += (var.empty()? "": ".") + segment;
    // logError(ctx, statement.pos, "Variable identifier '" + var + "' not found");
    // result.set_ident({});
}

void pushRelVarSegment(Context_t *ctx, const Symbol_t &segment) {
    // switch (segment.symbol_id) {
    // case LEX2::BUILTIN_THIS:
    //     logWarning(
    //         ctx,
    //         segment.pos,
    //         "Ignoring useless _this relative variable path segment"
    //     );
    //     return;
    //
    // case LEX2::BUILTIN_PARENT:
    //     logWarning(
    //         ctx,
    //         segment.pos,
    //         "Ignoring invalid _parent relative variable path segment"
    //     );
    //     return;
    //
    // default:
    //     ctx->ident.push_back(std::move(segment.val.as_str()));
    //     return;
    // }
}

void
includeFile(Context_t *ctx, const Symbol_t &incl, const Symbol_t &opts) {
    // // ensure that file option exists
    // auto iopt = opts.opt.find("file");
    // if (iopt == opts.opt.end()) {
    //     auto msg = "Cannot include a file; option 'file' is not specified";
    //     logError(ctx, incl.pos, msg);
    //     return;
    //
    // // ensure that we not go beyond include limit
    // } else if (ctx->lex1.size() >= ctx->params->getMaxIncludeDepth()) {
    //     auto msg = "Cannot include a file; template nesting level is too deep";
    //     logError(ctx, incl.pos, msg);
    //     return;
    // }
    //
    // // compile file (append compiled file at the end of current program)
    // compile_file(ctx, iopt->second);
}

void
openFormat(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &format,
    const Symbol_t &opts
) {
    // auto iopt = opts.opt.find("space");
    //
    // // ensure that space option exists and has value
    // if (iopt == opts.opt.end()) {
    //     auto msg = "Formatting block has no effect; option 'space' is missing";
    //     logError(ctx, format.pos, msg);
    //     return;
    //
    // } else if (iopt->second.empty()) {
    //     auto msg = "Formatting block has no effect; option 'space' is empty";
    //     logError(ctx, format.pos, msg);
    //     return;
    // }
    //
    // // check space option
    // result.val = int(resolveFormat(iopt->second));
    // if (result.val.integral() != Formatter_t::MODE_INVALID) {
    //     generateCode(ctx, Instruction_t::FORM, result.val);
    //     return;
    // }
    //
    // // invalid format
    // logError(
    //     ctx,
    //     format.pos,
    //     "Unsupported value '" + iopt->second + "' of 'space' formatting option"
    // );
}

void closeFormat(Context_t *ctx, const Symbol_t &format) {
    // // if was not error no block start
    // if (format.val.integral() != Formatter_t::MODE_INVALID)
    //     generateCode(ctx, Instruction_t::ENDFORM);
    // // TODO(burlog): tohle je fakt dobre? ma smysl testovat a jak?
    //
    // // do not optimize (join) print-vals across current prog end-addr
    // // ctx->lowestValPrintAddress = ctx->program->size();
}

void openFrag(Context_t *ctx, Symbol_t &result, const Symbol_t &name) {
    // // save current prog size/addr
    // result.prgsize = ctx->program->size();
    // result.val = name.val;
    //
    // // // generate code for frag
    // // Identifier_t id;
    // // if (ctx->pushFragment(name.pos, name.id, id)) {
    // //     generateCode(ctx, Instruction_t::FRAG, name.val);
    // //     ctx->program->back().identifier = id;
    // //     result.id = name.id;
    // //
    // // // error in fragment, erase identifier to mark error
    // // } else result.id.clear();
}

void closeFrag(Context_t *ctx, const Symbol_t &name) {
    // // pop previous fragment or remove invalid code
    // if (name.id.empty()) {
    //     ctx->cropCode(name.prgsize);
    // } else {
    //     generateCode(ctx, Instruction_t::ENDFRAG);
    //     ctx->popFragment(name.prgsize);
    // }
}

void
setVariable(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &statement,
    const Symbol_t &name
) {
    // // try find variable
    // if (!name.id.empty()) {
    //     Identifier_t id;
    //     result.id = name.id;
    //     result.val = name.val;
    //     // if (ctx->findFragmentForVar(result.pos, name.id, id)) {
    //     //     // fully qualified variable identifier is in -> $$.val.stringValue
    //     //     generateCode(ctx, Instruction_t::SET, result.val);
    //     //     ctx->program->back().identifier = id;
    //     //     return;
    //     // }
    //
    // } else {
    //     // invalid var name
    //     static auto *msg = "Invalid variable identifier; variable won't be set";
    //     logError(ctx, name.pos, msg);
    // }
    //
    // // reslt
    // eraseCodeFrom(ctx, statement.prgsize);
}

void buildIfEndJump(Context_t *ctx, const Symbol_t &, const Symbol_t &, const Symbol_t &) {
    throw std::runtime_error("udelej to debile");
        // {
        //     // calculate jump offset
        //     // add +1 for end-jump if some else(if) section was generated
        //     (*ctx->program)[$4.prgsize].value.integerValue =
        //             $6.prgsize - $4.prgsize - 1
        //             + ($6.prgsize != ctx->program->size());
        //     // update all end-jumps with proper address
        //     Symbol_t::AddressList_t::const_iterator i;
        //     for (i = $7.addr.begin(); i != $7.addr.end(); ++i) {
        //         (*ctx->program)[*i].value.integerValue =
        //                 ctx->program->size() - *i - 1;
        //     }
        //     // do not optimize (join) print-vals across current prog end-addr
        //     ctx->lowestValPrintAddress = ctx->program->size();
        // }
}

void buildElseEndJump(Context_t *ctx, Symbol_t &, const Symbol_t &, const Symbol_t &, const Symbol_t &, const Symbol_t &) {
    throw std::runtime_error("udelej to debile");
        //     // calculate jump offset
        //     // add +1 for end-jump if some else(if) section was generated
        //     (*ctx->program)[$5.prgsize].value.integerValue =
        //             $7.prgsize - $5.prgsize - 1
        //             + ($7.prgsize != ctx->program->size());
        //     // preserve end-jumps info
        //     $$.addr = $8.addr;
        //     $$.addr.push_back($2.prgsize);
        // }
}

std::size_t
finalizeBinOp(Context_t *ctx, const Symbol_t &lhs, const Symbol_t &rhs) {
    // (*ctx->program)[rhs.prgsize].value = ctx->program->size() - rhs.prgsize - 1;
    // optimizeExpression(ctx, lhs.prgsize);
    // // ctx->lowestValPrintAddress = ctx->program->size();
    // return lhs.prgsize;
    return {};
}

std::size_t buildTernOp(Context_t *ctx, const Symbol_t &true_expr) {
    // // correct conditional jump offset (relative addr) +1=JMP
    // (*ctx->program)[true_expr.prgsize].value
    //     = ctx->program->size() - true_expr.prgsize - 1 + 1;
    //
    // // jump stored later
    // auto prgsize = ctx->program->size();
    // generateCode(ctx, Instruction_t::JMP);
    // return prgsize;
    return {};
}

std::size_t
finalizeTernOp(Context_t *ctx, const Symbol_t &lhs, const Symbol_t &rhs) {
    // TODO(burlog):
    return finalizeBinOp(ctx, lhs, rhs);
}

std::size_t
generateUndefined(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &statement
) {
    // auto tmp = statement.prgsize;
    // eraseCodeFrom(ctx, statement.prgsize);
    // result.val = Undefined_t{};
    // result.prgsize = statement.prgsize;
    // generateCode(ctx, Instruction_t::VAL, result.val);
    // return tmp;
    return {};
}

Value_t openCType(Context_t *ctx, const Symbol_t &) {
    throw std::runtime_error("udelej to debile");
        //     // reset val
        //     $$.val = ParserValue_t();
        //     $$.val.integerValue = -1;
        //
        //     // get content type descriptor for given type
        //     const ContentType_t::Descriptor_t *ct
        //         = ContentType_t::findContentType($2.val.stringValue,
        //                                          ctx->program->getErrors(),
        //                                          $2.pos, true);
        //
        //     if (ct) {
        //         $$.val.integerValue = ct->index;
        //         CODE_VAL(CTYPE, $$.val);
        //     }
        // }
    return Value_t();
}

void closeCType(Context_t *ctx, const Symbol_t &) {
    throw std::runtime_error("udelej to debile");
        // {
        //     // if was not error no block start
        //     if ($4.val.integerValue >= 0)
        //         CODE(ENDCTYPE); //generate code
        //     // no print-values join below following address
        //     ctx->lowestValPrintAddress = ctx->program->size();
        // }
}

void openRepeat(Context_t *ctx, const Symbol_t &) {
    throw std::runtime_error("udelej to debile");
        // {
        //     // get address of referenced fragment
        //     Identifier_t id;
        //     int address = ctx->getFragmentAddress($2.pos, $2.id,
        //                                               $2.val.stringValue, id);
        //     if (address >= 0) {
        //         // set offset
        //         $2.val.integerValue = address - ctx->program->size();
        //
        //         // generate instruction
        //         CODE_VAL(REPEATFRAG, $2.val);
        //
        //         // set identifier
        //         ctx->program->back().identifier = id;
        //
        //         // do not optimize (join) print-vals across current
        //         // prog end-addr
        //         ctx->lowestValPrintAddress = ctx->program->size();
        //     }
        // }
}

void generateRepr(Context_t *ctx) {
    generateCode(ctx, Instruction_t::REPR);
}

void generateRepr(Context_t *ctx, const Symbol_t &op) {
    // switch (op.symbol_id) {
    // case LEX2::JSONIFY:
    //     generateCode(ctx, Instruction_t::REPR_JSONIFY);
    //     break;
    // case LEX2::COUNT:
    //     generateCode(ctx, Instruction_t::REPR_COUNT);
    //     break;
    // case LEX2::TYPE:
    //     generateCode(ctx, Instruction_t::REPR_TYPE);
    //     break;
    // case LEX2::DEFINED:
    //     generateCode(ctx, Instruction_t::REPR_DEFINED);
    //     break;
    // case LEX2::EXISTS:
    //     generateCode(ctx, Instruction_t::REPR_EXISTS);
    //     optimizeExpression(ctx, op.prgsize);
    //     break;
    // case LEX2::ISEMPTY:
    //     generateCode(ctx, Instruction_t::REPR_ISEMPTY);
    //     break;
    // default:
    //     throw std::runtime_error(__PRETTY_FUNCTION__);
    // }
}

void
generateDictLookup(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &ident
) {
    // auto generate_code = [&] (const std::string &value) {
    //     result.val = value;
    //     result.prgsize = ctx->program->size();
    //     generateCode(ctx, Instruction_t::VAL, result.val);
    // };

    // find item in dictionary
    // if (auto *item = ctx->langDictionary->lookup(ident.val.as_str()))
    //     return generate_code(*item);
    //
    // // find item in param/config dictionary
    // if (auto *item = ctx->paramDictionary->lookup(ident.val.as_str()))
    //     return generate_code(*item);

    // // use ident as result value
    // auto msg = "Dictionary item '" + ident.val.as_str() + "' was not found";
    // logError(ctx, ident.pos, msg);
    // generate_code(ident.val.as_str());
}

void
finalizeCaseOp(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &case_start,
    const Symbol_t &case_options
) {
    // // update all end-jumps with proper address
    // for (auto addr: case_options.addr)
    //     (*ctx->program)[addr].value = ctx->program->size() - addr - 1;
    //
    // // generateCode removing previously pushed value
    // generateCode(ctx, Instruction_t::POP);
    //
    // // try to optimize last expression
    // result.prgsize = case_start.prgsize;
    // optimizeExpression(ctx, result.prgsize);
    //
    // // do not optimize (join) print-vals across current prog end-addr
    // // ctx->lowestValPrintAddress = ctx->program->size();
}

void
finalizeCaseOptionsOp(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &matching_branch_start,
    const Symbol_t &matching_branch_end,
    const Symbol_t &case_option
) {
    // // propagate end-jump info
    // result.addr = case_option.addr;
    //
    // // add +1 for end-jump if some other case option was generated
    // auto skip_end_jump = (matching_branch_end.prgsize != ctx->program->size());
    //
    // // calculate jump offset
    // (*ctx->program)[matching_branch_start.prgsize].value
    //     = matching_branch_end.prgsize - matching_branch_start.prgsize
    //     - 1 + skip_end_jump;
}

void generateCaseLiteral(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &case_value
) {
    // // compare to stack top
    // result.val = 0;
    // generateCode(ctx, Instruction_t::STACK, result.val);
    // generateCode(ctx, Instruction_t::VAL, case_value.val);
    //
    // // choose proper comparison operator
    // auto opcode = case_value.val.is_string()
    //     ? Instruction_t::STREQ
    //     : Instruction_t::NUMEQ;
    // generateCode(ctx, opcode);
}

void generateCaseValues(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &case_value
) {
    // // join two case values with OR
    // result.prgsize = ctx->program->size();
    // generateCode(ctx, Instruction_t::OR);
    //
    // // compare to stack top
    // result.val = 0;
    // generateCode(ctx, Instruction_t::STACK, result.val);
    // generateCode(ctx, Instruction_t::VAL, case_value.val);
    //
    // // choose proper comparison operator
    // auto opcode = case_value.val.is_string()
    //     ? Instruction_t::STREQ
    //     : Instruction_t::NUMEQ;
    // generateCode(ctx, opcode);
    //
    // // calculate OR-jump offset
    // (*ctx->program)[result.prgsize].value
    //     = ctx->program->size() - result.prgsize - 1;
}

void
generateQueryInstr(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &op,
    const Symbol_t &name
) {
    // // remove mark of log supressing if it is present
    // // if (!ctx->program->empty())
    // //     if (ctx->program->back().operation == Instruction_t::SUPRESS_LOG)
    // //         ctx->program->pop_back();
    // //
    // // prepare log for invalid query
    // auto log_invalid_id = [&] {
    //     switch (op.symbol_id) {
    //     case LEX2::DEFINED:
    //         logError(ctx, name.pos, "Invalid identifier in defined() operator");
    //         break;
    //     case LEX2::EXISTS:
    //         logError(ctx, name.pos, "Invalid identifier in exists() operator");
    //         break;
    //     case LEX2::ISEMPTY:
    //         logError(ctx, name.pos, "Invalid identifier in isempty() operator");
    //         break;
    //     }
    // };
    //
    // // if identifier is invalid then use undefined value instruction instead
    // // of defined instruction
    // result.prgsize = ctx->program->size();
    // std::cerr << "aaaaaaaaaaaaa" << std::endl;
    // if (name.id.empty()) {
    //     std::cerr << "eeee" << std::endl;
    //     // the '$' character is magic value that is set to name when identifier
    //     // starts with '$' character that considered as invalid syntax
    //     static auto *msg = "Variable identifier must not start with '$'";
    //     if (name.val.str() == "$") logError(ctx, name.pos, msg);
    //     log_invalid_id();
    //     result.id = name.id;
    //     result.val = Undefined_t{};
    //     eraseCodeFrom(ctx, op.prgsize);
    //     generateCode(ctx, Instruction_t::VAL, result.val);
    //     return;
    // }
    //
    // // handle regular and builtin variables appropriately
    // bool mustBeOpen = false;
    // switch (name.symbol_id) {
    // case LEX2::BUILTIN_FIRST:
    // case LEX2::BUILTIN_INNER:
    // case LEX2::BUILTIN_LAST:
    // case LEX2::BUILTIN_INDEX:
    //     result.set_ident({name.id.begin(), std::prev(name.id.end())});
    //     mustBeOpen = true;
    //     break;
    // case LEX2::BUILTIN_COUNT:
    //     result.set_ident({name.id.begin(), std::prev(name.id.end())});
    //     break;
    // case LEX2::IDENT:
    // case LEX2::BUILTIN_THIS:
    // case LEX2::BUILTIN_PARENT:
    //     result.id = name.id;
    //     result.val = name.val;
    //     break;
    // }
    //
    // // converts token id to opcode
    // auto choose_opcode = [&] (int symbol_id) {
    //     switch (symbol_id) {
    //     case LEX2::DEFINED:
    //         return Instruction_t::DEFINED;
    //     case LEX2::EXISTS:
    //         return Instruction_t::EXISTS;
    //     case LEX2::ISEMPTY:
    //         return Instruction_t::ISEMPTY;
    //     }
    //     throw std::runtime_error(__PRETTY_FUNCTION__);
    // };
    //
    // // prepare callback for implementation
    // Identifier_t id;
    // auto make_query = [&] (Instruction_t::OpCode_t opcode) {
    //     generateCode(ctx, opcode, result.val);
    //     ctx->program->back().identifier = id;
    //     optimizeExpression(ctx, op.prgsize);
    // };
    // auto make_value = [&] (bool value) {
    //     result.val = value;
    //     generateCode(ctx, Instruction_t::VAL, result.val);
    // };
    //
    // // generate defined/exists/isempty instruction if might be defined
    // // switch (ctx->exists(name.pos, result.id, id, mustBeOpen)) {
    // // case Context_t::ER::FOUND:
    // //     if (op.symbol_id == LEX2::DEFINED)
    // //         make_query(Instruction_t::DEFINED);
    // //     else make_value(true);
    // //     break;
    // // case Context_t::ER::RUNTIME:
    // //     make_query(choose_opcode(op.symbol_id));
    // //     break;
    // // case Context_t::ER::NOT_FOUND:
    // //     make_value(false);
    // //     break;
    // // }
}

void
prepareQueryInstr(
    Context_t *ctx,
    Symbol_t &result,
    const Symbol_t &name,
    const char *value
) {
    // generateCode(ctx, Instruction_t::SUPRESS_LOG);
    // result.val = value;
    // result.symbol_id = name.symbol_id;
    // result.prgsize = name.prgsize;
}

std::size_t
generateFunction(
    Context_t *ctx,
    const Symbol_t &name,
    const Symbol_t &lparen,
    const Symbol_t &args
) {
    // generateFunctionCall(ctx, name.val.as_str(), args.val.as_int());
    // optimizeExpression(ctx, lparen.prgsize);
    // return lparen.prgsize;
    return {};
}

} // namespace Parser
} // namespace Teng

