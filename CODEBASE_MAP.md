# Codebase map

This file shows where things live in Teng. For build commands, test
commands and code conventions, see [AGENTS.md](AGENTS.md).

## Architecture

```
Teng_t::generatePage()                                  src/teng.cc
  |
  |  TemplateCache_t::createTemplate()                  src/template.{h,cc}
  |    returns cached or freshly built Template_t {program, dict, params}
  |    Dictionary_t     (language dict,  #{key})         src/dictionary.*
  |    Configuration_t  (params/config, %enable ...)     src/configuration.*
  |    Program_t        compiled by:
  v
compile_file() / compile_string()                       src/parsercontext.cc
  Parser::Context_t: lexer stack, open frames, the program being built
  |
  |  Lex1_t   splits the source into TEXT and directives: src/lex1.*
  |           <?teng ...?>, <? ...?>, ${expr}, %{expr}, #{dict}
  |  Lex2_t   tokenizes directive bodies                  src/lex2.*, lex2impl.ll (flex)
  |  Context_t::next_token() feeds the bison parser
  v
Parser::Parser_t (bison, api.namespace Teng::Parser::Impl)  src/syntax.yy
  |  grammar actions call generate_*/prepare_*/...       src/semantic*.{h,cc}
  |  compile-time expression folding (optimize_expr)     src/semanticexpr.cc
  v
Program_t = vector of Instruction_t (OPCODE, ~100 ops)   src/program.*, instruction.*
  |
  v
Processor_t::run(FragmentValue_t(&data), writer)         src/processor.{h,cc}
  |  switch over OPCODE -> executors in namespace exec    src/processor{ops,frag,other,debug}.h
  |  runtime frame stack (OpenFrames_t)                  src/openframes.h
  |  builtin functions (namespace builtin)               src/function*.{h,cc}
  |  user-defined functions (udf.*)                      src/udf.cc
  |  escaping: Escaper_t stack of ContentType_t          src/contenttype.*
  v
Formatter_t (whitespace modes from <?teng format?>)      src/formatter.*
  v
Writer_t (StringWriter_t / FileWriter_t / custom)        include/teng/writer.h, src/writer.cc
```

All errors go to `Error_t`. `generatePage()` returns `err.max_level`.

## Public API (`include/teng/`)

| Header | Key types / functions | Purpose |
|---|---|---|
| `teng.h` | `Teng_t`, `Teng_t::Settings_t`, `Teng_t::GenPageArgs_t`, `generatePage()`, `dictionaryLookup()`, `listSupportedContentTypes()` | Engine entry point. The positional `generatePage` overloads are `[[deprecated]]`. |
| `fragment.h` | `Fragment_t` (`addVariable`, `addIntVariable`, `addRealVariable`, `addFragment`, `addFragmentList`, `addValue`, `dump`, `json`) | Input data tree node (a map from name to `FragmentValue_t`). |
| `fragmentlist.h` | `FragmentList_t` | Repeated fragments with the same name. |
| `fragmentvalue.h` | `FragmentValue_t` | Variant: string, int, real, frag, frag list. |
| `value.h` | `Value_t` | Runtime value used by the processor and functions. |
| `writer.h` | `Writer_t`, `StringWriter_t`, `FileWriter_t` | Output sinks. Subclass `Writer_t` for a custom sink. |
| `error.h` | `Error_t` (`Level_t`: DEBUGING, WARNING, DIAG, ERROR, FATAL; `getEntries`, `dump`, `max_level`) | Error and diagnostics log. |
| `filesystem.h` | `FilesystemInterface_t`, `Filesystem_t`, `InMemoryFilesystem_t` | Template, dict and config loading. Can be injected into `Teng_t`. |
| `udf.h` | `Teng::udf::registerFunction()`, `udf::findFunction()` | User-defined functions, called from templates as `udf.name(...)`. |
| `invoke.h` | `FunctionCtx_t`, `Invoker_t` | Calling convention shared by builtins and UDFs. |
| `stringview.h` | `string_view_t`, `mutable_string_view_t`, `string_value_t` | Own string-view types (pre-`std::string_view` design). |
| `counted_ptr.h`, `stringify.h`, `structs.h`, `types.h` | helpers | Ref-counted pointer, number to string, shared typedefs. |
| `config.h.in` | `@TENG_MAJOR@`, `@TENG_MINOR@` | Generated `teng/config.h` for external users. |

A typical caller:

```cpp
Teng::Teng_t teng(root_dir);
Teng::Fragment_t data;
data.addVariable("name", "value");
auto &row = data.addFragment("row");      // <?teng frag row?>...<?teng endfrag?>
std::string out;
Teng::StringWriter_t writer(out);
Teng::Error_t err;
Teng::Teng_t::GenPageArgs_t args;
args.templateFilename = "page.html";      // or args.templateString
args.dictFilename = "dict.txt";  args.lang = "cs";   // -> dict.cs.txt
args.paramsFilename = "teng.conf";
teng.generatePage(args, data, writer, err);
```

`skin` and `lang` are inserted before the file extension by
`prependBeforeExt()` in `src/teng.cc`.

## Modules (`src/`)

| Files | Responsibility | Key types / functions |
|---|---|---|
| `teng.cc` | Implements `Teng_t` (pimpl `PTeng_t`) | `generatePage`, `prependBeforeExt` |
| `template.*` | Template assembly and caching of program, dict and config | `Template_t`, `TemplateCache_t::createTemplate` |
| `cache.*` | Generic LRU cache with change detection | `Cache_t<T>`, `CacheLRU_t`, `CacheEntry_t` |
| `sourcelist.*` | Source files a program or dict depends on (mtime checks) | `SourceList_t`, `FileStat_t` |
| `filesystem.cc` | Filesystem implementations | `Filesystem_t`, `InMemoryFilesystem_t` |
| `dictionary.*` | Language dictionary file parser and lookup | `Dictionary_t::lookup`, `parse` |
| `configuration.*` | Config file (`%enable`/`%disable` features) | `Configuration_t::is*Enabled`, `isEnabled` |
| `lex1.*` | Level-1 lexer: text versus directives | `Lex1_t` (`LEX1::TEXT/TENG/TENG_SHORT/ESC_EXPR/RAW_EXPR/DICT`), `Lex1Stack_t` |
| `lex2.*`, `lex2impl.ll`, `flexhelpers.h` | Level-2 lexer (flex, prefix `hp`) | `Lex2_t`, `Token_t`, `LEX2::*` token ids |
| `syntax.yy` | Bison grammar | `Parser::Parser_t` |
| `yystype.*` | Semantic values of grammar symbols | `Symbol_t`, `Literal_t`, `Variable_t`, `Options_t`, `NAryExpr_t` |
| `parsercontext.*` | Compile driver and parser state | `Parser::Context_t`, `compile_file`, `compile_string`, `load_file`, `next_token` |
| `parserfrag.*` | Parse-time frame and fragment tracking | `Parser::OpenFrames_t`, `FrameRec_t`, `FragRec_t` |
| `parserdiag.*` | Expression diagnostic codes | `diag_code`, `ExprDiag_t` |
| `overriddenblocks.h` | `extends`/`define`/`override` blocks | `OverriddenBlocks_t`, `ExtendsBlock_t` |
| `semantic.*` | Shared semantic helpers | |
| `semanticexpr.*` | Expressions and constant folding | `optimize_expr`, `note_optimization_point` |
| `semanticif/case/tern.*` | `if/elseif/else`, `case`, `?:` | `generate_if`, ... |
| `semanticfrag.*` | `frag`/`endfrag` | |
| `semanticvar.*` | Variables, `set`, builtin vars (`_count`, `_first`, ...) | |
| `semanticquery.*` | `defined()`, `exists()`, `count()`, `isempty()`, `type()`, `repr()`, `is*()` | |
| `semanticregex.*` | `/re/flags` literals and `=~` | |
| `semanticprint.*` | Print directives, `${}`, `%{}` | |
| `semanticblock.*`, `semanticinheritance.*` | `define`/`override`/`super` blocks, `extends` | `extends_file`, `close_extends` |
| `semanticother.*` | `include`, `format`, `ctype`, `debug`, `bytecode`, ... | `include_file` |
| `instruction.*` | Instruction set | `OPCODE`, `Instruction_t` and subclasses (`Print_t`, `Func_t`, `OpenFrag_t`, `Jmp_t`, ...), `opcode_str` |
| `program.*` | Instruction vector plus sources and errors | `Program_t` (`dump`, `isChanged`, `erase_from`) |
| `processor.*` | VM main loop, also used for compile-time `eval` | `Processor_t::run`, `Processor_t::eval` |
| `processorcontext.h` | Runtime contexts | `EvalCtx_t`, `RunCtx_t`, `GetArg_t` |
| `processorops.h` | Arithmetic, comparison and bit ops | namespace `exec` |
| `processorfrag.h` | Fragment and variable access at runtime | namespace `exec` |
| `processorother.h` | `val`, `dict`, `func`, `print`, formatter and escaper push/pop | namespace `exec` |
| `processordebug.h` | `<?teng debug?>` and `<?teng bytecode?>` output | namespace `exec` |
| `openframes.h`, `openframesapi.h` | Runtime fragment frame stack | `OpenFrames_t`, `FrameRec_t`, `ListPos_t`, `OFFApi_t` |
| `instructionpointer.h` | Sub-program handling for `define`/`call` | `InstructionPointer_t`, `SubProgram_t` |
| `function.*` | Builtin registry | `builtin_functions[]`, `findFunction` |
| `function{number,string,escaping,date,other,util}.h` | Builtin implementations | namespace `builtin` |
| `udf.cc` | UDF registry, mutex-guarded unless `NO_UDF_LOCKS` is defined (the `no-udf-locks` Meson option does not set it) | `udf::registerFunction` |
| `contenttype.*` | Content types and escaping state machines | `ContentType_t` (`find`, `listSupported`), `Escaper_t` |
| `formatter.*` | Whitespace formatting modes | `Formatter_t` (`Mode_t`, `push`, `pop`) |
| `writer.cc`, `filestream.h` | Writer implementations | `StringWriter_t`, `FileWriter_t`, `FileStream_t` |
| `error.cc`, `logging.*`, `position.*` | Error log and positions | `logError`, `logWarning`, ..., `Pos_t` |
| `regex.h` | PCRE2 wrapper | `Regex_t`, `RegexMatch_t`, `regex_flags_t` |
| `value.cc`, `fragment*.cc` | Implementations of the public data types | |
| `utf8.*`, `hex.*`, `md5.cc`, `fp.h`, `jsonutils.h`, `util.*`, `stringview.cc`, `aux.*` | Low-level helpers | |
| `debug.h`, `platform.h`, `identifier.h` | `DBG` macro, platform defines, `Identifier_t` | |

## Key concepts

- **Data tree.** `Fragment_t` is the root. `<?teng frag name?>` iterates
  over a `FragmentList_t`. Inside a fragment, builtin variables like
  `_count`, `_index`, `_first`, `_last` and `_inner` are compiled into the
  `PushFrag*_t` and `PushVal*_t` instructions.
- **Dictionary vs. configuration.** `Dictionary_t` holds `#{key}` language
  strings. `Configuration_t` extends it with `%enable`/`%disable` features:
  `debug`, `bytecode`, `errorfragment`, `logtooutput`, `watchfiles`,
  `format`, `alwaysescape`, `printescape` (needed for `%{}`) and `shorttag`
  (needed for `<? ?>`), plus `maxincludedepth` and `maxdebugvallength`.
  Test configs: `tests/teng*.conf`.
- **Caching.** `TemplateCache_t` holds three `Cache_t`s (program, dict,
  config). `watchfiles` makes it re-check `SourceList_t` mtimes. The cache
  sizes come from `Teng_t::Settings_t` (0 means the default of 50).
- **Escaping.** The content type (`GenPageArgs_t::contentType`) picks a
  `ContentType_t::Descriptor_t` from the `creators[]` table in
  `contenttype.cc`: text/plain, html/xhtml/xml, x-sh, csrc, quoted-string,
  jshtml, js, json. `<?teng ctype?>` pushes another escaper onto the
  `Escaper_t` stack.
- **Compile-time evaluation.** Constant sub-expressions are folded while
  parsing (`optimize_expr`), which runs `Context_t::coproc.eval()` (a `Processor_t`) on the partial
  program.
- **Errors.** The compiler logs into the caller's `Error_t` (passed to `Parser::Context_t`); `Program_t::getErrors()` keeps the program's own log.
  Runtime errors are logged through
  `logging.h`. With `errorfragment` enabled, templates can read the errors
  in `<?teng frag _error?>`.

## Where to change X

- **Builtin function:** implement `Result_t name(Ctx_t &ctx, const Args_t &args)`
  in the matching `src/function<category>.h` (namespace `builtin`), then
  register it in `builtin_functions[]` in `src/function.cc`. Add a test in
  `tests/fun-<category>.cc`.
- **New directive or syntax:** add the token to `src/lex2impl.ll`, the
  grammar rule to `src/syntax.yy`, and the actions to the matching
  `src/semantic*.{h,cc}`. New `src/*.cc` or `.h` files go in
  `meson.build`. For a new `<?teng`-level opener, see `src/lex1.cc`.
- **New instruction:** add it to `OPCODE` and a struct in
  `src/instruction.h`, `opcode_str` and the dump code in
  `src/instruction.cc`, a `case` in the `Processor_t` switch in
  `src/processor.cc`, and an executor in `src/processor*.h`.
- **New content type or escaper:** write a creator and add a `creators[]`
  entry in `src/contenttype.cc`. Test it in `tests/ctype.cc` or
  `tests/fun-escaping.cc`.
- **Config feature:** `src/configuration.{h,cc}` (flag, getter,
  `new_directive`).
- **Public API:** `include/teng/*.h`. New headers go in `headers` in
  `meson.build`.
- **New test:** add `tests/<feature>.cc` to `test_sources` in
  `meson.build`.

## Tests (`tests/`)

- There is a single Catch2 binary, `test-teng`. Each file is one feature
  area with its own tag: `cond`, `ctype`, `debug`, `dict`, `expr-*`
  (int, real, string, regex, case, tern, other), `format`, `frag`, `fun-*`
  (date, escaping, number, other, string), `incl`, `inheritance`,
  `queries`, `rtvars`, `vars`, `builtin-vars`, `simple`, `writer`, and
  `old` (legacy tests).
- Helpers in `tests/utils.h`: `g(templ, data, ctype)`, `g(err, templ, data,
  params, lang, ...)` and `gFromFile(err, filename, ...)`. Paths are
  relative to `TEST_ROOT` (`SRC_DIR "/tests/"`).
- Test data: `dict*.txt` (per language), `teng*.conf` (feature sets),
  `base*.html` and `override-*.html` (inheritance), `subdir/` and
  `incl.error.txt` (includes), and `text.txt`/`empty.txt`.
- `tests/fuzz.cc` provides `LLVMFuzzerTestOneInput` (it contains no
  scenarios). `fuzzer-dict.txt` and `fuzzer-coverage.sh` at the root belong
  to it. `meson.build` does not define a `teng-fuzzer` target.

## Other directories

| Path | Contents |
|---|---|
| `debian/` | Debian packaging (`libteng-dev`). `changelog` is updated with every release. |
| `gdb-pretty-printers/` | GDB Python printers for `Teng::Value_t`, `Teng::FragmentValue_t` and `Teng::string_view_t` |
| `subprojects/` | `catch2-with-main.wrap` (the only wrap) |
| `.clang-tidy` | Lint configuration used by the `clang-tidy` run target |
