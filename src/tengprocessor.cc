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

// TODO(burlog): remove it?
#include <iomanip>

#include "tengprocessorcontext.h"
#include "tengprocessorother.h"
#include "tengprocessorfrag.h"
#include "tengprocessorops.h"
#include "tengprocessor.h"

namespace Teng {
namespace {

/** Some, even whole, part of program.
 */
struct SubProgram_t {
    const Instruction_t &operator[](int32_t i) const {return program[i];}
    const int32_t start; //!< pointer to the first instruction in program
    const int32_t end;   //!< pointer to one past last instruction in program
    const Program_t &program; //!< the whole program
};

/** Represents safe instruction pointer.
 */
struct InstructionPointer_t {
    /** C'tor.
     */
    InstructionPointer_t(const SubProgram_t &program)
        : value(program.start), program(program)
    {}

    /** Increments the instruction pointer (it check boundaries).
     */
    int32_t operator++() {
        *this += 1;
        return value;
    }

    /** Increments the instruction pointer (it check boundaries).
     */
    int32_t operator++(int) {
        int32_t tmp = value;
        *this += 1;
        return tmp;
    }

    /** Increments the instruction pointer (it check boundaries).
     */
    int32_t operator+=(int32_t incr) {
        return value = *this + incr;
    }

    /** Increments the instruction pointer (it check boundaries).
     */
    int32_t operator+(int32_t incr) const {
        auto new_value = value + incr;
        if (new_value < 0)
            throw std::runtime_error("instruction pointer underflow");
        return new_value;
    }

    /** Returns true if value of ip is less than given address.
     */
    int32_t operator<(int32_t addr) const {
        return value < addr;
    }

    /** Returns numeric value of instruction pointer.
     */
    int32_t operator*() const {return value;}

protected:
    int32_t value;               //!< never will be changed to unsigned !!
    const SubProgram_t &program; //!< evaluated program
};

/** The core of Teng template engine. Renders the template.
 */
template <typename Ctx_t>
bool
process(Ctx_t *ctx, std::stack<Value_t> &stack, const SubProgram_t &program) {
    std::vector<Value_t> prg_stack;
    bool evaluating = std::is_same<std::decay_t<Ctx_t>, EvalCtx_t>::value;
    std::cerr << std::endl << "## PROGRAM -- "
              << (evaluating? "eval": "run")
              << std::endl;
    for (int i = program.start; i < program.end; ++i) {
        std::cerr << std::setw(3) << std::setfill('0')
                  << std::noshowpos << i << " " << program[i]
                  << std::endl;
    }
    std::cerr << "## END" << std::endl << std::endl;
    auto dump_instr = [&] (InstructionPointer_t &ip) {
        std::ostringstream os;
        os << std::setw(3) << std::setfill('0')
           << std::noshowpos << *ip << " " << program[*ip];
        int64_t len = std::max(0l, 75l - static_cast<int64_t>(os.str().size()));
        os << std::string(len, ' ') << " | ";
        auto stack_copy = stack;
        while (!stack_copy.empty()) {
            os << '[' << stack_copy.top() << "]";
            stack_copy.pop();
        }
        len = std::max(0l, 125l - static_cast<int64_t>(os.str().size()));
        os << std::string(len, ' ') << " | ";
        auto prg_stack_copy = prg_stack;
        while (!prg_stack_copy.empty()) {
            os << '[' << prg_stack_copy.back() << "]";
            prg_stack_copy.pop_back();
        }
        std::cerr << os.str() << std::endl;
    };

    // exec program on stack-based processor
    std::cerr << "## EXECUTION -- "
              << (evaluating? "eval": "run")
              << std::endl;
    GetArg_t get_arg(stack);
    for (InstructionPointer_t ip(program); ip < program.end; ++ip) try {
        ctx->instr = &program[*ip];
        dump_instr(ip);

        switch (ctx->instr->opcode()) {
        case OPCODE::NOOP:
            break;

        case OPCODE::DEBUG_FRAG:
            exec::debuging(ctx);
            break;

        case OPCODE::BYTECODE_FRAG:
            exec::bytecode(ctx);
            break;

        case OPCODE::VAL:
            stack.push(ctx->instr->template as<Val_t>().value);
            break;

        case OPCODE::DICT:
            stack.push(exec::dict(ctx, get_arg));
            break;

        case OPCODE::VAR:
            // TODO(burlog): zalomit
            stack.push(exec::var(ctx, program[ip + 1].opcode() == OPCODE::PRINT));
            break;

        case OPCODE::PRG_STACK_PUSH:
            exec::prg_stack_push(prg_stack, get_arg);
            break;

        case OPCODE::PRG_STACK_POP:
            exec::prg_stack_pop(prg_stack);
            break;

        case OPCODE::PRG_STACK_AT:
            stack.push(exec::prg_stack_at(ctx, prg_stack));
            break;

        case OPCODE::BIT_OR:
            stack.push(exec::numop(ctx, get_arg, std::bit_or<int64_t>()));
            break;

        case OPCODE::BIT_XOR:
            stack.push(exec::numop(ctx, get_arg, std::bit_xor<int64_t>()));
            break;

        case OPCODE::BIT_AND:
            stack.push(exec::numop(ctx, get_arg, std::bit_and<int64_t>()));
            break;

        case OPCODE::UNARY_PLUS:
            stack.push(exec::unary_plus(ctx, get_arg));
            break;

        case OPCODE::UNARY_MINUS:
            stack.push(exec::unary_minus(ctx, get_arg));
            break;

        case OPCODE::PLUS:
            stack.push(exec::strnumop(ctx, get_arg, std::plus<>()));
            break;

        case OPCODE::MINUS:
            stack.push(exec::numop(ctx, get_arg, std::minus<>()));
            break;

        case OPCODE::MUL:
            stack.push(exec::numop(ctx, get_arg, std::multiplies<>()));
            break;

        case OPCODE::DIV:
            stack.push(exec::numop(ctx, get_arg, std::divides<>()));
            break;

        case OPCODE::MOD:
            stack.push(exec::numop(ctx, get_arg, std::modulus<int64_t>()));
            break;

        case OPCODE::EQ:
            stack.push(exec::strnumop(ctx, get_arg, std::equal_to<>()));
            break;

        case OPCODE::NE:
            stack.push(exec::strnumop(ctx, get_arg, std::not_equal_to<>()));
            break;

        case OPCODE::GE:
            stack.push(exec::strnumop(ctx, get_arg, std::greater_equal<>()));
            break;

        case OPCODE::GT:
            stack.push(exec::strnumop(ctx, get_arg, std::greater<>()));
            break;

        case OPCODE::LE:
            stack.push(exec::strnumop(ctx, get_arg, std::less_equal<>()));
            break;

        case OPCODE::LT:
            stack.push(exec::strnumop(ctx, get_arg, std::less<>()));
            break;

        case OPCODE::CONCAT:
            stack.push(exec::strop(ctx, get_arg, std::plus<>()));
            break;

        case OPCODE::STR_EQ:
            stack.push(exec::strop(ctx, get_arg, std::equal_to<>()));
            break;

        case OPCODE::STR_NE:
            stack.push(exec::strop(ctx, get_arg, std::not_equal_to<>()));
            break;

        case OPCODE::REPEAT:
            stack.push(exec::repeat_string(ctx, get_arg));
            break;

        case OPCODE::NOT:
            stack.push(exec::logic_not(ctx, get_arg));
            break;

        case OPCODE::BIT_NOT:
            stack.push(exec::bit_not(ctx, get_arg));
            break;

        case OPCODE::REGEX_MATCH:
            stack.push(exec::regex_match(ctx, get_arg));
            break;

        case OPCODE::FUNC:
            stack.push(exec::func(ctx, get_arg));
            break;

        case OPCODE::AND:
            if (stack_top(stack)) stack.pop();
            else ip += ctx->instr->template as<And_t>().addr_offset;
            break;

        case OPCODE::OR:
            if (!stack_top(stack)) stack.pop();
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

        case OPCODE::PUSH_FRAG_COUNT:
            stack.push(exec::frag_count(ctx));
            break;

        case OPCODE::PUSH_FRAG_INDEX:
            stack.push(exec::frag_index(ctx));
            break;

        case OPCODE::PUSH_FRAG_FIRST:
            stack.push(exec::is_first_frag(ctx));
            break;

        case OPCODE::PUSH_FRAG_LAST:
            stack.push(exec::is_last_frag(ctx));
            break;

        case OPCODE::PUSH_FRAG_INNER:
            stack.push(exec::is_inner_frag(ctx));
            break;

        case OPCODE::PUSH_FRAG:
            stack.push(exec::push_frag(ctx));
            break;

        case OPCODE::PUSH_ROOT_FRAG:
            stack.push(exec::push_root_frag(ctx));
            break;

        case OPCODE::PUSH_THIS_FRAG:
            stack.push(exec::push_this_frag(ctx));
            break;

        case OPCODE::PRINT:
            exec::print(ctx, get_arg);
            break;

        case OPCODE::SET:
            exec::set_var(ctx, get_arg);
            break;

        case OPCODE::OPEN_CTYPE:
            exec::push_escaper(ctx);
            break;

        case OPCODE::CLOSE_CTYPE:
            exec::pop_escaper(ctx);
            break;

        case OPCODE::PUSH_ATTR_AT:
            stack.push(exec::push_attr_at(ctx, get_arg));
            break;

        case OPCODE::POP_ATTR:
            stack.push(exec::pop_attr(ctx, get_arg));
            break;

        case OPCODE::PUSH_ATTR:
            stack.push(exec::push_attr(ctx, get_arg));
            break;

        case OPCODE::REPR:
            stack.push(exec::repr(ctx, get_arg));
            break;

        case OPCODE::REPR_JSONIFY:
            stack.push(exec::repr_jsonify(ctx, get_arg));
            break;

        case OPCODE::REPR_COUNT:
            stack.push(exec::repr_count(ctx, get_arg));
            break;

        case OPCODE::REPR_TYPE:
            stack.push(exec::repr_type(ctx, get_arg));
            break;

        case OPCODE::REPR_DEFINED:
            stack.push(exec::repr_defined(ctx, get_arg));
            break;

        case OPCODE::REPR_EXISTS:
            stack.push(exec::repr_exists(ctx, get_arg));
            break;

        case OPCODE::REPR_ISEMPTY:
            stack.push(exec::repr_isempty(ctx, get_arg));
            break;

        case OPCODE::HALT:
            break;
        }

    } catch (const runtime_ctx_needed_t &) {
        if (std::is_same<std::decay_t<Ctx_t>, RunCtx_t>::value)
            throw std::runtime_error("runtime ctx of runtime ctx requested");
        std::cerr << "## END" << std::endl << std::endl;
        return false;

    } catch (const runtime_functx_needed_t &) {
        std::cerr << "## END" << std::endl << std::endl;
        return false;

    } catch (const std::exception &e) {
        std::cerr << "## END (" << e.what() << ")" << std::endl << std::endl;
        logFatal(*ctx, e.what());
        return false;
    }

    // warn about relicts on value and program stack
    if (!prg_stack.empty())
        logWarning(*ctx, "Program stack is not empty");
    if (!stack.empty())
        logWarning(*ctx, "Value stack is not empty");
    std::cerr << "## END" << std::endl << std::endl;
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
            + contentType
        );
        desc = ContentType_t::getDefault();
    }
    const ContentType_t *ct = desc->contentType.get();

    // run the program
    std::stack<Value_t> stack;
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
    std::stack<Value_t> stack;
    EvalCtx_t ctx{err, program, dict, params, encoding, frames};

    // disable log during optimization of expressions
    ++ctx.log_suppressed;

    // after evaluation the expression should left result value on the stack top
    if (!process(&ctx, stack, {start, end, program})) return Value_t();
    if (err.count()) return Value_t();
    if (stack.size() != 1) return Value_t();
    return Value_t(std::move(stack.top()));
}

} // namespace Teng

