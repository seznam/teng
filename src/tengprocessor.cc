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
    const Instruction_t &operator[](int i) const {return program[i];}
    const int start; //!< pointer to the first valid instruction in program
    const int end;   //!< pointer to one past last instruction in program
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
    int operator++() {
        *this += 1;
        return value;
    }

    /** Increments the instruction pointer (it check boundaries).
     */
    int operator++(int) {
        int tmp = value;
        *this += 1;
        return tmp;
    }

    /** Increments the instruction pointer (it check boundaries).
     */
    int operator+=(int incr) {
        return value = *this + incr;
    }

    /** Increments the instruction pointer (it check boundaries).
     */
    int operator+(int incr) const {
        auto new_value = value + incr;
        if (new_value < 0)
            throw std::runtime_error("instruction pointer underflow");
        return new_value;
    }

    /** Returns true if value of ip is less than given address.
     */
    int operator<(int addr) const {
        return value < addr;
    }

    /** Returns numeric value of instruction pointer.
     */
    int operator*() const {return value;}

protected:
    int value;                   //!< never will be changed to unsigned !!
    const SubProgram_t &program; //!< evaluated program
};

/** The core of Teng template engine. Renders the template.
 */
template <typename Ctx_t>
bool
process(Ctx_t *ctx, std::stack<Value_t> &stack, const SubProgram_t &program) {
    bool evaluating = std::is_same<std::decay_t<Ctx_t>, EvalCtx_t>::value;
    std::cerr << std::endl << "## PROGRAM -- "
              << (evaluating? "eval": "run")
              << std::endl;
    for (int i = program.start; i < program.end; ++i) {
        std::cerr << std::setw(3) << std::setfill('0')
                  << std::noshowpos << i << "\t" << program[i]
                  << std::endl;
    }
    std::cerr << "## END" << std::endl << std::endl;
    auto dump_instr = [&] (InstructionPointer_t &ip) {
        std::cerr << std::setw(3) << std::setfill('0')
                  << std::noshowpos << *ip << "\t" << program[*ip]
                  << std::endl;
    };
    using I = Instruction_t::OpCode_t;
    std::vector<Value_t> prg_stack;

    // because of undefined order of function arguments evaluation
    // you can't do some_function(move_back(stack), move_back(stack));
    // this lambda solves the issue by postponing args poping into function body
    auto get_arg = [&] {return move_back(stack);};

    // exec program on stack-based processor
    std::cerr << "## EXECUTION -- "
              << (evaluating? "eval": "run")
              << std::endl;
    for (InstructionPointer_t ip(program); ip < program.end; ++ip) try {
        ctx->instr = &program[*ip];
        dump_instr(ip);

        switch (ctx->instr->opcode) {
        case I::DEFINED:
            stack.push(exec::defined(ctx));
            break;

        case I::ISEMPTY:
            stack.push(exec::isempty(ctx));
            break;

        case I::EXISTS:
            stack.push(exec::exists(ctx));
            break;

        case I::DEBUG_FRAG:
            exec::debuging(ctx);
            break;

        case I::BYTECODE_FRAG:
            exec::bytecode(ctx);
            break;

        case I::VAL:
            stack.push(ctx->instr->value);
            break;

        case I::DICT:
            stack.push(exec::dict(ctx, get_arg));
            break;

        case I::VAR:
            stack.push(exec::var(ctx, program[ip + 1].opcode == I::PRINT));
            break;

        case I::PUSH:
            exec::prg_stack_push(prg_stack, get_arg);
            break;

        case I::POP:
            exec::prg_stack_pop(prg_stack);
            break;

        case I::STACK:
            stack.push(exec::prg_stack_at(ctx, prg_stack));
            break;

        case I::BITOR:
            stack.push(exec::numop(ctx, get_arg, std::bit_or<int64_t>()));
            break;

        case I::BITXOR:
            stack.push(exec::numop(ctx, get_arg, std::bit_xor<int64_t>()));
            break;

        case I::BITAND:
            stack.push(exec::numop(ctx, get_arg, std::bit_and<int64_t>()));
            break;

        case I::ADD:
            stack.push(exec::numop(ctx, get_arg, std::plus<>()));
            break;

        case I::SUB:
            stack.push(exec::numop(ctx, get_arg, std::minus<>()));
            break;

        case I::MUL:
            stack.push(exec::numop(ctx, get_arg, std::multiplies<>()));
            break;

        case I::DIV:
            stack.push(exec::numop(ctx, get_arg, std::divides<>()));
            break;

        case I::MOD:
            stack.push(exec::numop(ctx, get_arg, std::modulus<int64_t>()));
            break;

        case I::NUMEQ:
            stack.push(exec::numop(ctx, get_arg, std::equal_to<>()));
            break;

        case I::NUMGE:
            stack.push(exec::numop(ctx, get_arg, std::greater_equal<>()));
            break;

        case I::NUMGT:
            stack.push(exec::numop(ctx, get_arg, std::greater<>()));
            break;

        case I::CONCAT:
            stack.push(exec::strop(ctx, get_arg, std::plus<>()));
            break;

        case I::STREQ:
            stack.push(exec::strop(ctx, get_arg, std::equal_to<>()));
            break;

        case I::REPEAT:
            stack.push(exec::repeat_string(ctx, get_arg));
            break;

        case I::NOT:
            stack.push(exec::logic_not(ctx, get_arg));
            break;

        case I::BITNOT:
            stack.push(exec::bit_not(ctx, get_arg));
            break;

        case I::FUNC:
            stack.push(exec::func(ctx, get_arg));
            break;

        case I::AND:
            if (stack_top(stack)) stack.pop();
            else ip += ctx->instr->value.integral();
            break;

        case I::OR:
            if (!stack_top(stack)) stack.pop();
            else ip += ctx->instr->value.integral();
            break;

        case I::JMPIFNOT:
            if (!get_arg())
                ip += ctx->instr->value.integral();
            break;

        case I::JMP:
            ip += ctx->instr->value.integral();
            break;

        case I::FORM:
            exec::push_formatter(ctx);
            break;

        case I::ENDFORM:
            exec::pop_formatter(ctx);
            break;

        case I::FRAG:
            if (auto shift = exec::open_frag(ctx))
                ip += shift;
            break;

        case I::ENDFRAG:
            if (auto shift = exec::close_frag(ctx))
                ip += shift;
            break;

        case I::FRAGCNT:
            stack.push(exec::frag_count(ctx));
            break;

        case I::NESTED_FRAGCNT:
            stack.push(exec::nested_frag_count(ctx));
            break;

        case I::FRAGINDEX:
            stack.push(exec::frag_index(ctx));
            break;

        case I::FRAGFIRST:
            stack.push(exec::first_frag(ctx));
            break;

        case I::FRAGLAST:
            stack.push(exec::last_frag(ctx));
            break;

        case I::FRAGINNER:
            stack.push(exec::inner_frag(ctx));
            break;

        case I::PRINT:
            exec::print(ctx, get_arg);
            break;

        case I::SET:
            exec::set_var(ctx, get_arg);
            break;

        case I::CTYPE:
            exec::push_escaper(ctx);
            break;

        case I::ENDCTYPE:
            exec::pop_escaper(ctx);
            break;

        case I::SUPRESS_LOG:
            ++ctx->log_suppressed;
            break;

        case I::PUSH_ATTR_AT:
            stack.push(exec::push_attr_at(ctx, get_arg));
            break;

        case I::PUSH_ATTR:
            stack.push(exec::push_attr(ctx, get_arg));
            break;

        case I::PUSH_ROOT_FRAG:
            stack.push(exec::push_root_frag(ctx));
            break;

        case I::PUSH_THIS_FRAG:
            stack.push(exec::push_this_frag(ctx));
            break;

        case I::REPR:
            stack.push(exec::repr(ctx, get_arg));
            break;

        case I::REPR_JSONIFY:
            stack.push(exec::repr_jsonify(ctx, get_arg));
            break;

        case I::REPR_COUNT:
            stack.push(exec::repr_count(ctx, get_arg));
            break;

        case I::REPR_TYPE:
            stack.push(exec::repr_type(ctx, get_arg));
            break;

        case I::REPR_DEFINED:
            stack.push(exec::repr_defined(ctx, get_arg));
            break;

        case I::REPR_EXISTS:
            stack.push(exec::repr_exists(ctx, get_arg));
            break;

        case I::REPR_ISEMPTY:
            stack.push(exec::repr_isempty(ctx, get_arg));
            break;

        case I::HALT:
            break;
        }

    } catch (const runtime_ctx_needed_t &) {
        std::cerr << "## END" << std::endl << std::endl;
        std::cerr << "Can't optimize!" << std::endl;
        return false;

    } catch (const std::exception &e) {
        std::cerr << "## END" << std::endl << std::endl;
        logFatal(*ctx, e.what());
        return false;
    }
    std::cerr << "## END" << std::endl << std::endl;
    return true;
}

/** Returns default content type.
 */
const ContentType_t *default_content_type() {
    return ContentType_t::getDefault()->contentType.get();
}

} // namespace

Processor_t::Processor_t(
    Error_t &err,
    const Program_t &program,
    const Dictionary_t &dict,
    const Configuration_t &params,
    const std::string &encoding,
    const ContentType_t *contentType
): err(err), program(program), dict(dict), params(params), encoding(encoding),
   contentType(contentType? default_content_type(): contentType)
{srand(time(0) ^ getpid());}

void Processor_t::run(const Fragment_t &data, Formatter_t &output) {
    std::stack<Value_t> stack;
    RunCtx_t ctx{err, program, dict, params, encoding, contentType, data, output};
    process(&ctx, stack, {0, static_cast<int>(program.size()), program});
}

int Processor_t::eval(Parser::Value_t &result, int start, int end) {
    // skip evaluation if program is 'empty'
    if (end > program.size()) return -1;
    if (start < 0) return -1;
    if (start >= end) return -1;

    // init processor context (no run context - we are in compile time)
    std::stack<Value_t> stack;
    EvalCtx_t ctx{err, program, dict, params, encoding};

    // after evaluation the expression should left result value on the stack top
    if (!process(&ctx, stack, {start, end, program})) return -1;
    if (stack.size() != 1) return -1;
    result = stack.top();
    return 0;
}

} // namespace Teng

