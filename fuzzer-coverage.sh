#!/bin/sh

ROOT="$1"
shift

BUILDIR="build/src"
if [ ! -x "${ROOT}/${BUILDIR}/teng-fuzzer" ]; then
    BUILDIR="src"
    if [ ! -x "${ROOT}/${BUILDIR}/teng-fuzzer" ]; then
        BUILDIR="build/fuzz/src"
        if [ ! -x "${ROOT}/${BUILDIR}/teng-fuzzer" ]; then
            echo "no such executable: ${ROOT}/src/teng-fuzzer"
            exit 1
        else
            BIN="${ROOT}/${BUILDIR}/teng-fuzzer"
            PROF_RAW="${ROOT}/${BUILDIR}/teng-fuzzer.profraw"
            PROF_DATA="${ROOT}/${BUILDIR}/teng-fuzzer.profdata"
        fi
    else
        BIN="${ROOT}/${BUILDIR}/teng-fuzzer"
        PROF_RAW="${ROOT}/${BUILDIR}/teng-fuzzer.profraw"
        PROF_DATA="${ROOT}/${BUILDIR}/teng-fuzzer.profdata"
    fi
else
    BIN="${ROOT}/${BUILDIR}/teng-fuzzer"
    PROF_RAW="${ROOT}/${BUILDIR}/teng-fuzzer.profraw"
    PROF_DATA="${ROOT}/${BUILDIR}/teng-fuzzer.profdata"
fi

OUT="${ROOT}/fuzzer-coverage-html"

rm -f "${PROF_RAW}" "${PROF_DATA}"
LLVM_PROFILE_FILE="${PROF_RAW}" "${BIN}" "$@"
llvm-profdata merge -sparse "${PROF_RAW}" -o "${PROF_DATA}"
llvm-cov show "${BIN}" -output-dir="${OUT}" -instr-profile="${PROF_DATA}" -format=html -Xdemangler c++filt -Xdemangler -n -ignore-filename-regex=".*glib-2.0.*"

