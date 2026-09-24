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
 * $Id: $
 *
 * DESCRIPTION
 * Teng engine -- compile time expression optimizer tests.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2026-09-24  (burlog)
 *             Created.
 */

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <teng/teng.h>
#include <teng/udf.h>
#include <teng/invoke.h>
#include <teng/filesystem.h>

#include "catch2/catch_test_macros.hpp"
#include "utils.h"

#include "dictionary.h"
#include "configuration.h"
#include "function.h"
#include "instruction.h"
#include "logging.h"
#include "parsercontext.h"
#include "program.h"

namespace {

/** Compiles template and returns its program in human readable form: the
 * name of each instruction, followed by the value for VAL instructions.
 */
std::vector<std::string> c(Teng::Error_t &err, const std::string &templ) {
    auto filesystem = std::make_shared<Teng::Filesystem_t>(TEST_ROOT);
    Teng::Configuration_t params(err, filesystem);
    params.parse(TEST_ROOT "teng.conf");
    Teng::Dictionary_t dict(err, filesystem);
    dict.parse(TEST_ROOT "dict.txt");

    // compile template
    auto program = Teng::compile_string(
        err,
        &dict,
        &params,
        filesystem.get(),
        templ,
        "utf-8",
        "text/html"
    );

    // stringify program
    std::vector<std::string> result;
    for (auto &instr: *program) {
        std::ostringstream os;
        os << instr.instr_name();
        if (instr.opcode() == Teng::OPCODE::VAL)
            os << ' ' << instr.as<Teng::Val_t>().value;
        result.push_back(os.str());
    }
    return result;
}

/** Returns true if program contains given instruction.
 */
bool contains(const std::vector<std::string> &program, const char *instr) {
    for (auto &item: program)
        if (item == instr)
            return true;
    return false;
}

/** Function that needs the runtime context.
 */
Teng::FunctionResult_t
runtime_needed(Teng::FunctionCtx_t &ctx, const Teng::FunctionArgs_t &) {
    ctx.runtime_ctx_needed();
    return Teng::FunctionResult_t("runtime");
}

/** Function that fails with unknown exception.
 */
Teng::FunctionResult_t
unknown_failure(Teng::FunctionCtx_t &, const Teng::FunctionArgs_t &) {
    throw 42;
}

} // namespace

SCENARIO(
    "Constant expressions are folded at compile time",
    "[optimizer]"
) {
    GIVEN("Templates with constant expressions") {
        Teng::Fragment_t root;

        WHEN("Integral arithmetic is compiled") {
            Teng::Error_t err;
            auto t = "${(1 + 2) * 3 - 4}";
            auto program = c(err, t);

            THEN("It is replaced with its value") {
                std::vector<std::string> expected = {
                    "VAL integral(5)",
                    "PRINT",
                    "HALT",
                };
                REQUIRE(program == expected);
                REQUIRE(err.getEntries().empty());
                REQUIRE(g(err, t, root) == "5");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("Real arithmetic is compiled") {
            Teng::Error_t err;
            auto t = "${1.5 * 2}";
            auto program = c(err, t);

            THEN("It is replaced with its value") {
                std::vector<std::string> expected = {
                    "VAL real(3)",
                    "PRINT",
                    "HALT",
                };
                REQUIRE(program == expected);
                REQUIRE(g(err, t, root) == "3.0");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("Unary operators are compiled") {
            Teng::Error_t err;
            auto t = "${-(2 + 3)}${!0}${~0}";
            auto program = c(err, t);

            THEN("They are replaced with their values") {
                std::vector<std::string> expected = {
                    "VAL integral(-5)",
                    "PRINT",
                    "VAL integral(1)",
                    "PRINT",
                    "VAL integral(-1)",
                    "PRINT",
                    "HALT",
                };
                REQUIRE(program == expected);
                REQUIRE(g(err, t, root) == "-51-1");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("String concatenation is compiled") {
            Teng::Error_t err;
            auto t = "${'a' ++ 'b' ++ 'c'}";
            auto program = c(err, t);

            THEN("It is replaced with its value") {
                std::vector<std::string> expected = {
                    "VAL string(abc)",
                    "PRINT",
                    "HALT",
                };
                REQUIRE(program == expected);
                REQUIRE(g(err, t, root) == "abc");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("Comparisons are compiled") {
            Teng::Error_t err;
            auto t = "${1 < 2}${'a' == 'b'}";
            auto program = c(err, t);

            THEN("They are replaced with their values") {
                std::vector<std::string> expected = {
                    "VAL integral(1)",
                    "PRINT",
                    "VAL integral(0)",
                    "PRINT",
                    "HALT",
                };
                REQUIRE(program == expected);
                REQUIRE(g(err, t, root) == "10");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("Lazy evaluated operators are compiled") {
            Teng::Error_t err;
            auto t = "${0 || 'x'}${1 && 2}${0 && 1 / 0}";
            auto program = c(err, t);

            THEN("They are replaced with their values") {
                std::vector<std::string> expected = {
                    "VAL string(x)",
                    "PRINT",
                    "VAL integral(2)",
                    "PRINT",
                    "VAL integral(0)",
                    "PRINT",
                    "HALT",
                };
                REQUIRE(program == expected);
                REQUIRE(g(err, t, root) == "x20");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("Lazy evaluated operators with long strings are compiled") {
            Teng::Error_t err;
            auto t = "${0 || 'the string longer than short string buffer'}"
                     "${1? 'another string longer than short string buffer'"
                     ": 'no'}";
            auto program = c(err, t);

            THEN("The values are owned by the program") {
                std::vector<std::string> expected = {
                    "VAL string(the string longer than short string buffer)",
                    "PRINT",
                    "VAL string(another string longer than short string "
                    "buffer)",
                    "PRINT",
                    "HALT",
                };
                REQUIRE(program == expected);
                REQUIRE(g(err, t, root)
                        == "the string longer than short string buffer"
                           "another string longer than short string buffer");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("Ternary operator is compiled") {
            Teng::Error_t err;
            auto t = "${1 + 1 == 2? 'yes': 'no'}";
            auto program = c(err, t);

            THEN("It is replaced with its value") {
                std::vector<std::string> expected = {
                    "VAL string(yes)",
                    "PRINT",
                    "HALT",
                };
                REQUIRE(program == expected);
                REQUIRE(g(err, t, root) == "yes");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("Case expression is compiled") {
            Teng::Error_t err;
            auto t = "${case(1 + 1, 1: 'one', 2: 'two', *: 'many')}";
            auto program = c(err, t);

            THEN("It is replaced with its value") {
                std::vector<std::string> expected = {
                    "VAL string(two)",
                    "PRINT",
                    "HALT",
                };
                REQUIRE(program == expected);
                REQUIRE(g(err, t, root) == "two");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("Pure builtin functions are compiled") {
            Teng::Error_t err;
            auto t = "${len('abc')}${strtoupper('abc')}${substr('hello', 1, 3)}";
            auto program = c(err, t);

            THEN("They are replaced with their values") {
                std::vector<std::string> expected = {
                    "VAL integral(3)",
                    "PRINT",
                    "VAL string(ABC)",
                    "PRINT",
                    "VAL string(el)",
                    "PRINT",
                    "HALT",
                };
                REQUIRE(program == expected);
                REQUIRE(g(err, t, root) == "3ABCel");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("Existing dict item in expression is compiled") {
            Teng::Error_t err;
            auto t = "${#hello_world ++ '!'}";
            auto program = c(err, t);

            THEN("It is replaced with its value") {
                std::vector<std::string> expected = {
                    "VAL string(hello world!)",
                    "PRINT",
                    "HALT",
                };
                REQUIRE(program == expected);
                REQUIRE(g(err, t, root) == "hello world!");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("The dictexist() and getdict() functions are compiled") {
            Teng::Error_t err;
            auto t = "${dictexist('hello_world')}"
                     "${dictexist('hello_world_missing')}"
                     "${getdict('hello_world_missing', 'default')}";
            auto program = c(err, t);

            THEN("They are replaced with their values") {
                std::vector<std::string> expected = {
                    "VAL integral(1)",
                    "PRINT",
                    "VAL integral(0)",
                    "PRINT",
                    "VAL string(default)",
                    "PRINT",
                    "HALT",
                };
                REQUIRE(program == expected);
                REQUIRE(g(err, t, root) == "10default");
                REQUIRE(err.getEntries().empty());
            }
        }
    }
}

SCENARIO(
    "Folded expressions surrounded by text",
    "[optimizer]"
) {
    GIVEN("Template with text and constant expressions") {
        Teng::Fragment_t root;

        WHEN("It is compiled") {
            Teng::Error_t err;
            auto t = "a${1 + 2}b${'<' ++ '>'}c";
            auto program = c(err, t);

            THEN("The expressions are folded and escaped on print") {
                REQUIRE(contains(program, "VAL integral(3)"));
                REQUIRE(contains(program, "VAL string(<>)"));
                REQUIRE_FALSE(contains(program, "PLUS"));
                REQUIRE_FALSE(contains(program, "CONCAT"));
                REQUIRE(g(err, t, root) == "a3b&lt;&gt;c");
                REQUIRE(err.getEntries().empty());
            }
        }
    }
}

SCENARIO(
    "Non constant expressions are not folded",
    "[optimizer]"
) {
    GIVEN("Data with some variables") {
        Teng::Fragment_t root;
        root.addVariable("a", 1);

        WHEN("Expression with variable is compiled") {
            Teng::Error_t err;
            auto t = "${a + 2}";
            auto program = c(err, t);

            THEN("The variable is looked up in runtime") {
                REQUIRE(contains(program, "VAR"));
                REQUIRE(contains(program, "PLUS"));
                REQUIRE(g(err, t, root) == "3");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("Constant subexpression of non constant expression is compiled") {
            Teng::Error_t err;
            auto t = "${a + (2 * 3)}";
            auto program = c(err, t);

            THEN("Only the constant subexpression is folded") {
                REQUIRE(contains(program, "VAR"));
                REQUIRE(contains(program, "VAL integral(6)"));
                REQUIRE_FALSE(contains(program, "MUL"));
                REQUIRE(g(err, t, root) == "7");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("Ternary operator with non constant condition is compiled") {
            Teng::Error_t err;
            auto t = "${a? 1 + 2: 3 * 4}";
            auto program = c(err, t);

            THEN("The branches are folded but the jumps remain valid") {
                REQUIRE(contains(program, "VAL integral(3)"));
                REQUIRE(contains(program, "VAL integral(12)"));
                REQUIRE(g(err, t, root) == "3");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("Lazy operator with non constant operand is compiled") {
            Teng::Error_t err;
            auto t = "${a && 1 + 2}${!a || 3 * 4}";
            auto program = c(err, t);

            THEN("The constant operand is folded") {
                REQUIRE(contains(program, "VAL integral(3)"));
                REQUIRE(contains(program, "VAL integral(12)"));
                REQUIRE(g(err, t, root) == "312");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("The now() function is compiled") {
            Teng::Error_t err;
            auto t = "${now() > 0}";
            auto program = c(err, t);

            THEN("It is called in runtime") {
                REQUIRE(contains(program, "FUNC"));
                REQUIRE(g(err, t, root) == "1");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("The random() function is compiled") {
            Teng::Error_t err;
            auto t = "${random(1) < 1}";
            auto program = c(err, t);

            THEN("It is called in runtime") {
                REQUIRE(contains(program, "FUNC"));
                REQUIRE(g(err, t, root) == "1");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("The isenabled() function is compiled") {
            Teng::Error_t err;
            auto t = "${isenabled('shorttag')}";
            auto program = c(err, t);

            THEN("It is called in runtime") {
                REQUIRE(contains(program, "FUNC"));
                REQUIRE(g(err, t, root) == "1");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("The escape() function is compiled") {
            Teng::Error_t err;
            auto t = "%{escape('<b>')}";
            auto program = c(err, t);

            THEN("It is called in runtime without errors") {
                REQUIRE(contains(program, "FUNC"));
                REQUIRE(err.getEntries().empty());
                REQUIRE(g(err, t, root) == "&lt;b&gt;");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("The unescape() function is compiled") {
            Teng::Error_t err;
            auto t = "%{unescape('&lt;b&gt;')}";
            auto program = c(err, t);

            THEN("It is called in runtime without errors") {
                REQUIRE(contains(program, "FUNC"));
                REQUIRE(err.getEntries().empty());
                REQUIRE(g(err, t, root) == "<b>");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("The escape() function is used in different content types") {
            Teng::Error_t err;
            auto t = "<?teng ctype 'quoted-string'?>"
                     "%{escape('a\"b')}"
                     "<?teng endctype?>"
                     "%{escape('a\"b')}";
            auto result = g(err, t, root);

            THEN("The escaping of current content type is used") {
                REQUIRE(result == "a\\\"ba&quot;b");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("User defined function is compiled") {
            Teng::udf::registerFunction(
                "optimizer_test",
                [] (const Teng::udf::Args_t &args) {
                    return Teng::udf::Result_t(args.front().as_int() * 2);
                }
            );
            Teng::Error_t err;
            auto t = "${udf.optimizer_test(1 + 2)}";
            auto program = c(err, t);

            THEN("It is called in runtime but its args are folded") {
                REQUIRE(contains(program, "FUNC"));
                REQUIRE(contains(program, "VAL integral(3)"));
                REQUIRE(err.getEntries().empty());
                REQUIRE(g(err, t, root) == "6");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("Unknown function is compiled") {
            Teng::Error_t err;
            auto t = "${unknown_function(1)}";
            auto program = c(err, t);

            THEN("The error is reported once in runtime") {
                REQUIRE(contains(program, "FUNC"));
                REQUIRE(err.getEntries().empty());
                g(err, t, root);
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::ERROR,
                    {1, 2},
                    "Runtime: Call of unknown function unknown_function()"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
            }
        }

        WHEN("Missing dict item in expression is compiled") {
            Teng::Error_t err;
            auto t = "${#hello_world_missing ++ '!'}";
            auto program = c(err, t);

            THEN("The item is looked up in runtime") {
                REQUIRE(contains(program, "DICT"));
                REQUIRE(err.getEntries().empty());
                REQUIRE(g(err, t, root) == "hello_world_missing!");
                std::vector<Teng::Error_t::Entry_t> errs = {{
                    Teng::Error_t::WARNING,
                    {1, 3},
                    "Runtime: Dictionary item 'hello_world_missing' "
                    "was not found"
                }};
                ERRLOG_TEST(err.getEntries(), errs);
            }
        }
    }
}

SCENARIO(
    "Failing constant expressions are not folded",
    "[optimizer]"
) {
    GIVEN("Templates with failing constant expressions") {
        Teng::Fragment_t root;

        WHEN("Division by zero is compiled") {
            Teng::Error_t err;
            auto t = "${1 / 0}";
            auto program = c(err, t);

            THEN("The error is not reported during compilation") {
                REQUIRE(contains(program, "DIV"));
                REQUIRE(err.getEntries().empty());
            }

            THEN("The error is reported once in runtime") {
                g(err, t, root);
                REQUIRE(err.getEntries().size() == 1);
            }
        }

        WHEN("Function with wrong number of args is compiled") {
            Teng::Error_t err;
            auto t = "${len()}";
            auto program = c(err, t);

            THEN("The error is not reported during compilation") {
                REQUIRE(contains(program, "FUNC"));
                REQUIRE(err.getEntries().empty());
            }

            THEN("The error is reported once in runtime") {
                g(err, t, root);
                REQUIRE(err.getEntries().size() == 1);
            }
        }
    }
}

SCENARIO(
    "Folded expressions inside of statements",
    "[optimizer]"
) {
    GIVEN("Data with fragments") {
        Teng::Fragment_t root;
        root.addFragment("x");
        root.addFragment("x");

        WHEN("If statement with constant condition is rendered") {
            Teng::Error_t err;
            auto t = "<?teng if 1 + 1 == 2?>yes<?teng elif 2 * 2?>elif"
                     "<?teng else?>no<?teng endif?>${3 * 3}";
            auto result = g(err, t, root);

            THEN("The branch jumps remain valid") {
                REQUIRE(result == "yes9");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("If statement with false constant condition is rendered") {
            Teng::Error_t err;
            auto t = "<?teng if 1 - 1?>yes<?teng elif 2 * 0?>elif"
                     "<?teng else?>${'n' ++ 'o'}<?teng endif?>";
            auto result = g(err, t, root);

            THEN("The else branch is used") {
                REQUIRE(result == "no");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("Nested if statements with constant expressions are rendered") {
            Teng::Error_t err;
            auto t = "<?teng if 1?>a${1 + 1}"
                     "<?teng if 0?>b<?teng else?>c${2 + 2}<?teng endif?>"
                     "d${3 + 3}<?teng endif?>e";
            auto result = g(err, t, root);

            THEN("The branch jumps remain valid") {
                REQUIRE(result == "a2c4d6e");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("Constant expression inside of fragment is rendered") {
            Teng::Error_t err;
            auto t = "<?teng frag x?>${1 + 2}${_count * 2}<?teng endfrag?>";
            auto result = g(err, t, root);

            THEN("It is printed for each fragment") {
                REQUIRE(result == "3434");
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("Set statement with constant expression is rendered") {
            Teng::Error_t err;
            auto t = "<?teng set .v = 2 * 21?>${.v}";
            auto result = g(err, t, root);

            THEN("The variable has folded value") {
                REQUIRE(result == "42");
                REQUIRE(err.getEntries().empty());
            }
        }
    }
}

SCENARIO(
    "The function invoker passes the request for runtime context through",
    "[optimizer]"
) {
    GIVEN("Function context without escaper (compile time)") {
        Teng::Error_t err;
        auto filesystem = std::make_shared<Teng::Filesystem_t>(TEST_ROOT);
        Teng::Configuration_t params(err, filesystem);
        Teng::Dictionary_t dict(err, filesystem);
        Teng::Pos_t pos;
        Teng::string_view_t encoding = "utf-8";
        Teng::FunctionCtx_t ctx(err, pos, encoding, nullptr, params, dict);
        Teng::FunctionArgs_t args;
        auto *pctx = &err;

        WHEN("Function needing runtime context is invoked") {
            std::string name = "runtime_needed";
            Teng::Invoker_t<Teng::Function_t> invoker{name, runtime_needed};

            THEN("The request is rethrown and nothing is logged") {
                REQUIRE_THROWS_AS(
                    invoker(pctx, ctx, args),
                    Teng::runtime_functx_needed_t
                );
                REQUIRE(err.getEntries().empty());
            }
        }

        WHEN("Function failing with unknown exception is invoked") {
            std::string name = "unknown_failure";
            Teng::Invoker_t<Teng::Function_t> invoker{name, unknown_failure};

            THEN("The failure is logged") {
                auto result = invoker(pctx, ctx, args);
                REQUIRE(result.is_undefined());
                REQUIRE(err.getEntries().size() == 1);
            }
        }
    }
}
