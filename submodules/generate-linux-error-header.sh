#!/usr/bin/env bash

SCRIPT_DIR="$(dirname "$(realpath "$0")")"
cd "$SCRIPT_DIR"

set -euo pipefail

gcc -undef -E -P -fdirectives-only -isystem linux/include/uapi -include asm-generic/errno.h -x c /dev/null -o /dev/stdout \
    | grep -E '^#define E' \
    | awk 'BEGIN{print "#pragma once"}
           BEGIN{print ""}
           BEGIN{print "// generated with generate-linux-error-header.sh"}
           BEGIN{print ""}
           BEGIN{print "#include <ivl/utility/enum>"}
           BEGIN{print "#include <cstdint>"}
           BEGIN{print ""}
           BEGIN{print "namespace ivl::linux {"}
           BEGIN{print "enum class syscall_error_raw : int16_t {"}
           {print "  " $2 " = " $3 ","}
           END{print "};"}
           END{print ""}
           END{print "using syscall_error = checked_enum<syscall_error_raw>;"}
           END{print "} // namespace ivl::linux"}'
