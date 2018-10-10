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
 * $Id: tengfunction.cc,v 1.18 2008-11-20 23:32:29 burlog Exp $
 *
 * DESCRIPTION
 * Teng processor function (like len, substr, round or date)
 *
 * AUTHORS
 * Jan Nemec <jan.nemec@firma.seznam.cz>
 * Stepan Skrob <stepan@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-26  (honza)
 *             Created.
 * 2004-05-08  (vasek)
 *             Polished code.
 * 2005-06-21  (roman)
 *             Win32 support.
 * 2018-06-14  (burlog)
 *             Polished.
 */

#ifndef TENGFUNCTIONNUMBER_H
#define TENGFUNCTIONNUMBER_H

#include <cmath>
#include <algorithm>

#include <tengfunctionutil.h>
#include <tengfunction.h>

namespace Teng {
namespace builtin {
namespace {

// powers of base = 10
static double pow10[] = {
    1.0e+0,  1.0e+1,  1.0e+2,  1.0e+3,  1.0e+4,
    1.0e+5,  1.0e+6,  1.0e+7,  1.0e+8,  1.0e+9,
    1.0e+10, 1.0e+11, 1.0e+12, 1.0e+13, 1.0e+14,
    1.0e+15, 1.0e+16, 1.0e+17, 1.0e+18, 1.0e+19,
    1.0e+20, 1.0e+21, 1.0e+22, 1.0e+23, 1.0e+24,
    1.0e+25, 1.0e+26, 1.0e+27, 1.0e+28, 1.0e+29,
    1.0e+30, 1.0e+31, 1.0e+32, 1.0e+33, 1.0e+34,
    1.0e+35, 1.0e+36, 1.0e+37, 1.0e+38, 1.0e+39
};

/** Rounds integral number according to given precision.
 */
Result_t round_integer(int64_t number, int64_t precision) {
    if (precision >= 0)
        return Result_t(number);

    // do round
    int sign = number < 0 ? -1 : 1;
    for (int l = 0; l > precision; --l)
        number = (number + sign * 5) / 10;
    for (int l = 0; l > precision; --l)
        number *= 10;

    // done
    return Result_t(number);
}

/** Rounds real number according to given precision.
 */
Result_t round_real(double number, int64_t precision) {
    if (precision <= 0) {
        number /= pow10[-precision];
        number = round(number);
        number *= pow10[-precision];
        return Result_t(number);
    }

    number *= pow10[precision];
    number = round(number);
    number /= pow10[precision];
    // TODO(burlog): result.setReal(number, precision);
    return Result_t(number);
}

} // namespace

/** Random integer
 *
 * @param args Teng function arguments
 * @param ctx Teng function ctx
 * @param result Teng function result
 */
Result_t random(Ctx_t &ctx, const Args_t &args) {
    if (args.size() != 1)
        return wrongNumberOfArgs(ctx, "random", 1);

    if (!args.front().is_integral())
        return failed(ctx, "random", "Arg must be an int");

    if (args.front().as_int() < 0)
        return failed(ctx, "random", "Arg must be positive int");

    // ensure that we are in runtime environment
    ctx.runtime_ctx_needed();

    // it is not good to use low bits of rand() see man 3 rand for detail
    return Result_t(
        (rand() * (args.front().as_int() + 0.0)) / (RAND_MAX + 1.0)
    );
}

/** Format number to be suitable for human reading.
 *
 * Function works like round().
 *
 * arg0 the number to be rounded
 * arg1 precision
 * arg2 decimal point character (as string)
 * arg3 thousands separator (as string)
 *
 * @param args Teng function arguments
 * @param ctx Teng function ctx
 * @param result Teng function result
 */
Result_t numformat(Ctx_t &ctx, const Args_t &args) {
    if ((args.size() < 2) || (args.size() > 4))
        return wrongNumberOfArgs(ctx, "numformat", 2, 4);
    auto iarg = args.rbegin();

    // 0: the number
    if (!iarg->is_number())
        return failed(ctx, "numformat", "First arg must be number");
    double number = (iarg++)->real();

    // 1: the precision
    if (!iarg->is_integral())
        return failed(ctx, "numformat", "Second arg must be int");
    if (iarg->as_int() > 39)
        return failed(ctx, "numformat", "Second arg must be in <-39,39> range");
    if (iarg->as_int() < -39)
        return failed(ctx, "numformat", "Second arg must be in <-39,39> range");
    auto precision = (iarg++)->as_int();

    // 2: decimal point
    std::string decipoint = ".";
    if (iarg != args.rend())
        decipoint = (iarg++)->printable();

    // 3: thousands separator
    std::string thousandsep;
    if (iarg != args.rend())
        thousandsep = (iarg++)->printable();

    // round the number
    int sign = 1;
    if (number < 0) {
        sign = -1;
        number = -number;
    }

    // temp value for showing decimal part
    Value_t::int_type powernum = 0;
    if (precision <= 0) {
        number /= pow10[-precision];
        number = ::round(number);
        number *= pow10[-precision];
    }
    else {
        number *= pow10[precision];
        number = ::round(number);
        powernum = static_cast<Value_t::int_type>(number);
        number /= pow10[precision];
    }

    // split number into integer and decimal parts and
    // print string using thousand and decimal separators
    char buf[16];
    std::string str;
    auto int_part = static_cast<Value_t::int_type>(trunc(number));

    // if zero integer part
    if (int_part == 0) {
        str = ((precision > 0) && (sign < 0))? "-0": "0";
    }
    else {
        // while something rest
        while (int_part > 0) {
            auto digits = int_part % 1000;
            int_part /= 1000;

            if (int_part > 0) {
                snprintf(buf, sizeof(buf), "%03ld", digits);
                str = thousandsep + buf + str;
            }
            else {
                snprintf(buf, sizeof(buf), "%s%ld", sign < 0? "-": "", digits);
                str = buf + str;
            }
        }
    }

    // if decimal part
    if (precision > 0) {
        str.append(decipoint);
        int i = 0;
        do {
            auto digit = powernum % 10;
            powernum /= 10;
            str.push_back('0' + digit);
        } while (++i < precision);
        std::reverse(str.end() - i, str.end());
    }

    // store result
    return Result_t(std::move(str));
}

/** Round real number to a defined precision.
 *
 * Examples:
 *     round(1234.56789, 2) => 1234.57
 *     round(1234.56, -2) => 1200
 *
 * @param args Teng function arguments.
 * @param ctx Teng function ctx
 * @param result Teng function result
 */
Result_t round(Ctx_t &ctx, const Args_t &args) {
    if (args.size() != 2)
        return wrongNumberOfArgs(ctx, "round", 2);
    auto iarg = args.rbegin();

    // 0: the number
    if (!iarg->is_number())
        return failed(ctx, "round", "First arg must be number");
    auto number = *iarg++;

    // 1: the precision
    if (!iarg->is_integral())
        return failed(ctx, "round", "Second arg must be int");
    if (iarg->as_int() > 39)
        return failed(ctx, "round", "Second arg must be in <-39,39> range");
    if (iarg->as_int() < -39)
        return failed(ctx, "round", "Second arg must be in <-39,39> range");
    auto precision = (iarg++)->as_int();

    // done
    return Result_t(
        number.is_integral()
        ? round_integer(number.as_int(), precision)
        : round_real(number.as_real(), precision)
    );
}

/** Makes Teng integer from Teng real.
 *
 * @param args Teng function arguments
 * @param ctx Teng function ctx
 * @param result Teng function result
 */
Result_t toint(Ctx_t &ctx, const Args_t &args) {
    if ((args.size() < 1) || (args.size() > 2))
        return wrongNumberOfArgs(ctx, "int", 2, 4);

    // args
    auto &arg = args.back();
    bool ignore_conversion_errors = args.size() == 2? args.front().as_int(): 0;

    // conversion of string to int
    auto str2int_impl = [&] (const string_view_t &arg) {
        char *err = nullptr;

        // try convert string to integral number
        auto int_number = strtol(arg.data(), &err, 10);
        if (*err == '\0') return Result_t(int_number);

        // if any number has been parsed and we should ignore errors return it
        if (err != arg.data())
            if (ignore_conversion_errors)
                return Result_t(int_number);

        // try convert string to floating point number
        auto fp_number = strtod(arg.data(), &err);
        if (*err == '\0') return Result_t(IntType_t(fp_number));

        // to report or not to report an error ;)
        return ignore_conversion_errors
            ? Result_t(fp_number)
            : failed(ctx, "int", "can't convert string to int");
    };

    // do conversion
    switch (arg.type()) {
    case Value_t::tag::integral:
    case Value_t::tag::real:
        return Result_t(arg.integral());

    case Value_t::tag::undefined:
        return ignore_conversion_errors
            ? Result_t(0)
            : failed(ctx, "int", "can't convert undefined to int");

    case Value_t::tag::frag_ref:
        return ignore_conversion_errors
            ? Result_t(0)
            : failed(ctx, "int", "can't convert frag to int");

    case Value_t::tag::list_ref:
        return ignore_conversion_errors
            ? Result_t(0)
            : failed(ctx, "int", "can't convert list to int");

    case Value_t::tag::string:
        return str2int_impl(arg.as_string());

    case Value_t::tag::string_ref:
        return str2int_impl(arg.as_string_ref());

    case Value_t::tag::regex:
        return ignore_conversion_errors
            ? Result_t(0)
            : failed(ctx, "int", "can't convert regex to int");
    }
    throw std::runtime_error(__PRETTY_FUNCTION__);
}

/** Check whether argument is a number.
 *
 * @param args Function arguments (list of values).
 * @param ctx Teng function ctx.
 * @param result Function's result value.
 */
Result_t isnumber(Ctx_t &ctx, const Args_t &args) {
    return args.size() != 1
        ? wrongNumberOfArgs(ctx, "isnumber", 1)
        : Result_t(args.front().is_number());
}

} // namespace builtin
} // namespace Teng

#endif /* TENGFUNCTIONNUMBER_H */

