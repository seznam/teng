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
 * $Id: tengprocessor.cc,v 1.15 2010-06-11 07:46:26 burlog Exp $
 *
 * DESCRIPTION
 * Teng processor. Executes programs.
 *
 * AUTHORS
 * Jan Nemec <jan.nemec@firma.seznam.cz>
 * Vaclav Blazek <blazek@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-22  (jan)
 *             Created.
 * 2004-05-30  (vasek)
 *             Revised processor source code.
 * 2004-09-19  (vasek)
 *             Some minor fixes.
 * 2018-07-07  (burlog)
 *             Cleared.
 */

#include <sys/types.h>
#include <unistd.h>

#include "tenginstructionpointer.h"
#include "tengprocessorcontext.h"
#include "tengprocessorother.h"
#include "tengprocessordebug.h"
#include "tengprocessorfrag.h"
#include "tengprocessorops.h"
#include "tengprocessor.h"

#ifdef DEBUG
#define DBG(...) __VA_ARGS__
#else /* DEBUG */
#define DBG(...)
#endif /* DEBUG */

namespace Teng {
namespace {

template <typename Ctx_t>
void dump_program(Ctx_t *ctx, const SubProgram_t &program, std::ostream &out) {
    bool optimization = std::is_same<std::decay_t<Ctx_t>, EvalCtx_t>::value;

    // top banner
    out << std::endl << "## PROGRAM -- "
        << (optimization? "optimization": "run")
        << std::endl;

    // program
    for (int i = program.start; i < program.end; ++i)
        out << std::setw(3) << std::setfill('0')
            << std::noshowpos << i << " " << program[i]
            << std::endl;

    // bottom banner
    out << "## END" << std::endl << std::endl
        << "## EXECUTION -- "
        << (optimization? "optimization": "run")
        << std::endl;
}

template <typename Ctx_t>
void dump_instr(
    Ctx_t *ctx,
    const SubProgram_t &program,
    InstructionPointer_t &ip,
    const std::vector<Value_t> &stack,
    const std::vector<Value_t> &prg_stack,
    std::ostream &out
) {
    std::ostringstream os;

    // instruction
    os << std::setw(3) << std::setfill('0')
       << std::noshowpos << *ip << ' ' << program[*ip];

    // stack
    int64_t len = std::max(0l, 75l - static_cast<int64_t>(os.str().size()));
    os << std::string(len, ' ') << " | ";
    for (auto &item: stack)
        os << '[' << item << ']';

    // prg_stack
    len = std::max(0l, 125l - static_cast<int64_t>(os.str().size()));
    os << std::string(len, ' ') << " | ";
    for (auto &item: prg_stack)
        os << '[' << item << ']';

    // done
    out << os.str() << std::endl;
}

/** The core of Teng template engine. Renders the template.
 */
template <typename Ctx_t>
bool
process(Ctx_t *ctx, std::vector<Value_t> &stack, const SubProgram_t &program) {
    std::vector<FragmentList_t> error_list;
    std::vector<Value_t> prg_stack;
    prg_stack.reserve(128);
    DBG(dump_program(ctx, program, std::cerr));

    // syntactic sugar
    auto push = [&] (auto &&value) {
        stack.push_back(std::forward<decltype(value)>(value));
    };
    auto top = [&] () -> Value_t &{
        if (stack.empty())
            throw std::runtime_error("program stack underflow");
        return stack.back();
    };

    // exec program on stack-based processor
    GetArg_t get_arg(stack);
    for (InstructionPointer_t ip(program); ip < program.end; ++ip) try {
        ctx->instr = &program[*ip];
        DBG(dump_instr(ctx, program, ip, stack, prg_stack, std::cerr));

        switch (ctx->instr->opcode()) {
        case OPCODE::NOOP:
            break;

        case OPCODE::DEBUG_FRAG:
            exec::debug_frag(ctx);
            break;

        case OPCODE::BYTECODE_FRAG:
            exec::bytecode_frag(ctx);
            break;

        case OPCODE::PRINT:
            exec::print(ctx, get_arg);
            break;

        case OPCODE::SET:
            exec::set_var(ctx, get_arg);
            break;

        case OPCODE::VAL:
            push(exec::val(ctx));
            break;

        case OPCODE::DICT:
            push(exec::dict(ctx, get_arg));
            break;

        case OPCODE::VAR:
            push(exec::var(ctx, program[ip + 1].opcode() == OPCODE::PRINT));
            break;

        case OPCODE::PRG_STACK_PUSH:
            exec::prg_stack_push(prg_stack, get_arg);
            break;

        case OPCODE::PRG_STACK_POP:
            exec::prg_stack_pop(prg_stack);
            break;

        case OPCODE::PRG_STACK_AT:
            push(exec::prg_stack_at(ctx, prg_stack));
            break;

        case OPCODE::BIT_OR:
            push(exec::numop(ctx, get_arg, std::bit_or<int64_t>()));
            break;

        case OPCODE::BIT_XOR:
            push(exec::numop(ctx, get_arg, std::bit_xor<int64_t>()));
            break;

        case OPCODE::BIT_AND:
            push(exec::numop(ctx, get_arg, std::bit_and<int64_t>()));
            break;

        case OPCODE::UNARY_PLUS:
            push(exec::unary_plus(ctx, get_arg));
            break;

        case OPCODE::UNARY_MINUS:
            push(exec::unary_minus(ctx, get_arg));
            break;

        case OPCODE::PLUS:
            push(exec::strnumop(ctx, get_arg, std::plus<>()));
            break;

        case OPCODE::MINUS:
            push(exec::numop(ctx, get_arg, std::minus<>()));
            break;

        case OPCODE::MUL:
            push(exec::numop(ctx, get_arg, std::multiplies<>()));
            break;

        case OPCODE::DIV:
            push(exec::numop(ctx, get_arg, std::divides<>()));
            break;

        case OPCODE::MOD:
            push(exec::numop(ctx, get_arg, std::modulus<int64_t>()));
            break;

        case OPCODE::EQ:
            push(exec::strnumop(ctx, get_arg, std::equal_to<>()));
            break;

        case OPCODE::NE:
            push(exec::strnumop(ctx, get_arg, std::not_equal_to<>()));
            break;

        case OPCODE::GE:
            push(exec::strnumop(ctx, get_arg, std::greater_equal<>()));
            break;

        case OPCODE::GT:
            push(exec::strnumop(ctx, get_arg, std::greater<>()));
            break;

        case OPCODE::LE:
            push(exec::strnumop(ctx, get_arg, std::less_equal<>()));
            break;

        case OPCODE::LT:
            push(exec::strnumop(ctx, get_arg, std::less<>()));
            break;

        case OPCODE::CONCAT:
            push(exec::strop(ctx, get_arg, std::plus<>()));
            break;

        case OPCODE::STR_EQ:
            push(exec::strop(ctx, get_arg, std::equal_to<>()));
            break;

        case OPCODE::STR_NE:
            push(exec::strop(ctx, get_arg, std::not_equal_to<>()));
            break;

        case OPCODE::REPEAT:
            push(exec::repeat_string(ctx, get_arg));
            break;

        case OPCODE::NOT:
            push(exec::logic_not(ctx, get_arg));
            break;

        case OPCODE::BIT_NOT:
            push(exec::bit_not(ctx, get_arg));
            break;

        case OPCODE::MATCH_REGEX:
            push(exec::regex_match(ctx, get_arg));
            break;

        case OPCODE::FUNC:
            push(exec::func(ctx, get_arg));
            break;

        case OPCODE::AND:
            if (top()) stack.pop_back();
            else ip += ctx->instr->template as<And_t>().addr_offset;
            break;

        case OPCODE::OR:
            if (!top()) stack.pop_back();
            else ip += ctx->instr->template as<Or_t>().addr_offset;
            break;

        case OPCODE::JMP_IF_NOT:
            if (!get_arg())
                ip += ctx->instr->template as<JmpIfNot_t>().addr_offset;
            break;

        case OPCODE::JMP:
            ip += ctx->instr->template as<Jmp_t>().addr_offset;
            break;

        case OPCODE::OPEN_FORMAT:
            exec::push_formatter(ctx);
            break;

        case OPCODE::CLOSE_FORMAT:
            exec::pop_formatter(ctx);
            break;

        case OPCODE::OPEN_FRAG:
            if (auto shift = exec::open_frag(ctx))
                ip += shift;
            break;

        case OPCODE::OPEN_ERROR_FRAG:
            if (auto shift = exec::open_error_frag(ctx))
                ip += shift;
            break;

        case OPCODE::CLOSE_FRAG:
            if (auto shift = exec::close_frag(ctx))
                ip += shift;
            break;

        case OPCODE::OPEN_FRAME:
            exec::open_frame(ctx);
            break;

        case OPCODE::CLOSE_FRAME:
            exec::close_frame(ctx);
            break;

        case OPCODE::OPEN_CTYPE:
            exec::push_escaper(ctx);
            break;

        case OPCODE::CLOSE_CTYPE:
            exec::pop_escaper(ctx);
            break;

        case OPCODE::PUSH_FRAG_COUNT:
            push(exec::frag_count(ctx));
            break;

        case OPCODE::PUSH_FRAG_INDEX:
            push(exec::frag_index(ctx));
            break;

        case OPCODE::PUSH_FRAG_FIRST:
            push(exec::is_first_frag(ctx));
            break;

        case OPCODE::PUSH_FRAG_LAST:
            push(exec::is_last_frag(ctx));
            break;

        case OPCODE::PUSH_FRAG_INNER:
            push(exec::is_inner_frag(ctx));
            break;

        case OPCODE::PUSH_VAL_COUNT:
            push(exec::frag_count(ctx, get_arg));
            break;

        case OPCODE::PUSH_VAL_INDEX:
            push(exec::frag_index(ctx, get_arg));
            break;

        case OPCODE::PUSH_VAL_FIRST:
            push(exec::is_first_frag(ctx, get_arg));
            break;

        case OPCODE::PUSH_VAL_LAST:
            push(exec::is_last_frag(ctx, get_arg));
            break;

        case OPCODE::PUSH_VAL_INNER:
            push(exec::is_inner_frag(ctx, get_arg));
            break;

        case OPCODE::PUSH_FRAG:
            push(exec::push_frag(ctx));
            break;

        case OPCODE::PUSH_ROOT_FRAG:
            push(exec::push_root_frag(ctx));
            break;

        case OPCODE::PUSH_THIS_FRAG:
            push(exec::push_this_frag(ctx));
            break;

        case OPCODE::PUSH_ERROR_FRAG:
            push(exec::push_error_frag(ctx, get_arg));
            break;

        case OPCODE::PUSH_ATTR_AT:
            push(exec::push_attr_at(ctx, get_arg));
            break;

        case OPCODE::POP_ATTR:
            push(exec::pop_attr(ctx, get_arg));
            break;

        case OPCODE::PUSH_ATTR:
            push(exec::push_attr(ctx, get_arg));
            break;

        case OPCODE::REPR:
            push(exec::repr(ctx, get_arg));
            break;

        case OPCODE::QUERY_REPR:
            push(exec::query_repr(ctx, get_arg));
            break;

        case OPCODE::QUERY_COUNT:
            push(exec::query_count(ctx, get_arg));
            break;

        case OPCODE::QUERY_TYPE:
            push(exec::query_type(ctx, get_arg));
            break;

        case OPCODE::QUERY_DEFINED:
            push(exec::query_defined(ctx, get_arg));
            break;

        case OPCODE::QUERY_EXISTS:
            push(exec::query_exists(ctx, get_arg));
            break;

        case OPCODE::QUERY_ISEMPTY:
            push(exec::query_isempty(ctx, get_arg));
            break;

        case OPCODE::LOG_SUPPRESS:
            ++ctx->log_suppressed;
            break;

        case OPCODE::HALT:
            break;
        }

    } catch (const runtime_ctx_needed_t &) {
        if (std::is_same<std::decay_t<Ctx_t>, RunCtx_t>::value)
            throw std::runtime_error("runtime ctx of runtime ctx requested");
        DBG(std::cerr << "## END\n" << std::endl);
        return false;

    } catch (const runtime_functx_needed_t &) {
        if (std::is_same<std::decay_t<Ctx_t>, RunCtx_t>::value)
            throw std::runtime_error("runtime ctx of runtime ctx requested");
        DBG(std::cerr << "## END\n" << std::endl);
        return false;

    } catch (const std::exception &e) {
        DBG(std::cerr << "## END (" << e.what() << ")\n" << std::endl);
        ctx->log_suppressed = 0;
        logFatal(*ctx, e.what());
        return false;
    }

    // warn about relicts on value and program stack
    if (!prg_stack.empty())
        logError(*ctx, "Program stack is not empty");
    if (!stack.empty())
        logError(*ctx, "Value stack is not empty");
    DBG(std::cerr << "## END\n" << std::endl);
    return true;
}

int logErrors(const ContentType_t *ct, Writer_t &writer, Error_t &err) {
    if (!err) return 0;
    bool useLineComment = false;

    if (ct->blockComment.first.empty()) {
        useLineComment = true;
        if (!ct->lineComment.empty())
            writer.write(ct->lineComment + ' ');

    } else {
        if (!ct->blockComment.first.empty())
            writer.write(ct->blockComment.first + ' ');
    }

    writer.write("Error log:\n");
    for (auto &errorEntry: err.getEntries()) {
        if (useLineComment && !ct->lineComment.empty())
            writer.write(ct->lineComment + ' ');
        writer.write(errorEntry.getLogLine());
    }

    if (!useLineComment)
        writer.write(ct->blockComment.second + '\n');

    return 0;
}

} // namespace

Processor_t::Processor_t(
    Error_t &err,
    const Program_t &program,
    const Dictionary_t &dict,
    const Configuration_t &params,
    const string_view_t &encoding,
    const string_view_t &contentType
): err(err), program(program), dict(dict), params(params),
   encoding(encoding), contentType(contentType)
{srand(time(0) ^ getpid());}

void Processor_t::run(const FragmentValue_t &data, Writer_t &writer) {
    // ensure content type
    auto *desc = ContentType_t::find(contentType);
    if (!desc) {
        logError(
            err,
            "Invalid content-type in argument of Teng::generatePage(): "
            + contentType + "; using default"
        );
        desc = ContentType_t::getDefault();
    }
    const ContentType_t *ct = desc->contentType.get();

    // run the program
    std::vector<Value_t> stack;
    stack.reserve(128);
    Formatter_t output(writer);
    RunCtx_t ctx{err, program, dict, params, encoding, ct, data, output};
    process(&ctx, stack, {0, static_cast<int32_t>(program.size()), program});

    // log errors into log, if said
    if (params.isLogToOutputEnabled()) logErrors(ct, writer, err);
}

Value_t
Processor_t::eval(const OFFApi_t *frames, int32_t start) {
    // skip evaluation if program is 'empty'
    if (start < 0) return Value_t();
    int32_t end = program.size();

    // init processor context (no run context - we are in compile time)
    std::vector<Value_t> stack;
    stack.reserve(128);
    Error_t opt_err;
    EvalCtx_t ctx{opt_err, program, dict, params, encoding, frames};

    // after evaluation the expression should left result value on the stack top
    if (!process(&ctx, stack, {start, end, program})) return Value_t();
    if (!opt_err.empty()) return Value_t();
    if (stack.size() != 1) return Value_t();
    return Value_t(std::move(stack.back()));
}

} // namespace Teng

