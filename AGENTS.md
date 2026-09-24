# AGENTS.md

Teng is a general-purpose C++ template engine by Seznam.cz (LGPL-2.1).
It is in maintenance mode: bug fixes go in, big features don't. The current
line is 5.x on `master`. 3.x lives in the `libteng3` branch.

## Build and test (Meson)

Use Meson. Ignore `Makefile`: it is a symlink to a personal helper and is
not part of the project.

```sh
meson setup build -Db_sanitize=address,undefined,leak
meson compile -C build
meson test -C build                 # runs test-teng (Catch2)
build/test-teng "[cond]"            # run one Catch2 tag
meson compile -C build clang-tidy   # only if clang-tidy is installed
```

- Dependencies: flex, bison, glib-2.0, libpcre2-8, python3 (it generates
  `config.h`). C++17, static library only.
- `wrap_mode=nofallback` is the default, and only `catch2-with-main` is
  forced as a fallback (git wrap, `subprojects/catch2-with-main.wrap`).
- The build generates the parser and lexer from `src/syntax.yy` (bison) and
  `src/lex2impl.ll` (flex). Edit those files, never the generated
  `build/syntax.cc` or `build/lex2impl.cc`.
- `-DDEBUG` is defined only when optimization is off (`-Doptimization=0`).
- Tests find their data files through the `SRC_DIR` macro, so
  `build/test-teng` runs from any directory.

## Layout

- `include/teng/`: public headers. A new public header must also be added
  to `headers` in `meson.build`, or it won't be installed.
- `src/`: implementation. The file prefixes show the pipeline:
  `lex1`/`lex2` (lexers), `syntax.yy` plus `parser*` and `semantic*`
  (grammar and semantic actions), `instruction`/`program` (bytecode),
  `processor*` (the VM), `function*.h` (built-in functions).
- `tests/`: Catch2 BDD tests (`SCENARIO`/`GIVEN`/`WHEN`/`THEN`, one tag per
  file, such as `[cond]`). Shared helpers are in `tests/utils.h`, and
  fixtures are the `*.html`, `*.txt` and `*.conf` files in `tests/`.
- `meson.build` lists every source and test file explicitly. Add new
  files there.
- `debian/`: Debian packaging. `gdb-pretty-printers/`: GDB helpers.

## Code conventions

- Every file starts with the LGPL license header block (DESCRIPTION /
  AUTHORS / HISTORY). Copy it from a neighbouring file.
- Header guards look like `TENG<NAME>_H`, for example `TENGSEMANTICIF_H`.
- Code lives in `namespace Teng` and sub-namespaces such as `Parser`.
  Type names end in `_t` (`Fragment_t`, `Pos_t`), methods use camelCase,
  and parser helpers use snake_case. Indent with 4 spaces.
- There is no `.clang-format`, so follow the style of the surrounding code.
  `.clang-tidy` defines the lint checks.
- Keep the build free of warnings on both GCC and clang. The build enables
  many warnings (`-Wconversion`, `-Wuseless-cast`, ...).

## Commits and releases

- Commit subjects are short and imperative. Recent ones often use a prefix
  such as `fix:`, `build:`, `feat:` or `docs:`, with the GitHub PR number
  in parentheses, like `(#32)`.
- A release is its own commit with the subject ` #! vX.Y.Z` (note the
  leading space). It bumps `version` in `meson.build` and adds a
  `debian/changelog` entry built from the commit messages since the last
  release. Don't make release commits unless asked.
