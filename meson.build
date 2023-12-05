project('teng', 'cpp',
        version: '5.0.5',
        default_options : ['cpp_std=c++14',
                           'warning_level=2',
                           'buildtype=debugoptimized',
                           'wrap_mode=nofallback'
                          ])

# NOTE:
# sanitizations can be invoked in meson setup step via -Db_sanitize=...
# coverage can be enabled with meson's builtin -Db_coverage=true
# fuzzer is enabled with -Dfuzzer=true

pkg = import('pkgconfig')

lib_deps = [
  dependency('glib-2.0'),
  dependency('libpcre2-8')
]

add_project_arguments('-D_ISOC99_SOURCE', language: 'cpp')

flex = find_program('flex')
bison = find_program('bison')

# tengsyntax.[hh, cc] is generated via bison from tengsyntax.yy
tengsyntax = custom_target('tengsyntax',
                           input: ['src/syntax.yy'],
                           output: ['syntax.cc', 'syntax.hh'],
                           command: [bison, '-o', '@OUTPUT0@', '@INPUT@'])

# tenglex2.cc is generated via flex from tenglex2.ll and tengsyntax.[hh,cc]
tenglex2 = custom_target('tenglex2',
                         input: ['src/lex2impl.ll', tengsyntax],
                         output: 'lex2impl.cc',
                         command: [flex, '-Php', '-o',
                                   '@OUTPUT@', '@INPUT0@'])

generated_sources = [tengsyntax, tenglex2]

# source code
src = [
  'src/teng.cc',
  'src/fragment.cc',
  'src/fragmentlist.cc',
  'src/util.cc',
  'src/fragmentvalue.cc',
  'src/sourcelist.cc',
  'src/dictionary.cc',
  'src/instruction.cc',
  'src/value.cc',
  'src/parsercontext.cc',
  'src/program.cc',
  'src/function.cc',
  'src/processor.cc',
  'src/writer.cc',
  'src/formatter.cc',
  'src/cache.cc',
  'src/contenttype.cc',
  'src/error.cc',
  'src/template.cc',
  'src/lex1.cc',
  'src/position.cc',
  'src/udf.cc',
  'src/yystype.cc',
  'src/md5.cc',
  'src/configuration.cc',
  'src/aux.cc',
  'src/logging.cc',
  'src/semantic.cc',
  'src/hex.cc',
  'src/utf8.cc',
  'src/stringview.cc',
  'src/lex2.cc',
  'src/parserfrag.cc',
  'src/parserdiag.cc',
  'src/semanticblock.cc',
  'src/semanticcase.cc',
  'src/semanticexpr.cc',
  'src/semanticfrag.cc',
  'src/semanticif.cc',
  'src/semanticother.cc',
  'src/semanticprint.cc',
  'src/semanticquery.cc',
  'src/semanticregex.cc',
  'src/semantictern.cc',
  'src/semanticvar.cc',
  'src/semanticinheritance.cc',
  'src/filesystem.cc'
]

headers = [
  'include/teng/config.h.in',
  'include/teng/counted_ptr.h',
  'include/teng/error.h',
  'include/teng/filesystem.h',
  'include/teng/fragment.h',
  'include/teng/fragmentlist.h',
  'include/teng/fragmentvalue.h',
  'include/teng/invoke.h',
  'include/teng/stringify.h',
  'include/teng/stringview.h',
  'include/teng/structs.h',
  'include/teng/teng.h',
  'include/teng/udf.h',
  'include/teng/value.h',
  'include/teng/writer.h'
]

# '.' is for generated files
inc = include_directories('src')

fuzzer_build = get_option('fuzzer')

if not fuzzer_build
     lib = library('teng',
                   sources: [src, generated_sources],
                   install: true,
                   include_directories: inc,
                   dependencies: lib_deps)

  # teng5 installs files into subdir of /usr/include
  install_headers(headers, subdir: 'teng', preserve_path: false)

  pkg.generate(lib, name: 'libteng')

  libteng_dep = declare_dependency(include_directories : inc,
                                   dependencies: lib_deps,
                                   link_with : lib)

  # >= 0.54
  if meson.version().version_compare('>=0.54.0')
    meson.override_dependency('libteng', libteng_dep)
  endif


  # tests
  test_sources = [
    'src/tests/main.cc',
    'src/tests/old.cc',
    'src/tests/fun-escaping.cc',
    'src/tests/frag.cc',
    'src/tests/vars.cc',
    'src/tests/simple.cc',
    'src/tests/incl.cc',
    'src/tests/format.cc',
    'src/tests/expr-string.cc',
    'src/tests/expr-int.cc',
    'src/tests/expr-real.cc',
    'src/tests/expr-other.cc',
    'src/tests/dict.cc',
    'src/tests/expr-regex.cc',
    'src/tests/ctype.cc',
    'src/tests/cond.cc',
    'src/tests/expr-tern.cc',
    'src/tests/expr-case.cc',
    'src/tests/fun-date.cc',
    'src/tests/fun-number.cc',
    'src/tests/fun-string.cc',
    'src/tests/fun-other.cc',
    'src/tests/queries.cc',
    'src/tests/debug.cc',
    'src/tests/rtvars.cc',
    'src/tests/builtin-vars.cc',
    'src/tests/inheritance.cc'
  ]

  test_teng = executable('test-teng',
                         sources: [test_sources],
                         dependencies: libteng_dep,
                         cpp_args: '-DSRC_DIR="' + meson.source_root() + '"')

  test('test-teng', test_teng, workdir: meson.source_root())
else
  # fuzzer
  fuzz_args = [
    '-fsanitize=fuzzer'
  ]

  libfuzzer = meson.get_compiler('cpp').find_library('Fuzzer', required: false)

  fuzzer = executable('teng-fuzzer',
                      sources: ['src/tests/fuzz.cc', src, generated_sources],
                      dependencies: [lib_deps, libfuzzer],
                      cpp_args: fuzz_args,
                      link_args: fuzz_args,
                      include_directories: inc)
endif
