#!/usr/bin/env bash

set -euo pipefail

uftrace record --no-libcall -P . \
        /opt/GCC-release/libexec/gcc/x86_64-pc-linux-gnu/17.0.0/cc1plus  \
        -quiet -imultiarch x86_64-linux-gnu -D_GNU_SOURCE ubsan-bug.cpp  \
        -quiet -dumpbase ubsan-bug.cpp -dumpbase-ext .cpp -mtune=generic \
        -march=x86-64 -std=c++26 -o /tmp/ccywqEBz.s

        # \
        # -N tree_cons \
        # -N ix86_class_max_nregs \
        # -N ix86_hard_regno_nregs \
        # -N lookup_scoped_attribute_spec \
        # -N make_node \
        # -N ggc_internal_alloc_ \
        # -N ix86_hard_regno_mode_ok \
        # \
        # -N set_decl_linkage \
        # -N decl_internal_context_p \
        # -N list_length \
        # -N auto_override_urlifier::auto_override_urlifier \
        # -N check_default_args \
        # -N auto_diagnostic_group::auto_diagnostic_group \
        # -N auto_diagnostic_group::~auto_diagnostic_group \
        # -N pushdecl \
        # -N iterative_hash \
        # -N ht_lookup \
        # -N ht_lookup_with_hash \
        # \
