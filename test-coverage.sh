#!/bin/sh

ROOT="$1"
shift

BUILDIR="build/src"
if [ ! -x "${ROOT}/${BUILDIR}/test-teng" ]; then
    BUILDIR="src"
    if [ ! -x "${ROOT}/${BUILDIR}/test-teng" ]; then
        BUILDIR="build/debug/src"
        if [ ! -x "${ROOT}/${BUILDIR}/test-teng" ]; then
            echo "no such executable: ${ROOT}/src/test-teng"
            exit 1
        else
            BIN="${ROOT}/${BUILDIR}/test-teng"
            PROF_RAW="${ROOT}/${BUILDIR}/test-teng.profraw"
            PROF_DATA="${ROOT}/${BUILDIR}/test-teng.profdata"
        fi
    else
        BIN="${ROOT}/${BUILDIR}/test-teng"
        PROF_RAW="${ROOT}/${BUILDIR}/test-teng.profraw"
        PROF_DATA="${ROOT}/${BUILDIR}/test-teng.profdata"
    fi
else
    BIN="${ROOT}/${BUILDIR}/test-teng"
    PROF_RAW="${ROOT}/${BUILDIR}/test-teng.profraw"
    PROF_DATA="${ROOT}/${BUILDIR}/test-teng.profdata"
fi

OUT="${ROOT}/test-coverage-html"

rm -f "${PROF_RAW}" "${PROF_DATA}"
LLVM_PROFILE_FILE="${PROF_RAW}" "${BIN}" "$@"
llvm-profdata merge -sparse "${PROF_RAW}" -o "${PROF_DATA}"
llvm-cov show "${BIN}" -output-dir="${OUT}" -instr-profile="${PROF_DATA}" -format=html -Xdemangler c++filt -Xdemangler -n

