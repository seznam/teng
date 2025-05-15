project(
  'libteng',
  'cpp',
  version: '5.0.5',
  default_options : [
    'cpp_std=c++17',
    'warning_level=2',
    'buildtype=debugoptimized',
    'wrap_mode=nofallback',
    'force_fallback_for=catch2-with-main',
  ],
  meson_version: '>= 1.1.0'
)

install_files = not meson.is_subproject()

dependencies = [
  dependency('threads'),
  dependency('dl', required: false),
  dependency('libpcre2-8'),
  dependency('glib-2.0'),
  dependency('boost', modules: ['system', 'filesystem']),
]

includes = include_directories(
  'include',
  'src',
)

headers = [
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
  'include/teng/writer.h',
]

sources = [
  'src/aux.cc',
  'src/aux.h',
  'src/cache.cc',
  'src/cache.h',
  'src/configuration.cc',
  'src/configuration.h',
  'src/contenttype.cc',
  'src/contenttype.h',
  'src/dictionary.cc',
  'src/dictionary.h',
  'src/error.cc',
  'src/filestream.h',
  'src/filesystem.cc',
  'src/flexhelpers.h',
  'src/formatter.cc',
  'src/formatter.h',
  'src/fp.h',
  'src/fragment.cc',
  'src/fragmentlist.cc',
  'src/fragmentvalue.cc',
  'src/function.cc',
  'src/function.h',
  'src/functiondate.h',
  'src/functionescaping.h',
  'src/functionnumber.h',
  'src/functionother.h',
  'src/functionstring.h',
  'src/functionutil.h',
  'src/hex.cc',
  'src/hex.h',
  'src/identifier.h',
  'src/instruction.cc',
  'src/instruction.h',
  'src/instructionpointer.h',
  'src/jsonutils.h',
  'src/lex1.cc',
  'src/lex1.h',
  'src/lex2.cc',
  'src/lex2.h',
  'src/logging.cc',
  'src/logging.h',
  'src/md5.cc',
  'src/openframes.h',
  'src/openframesapi.h',
  'src/overriddenblocks.h',
  'src/parsercontext.cc',
  'src/parsercontext.h',
  'src/parserdiag.cc',
  'src/parserdiag.h',
  'src/parserfrag.cc',
  'src/parserfrag.h',
  'src/platform.h',
  'src/position.cc',
  'src/position.h',
  'src/processor.cc',
  'src/processor.h',
  'src/processorcontext.h',
  'src/processordebug.h',
  'src/processorfrag.h',
  'src/processorops.h',
  'src/processorother.h',
  'src/program.cc',
  'src/program.h',
  'src/regex.h',
  'src/semantic.cc',
  'src/semantic.h',
  'src/semanticblock.cc',
  'src/semanticblock.h',
  'src/semanticcase.cc',
  'src/semanticcase.h',
  'src/semanticexpr.cc',
  'src/semanticexpr.h',
  'src/semanticfrag.cc',
  'src/semanticfrag.h',
  'src/semanticif.cc',
  'src/semanticif.h',
  'src/semanticinheritance.cc',
  'src/semanticinheritance.h',
  'src/semanticother.cc',
  'src/semanticother.h',
  'src/semanticprint.cc',
  'src/semanticprint.h',
  'src/semanticquery.cc',
  'src/semanticquery.h',
  'src/semanticregex.cc',
  'src/semanticregex.h',
  'src/semantictern.cc',
  'src/semantictern.h',
  'src/semanticvar.cc',
  'src/semanticvar.h',
  'src/sourcelist.cc',
  'src/sourcelist.h',
  'src/stringview.cc',
  'src/template.cc',
  'src/template.h',
  'src/teng.cc',
  'src/udf.cc',
  'src/utf8.cc',
  'src/utf8.h',
  'src/util.cc',
  'src/util.h',
  'src/value.cc',
  'src/writer.cc',
  'src/yystype.cc',
  'src/yystype.h',
]

test_sources = [
  'src/tests/builtin-vars.cc',
  'src/tests/cond.cc',
  'src/tests/ctype.cc',
  'src/tests/debug.cc',
  'src/tests/dict.cc',
  'src/tests/expr-case.cc',
  'src/tests/expr-int.cc',
  'src/tests/expr-other.cc',
  'src/tests/expr-real.cc',
  'src/tests/expr-regex.cc',
  'src/tests/expr-string.cc',
  'src/tests/expr-tern.cc',
  'src/tests/format.cc',
  'src/tests/frag.cc',
  'src/tests/fun-date.cc',
  'src/tests/fun-escaping.cc',
  'src/tests/fun-number.cc',
  'src/tests/fun-other.cc',
  'src/tests/fun-string.cc',
  'src/tests/fuzz.cc',
  'src/tests/incl.cc',
  'src/tests/inheritance.cc',
  'src/tests/old.cc',
  'src/tests/queries.cc',
  'src/tests/rtvars.cc',
  'src/tests/simple.cc',
  'src/tests/vars.cc',
  'src/tests/utils.h',
]

generated_sources = []

# can't use configure_file() because stupid meson restriction
# 'Output must not contain a path segment.' error
# the solution is to use custom_target() and run a python script
# to generate the file where meson wants it and also its copy
# where the header file should be
generated_sources += custom_target(
  'config.h',
  input: 'include/teng/config.h.in',
  output: 'config.h',
  install_dir: 'include/teng',
  install: true,
  command: [
    'python',
    '-c',
    'import sys, os;'
    + 'cdata = open(sys.argv[1], "r").read();'
    + 'cdata.replace("@TENG_MAJOR@", sys.argv[4]);'
    + 'cdata.replace("@TENG_MINOR@", sys.argv[5]);'
    + 'open(sys.argv[2], "w").write(cdata);'
    + 'os.makedirs(sys.argv[3] + "", exist_ok=True);'
    + 'open(sys.argv[3] + "/config.h", "w").write(cdata);',
    '@INPUT@',
    '@OUTPUT@',
    meson.project_build_root() / 'include/teng',
    meson.project_version().split('.')[0],
    meson.project_version().split('.')[1],
  ],
)

flex = find_program('flex')
generated_sources += custom_target(
  'tenglex2',
  input: ['src/lex2impl.ll'],
  output: ['lex2impl.cc', 'lex2impl.h'],
  command: [flex, '-Php', '-o', '@OUTPUT0@', '--header-file=@OUTPUT1@', '@INPUT0@']
)

bison = find_program('bison')
generated_sources += custom_target(
  'tengsyntax',
  input: ['src/syntax.yy'],
  output: ['syntax.cc', 'syntax.hh'],
  command: [bison, '-o', '@OUTPUT0@', '@INPUT@']
)

git_revision = 'no revision'
git = find_program('git', required: false)
fs = import('fs')
if git.found() and fs.is_dir('.git')
    res = run_command('git', 'describe', '--all', '--dirty', '--long', '--abbrev=40', check: true)
    git_revision = res.stdout().strip()
endif

add_project_arguments(
  '-DPACKAGE_VERSION="' + meson.project_version() + '"',
  language: ['c', 'cpp']
)

add_project_arguments(
  '-DPACKAGE_NAME="' + meson.project_name() + '"',
  language: ['c', 'cpp']
)

add_project_arguments(
  '-DGIT_REVISION="' + git_revision + '"',
  language: ['c', 'cpp']
)

add_project_arguments(
  '-DSRC_DIR="' + meson.project_source_root() + '"',
  language: ['c', 'cpp']
)

if get_option('optimization') not in ['1', '2', '3']
  add_project_arguments(
    '-DDEBUG',
    language: ['c', 'cpp']
  )
endif

if get_option('debug')
  add_project_arguments(
    '-ggdb',
    language: ['c', 'cpp']
  )
endif

cxx = meson.get_compiler('cpp')
add_project_arguments(
  cxx.get_supported_arguments(
    [
      '-Wconversion',
      '-Wno-sign-conversion',
      '-Wnon-virtual-dtor',
      '-Woverloaded-virtual',
      '-Wctor-dtor-privacy',
      '-fno-omit-frame-pointer',
      '-ftemplate-backtrace-limit=0',
      '-Wextra-semi',
      '-Wuseless-cast',
      '-Wzero-as-null-pointer-constant',
      '-Wundef',
      '-Wlogical-op',
      '-Wpointer-arith',
      '-Wcast-qual',
      '-Wformat=2',
      '-Wformat-truncation',
      '-Wdisabled-optimization',
      '-Wnull-dereference',
      '-Wduplicated-cond',
      '-fdiagnostics-color=always',
      '-Werror=format',
    ]
  ),
  language: 'cpp'
)

lib = static_library(
  'teng',
  [generated_sources, sources],
  dependencies: dependencies,
  install: install_files,
  include_directories: includes,
  link_args: '-rdynamic'
)

if install_files
  install_headers(
    headers,
    subdir: '..',
    preserve_path: true
  )
endif

pkg = import('pkgconfig')
pkg.generate(lib, name: 'libteng')

libteng_dep = declare_dependency(
  include_directories: includes,
  dependencies: dependencies,
  link_with: lib
)
meson.override_dependency('libteng', libteng_dep)

catch2_with_main = subproject('catch2-with-main', default_options: ['install=false'])
catch2_with_main_dep = catch2_with_main.get_variable('catch2_with_main_dep')

test(
  'test-teng',
  executable(
    'test-teng',
    test_sources,
    include_directories: [includes, 'src/tests'],
    dependencies: [
      libteng_dep,
      catch2_with_main_dep,
    ],
    install: false
  ),
)

# test_ssl = executable(
#   'server',
#   test_ssl_sources,
#   dependencies: libmtd_dep,
#   install: false)
#
# run_sh = find_program(
#   meson.current_source_dir() + '/test/ssl/run.sh'
# )
#
# conf_data = configuration_data()
# conf_data.set('TEST_DIR', meson.current_build_dir())
# configure_file(
#   output: 'mtd.conf',
#   input: 'test/ssl/mtd.conf.in',
#   configuration : conf_data
# )
#
# test(
#   'test_ssl',
#   run_sh,
#   depends: test_ssl,
#   timeout: 240,
#   env: [
#     'TEST_DIR=' + meson.current_source_dir(),
#     'EXE_DIR=' + meson.current_build_dir()
#   ],
#   is_parallel : false
# )
#
# test_network = executable(
#   'server-network',
#   test_network_sources,
#   dependencies: libmtd_dep,
#   install: false
# )
#
# run_network_sh = find_program(
#   meson.current_source_dir() + '/test/network/run.sh'
# )
#
# test(
#   'test_network',
#   run_network_sh,
#   depends: test_network,
#   timeout: 20,
#   env: [
#     'TEST_DIR=' + meson.current_source_dir(),
#     'EXE_DIR=' + meson.current_build_dir(),
#     'SOURCE_DIR=' + meson.project_source_root()
#   ]
# )

clang_tidy = find_program('clang-tidy', required: false)
if clang_tidy.found()
  input = files(sources + headers)
  command = [clang_tidy, '-quiet', '-p', meson.project_build_root(), '-header-filter', '^$']
  run_target('clang-tidy', command: command + input)
  run_target('clang-tidy-fix', command: command + ['-fix'] + input)
endif

if (get_option('docs'))
  doxygen = find_program('doxygen', required: true)
  dot = find_program('dot', required: true)

  input = [meson.project_source_root() / 'doc']
  foreach file: sources + headers
    input += meson.project_source_root() / file
  endforeach

  conf_data = configuration_data()
  conf_data.set('PROJECT', meson.project_name())
  conf_data.set('VERSION', meson.project_version())
  conf_data.set_quoted('REVISION', git_revision)
  conf_data.set('SRCDIR', meson.project_source_root())
  conf_data.set('DOT_PATH', dot.full_path())
  conf_data.set('OUTPUT', meson.project_build_root())
  conf_data.set('INPUT', ' '.join(input))

  doxyfile = configure_file(
    input: 'Doxyfile.in',
    output: 'Doxyfile',
    configuration: conf_data
  )
  run_target('docs', command: [doxygen, doxyfile])
endif
