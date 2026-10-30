#/usr/bin/env bash

set -euo pipefail

# strace -f -o strace."$1" \

SUFFIX="$1"
if [ "$#" -ge 2 ]
then
    SUFFIX="$1.$2"
fi

if false
   then
valgrind --tool=callgrind \
         --callgrind-out-file=callgrind.out."$SUFFIX" \
/opt/GCC-release/libexec/gcc/x86_64-pc-linux-gnu/17.0.0/cc1plus \
    -quiet -imultiarch x86_64-linux-gnu -D_GNU_SOURCE \
    "$1".cpp \
    -quiet \
    -dumpbase "$SUFFIX".cpp \
    -dumpbase-ext .cpp -march=core-avx2 -O3 -std=c++26 \
    -frandom-seed=foo \
    -o "$SUFFIX".s

callgrind_annotate --show-percs=no --auto=no --threshold=100 --tree=both callgrind.out."$SUFFIX" > ann_orig."$SUFFIX"
cat ann_orig."$SUFFIX" \
    | sed 's|/home/ilazaric.*:||g' \
    | sed 's|^ *||' \
    | sed -E 's|\([0-9,]+x\)|(N)|g' \
    | cut -d ' ' -f 2- > ann_clean."$SUFFIX"

md5sum callgrind.out."$SUFFIX" ann_orig."$SUFFIX" ann_clean."$SUFFIX"
fi

        # \
        # -N dom_info_state \
        # -N 'wide_int_storage::operator=' \
        # -N parse_N_M \
        # -N htab_hash_string \
        # -N bitmap_clear \
        # -N bitmap_set_bit \
        # -N shift_cost \
        # -N gen_int_shift_amount \
        # -N gen_int_mode \
        # -N gen_rtx_CONST_INT \
        # -N 'operator new' \
        # -N vector_builder::elt \
        # -N concat \
        # \
        # -N '.*alloc' \
        # -N '.*_p' \
        # \
        # -N pushdecl \
        # -N recog_350 \
        # -N hash_table::find_slot_with_hash \
        # -N def_builtin \
        # -N xmalloc \
        # -N recog_18 \
        # -N recog_22 \
        # -N recog_468 \
        # -N recog \
        # -N ggc_internal_alloc \
        # \
        # -N ht_lookup \
        # -N bitmap_bit_p \
        # -N list_length \
        # -N stringpool_ggc_alloc \
        # -N ht_lookup_with_hash \
        # -N ix86_multiplication_cost \
        # -N tree_cons \
        # -N decl_internal_context_p \
        # -N lookup_scoped_attribute_spec \
        # -N find_attribute_namespace \
        # -N general_operand \
        # -N set_mode_and_regno \
        # -N iterative_hash \
        # -N make_node \
        # -N ix86_hard_regno_nregs \
        # -N x86_64_immediate_operand \
        # -N x86_64_immediate_operand_1 \
        # -N trunc_int_for_mode \
        # -N ix86_vec_cost \
        # -N ix86_class_max_nregs \
        # -N inline_secondary_memory_needed \
        # -N ix86_register_move_cost \
        # -N ggc_internal_alloc \
        # -N inline_memory_move_cost \
        # -N rtx_cost \
        # -N ix86_hard_regno_mode_ok \
        # -N reg_classes_intersect_p \
        # -N reg_class_subset_p \
        # \

uftrace record \
        --no-libcall \
        -P . \
        -d uftrace.data."$SUFFIX" \
/opt/GCC-release/libexec/gcc/x86_64-pc-linux-gnu/17.0.0/cc1plus \
    -quiet -imultiarch x86_64-linux-gnu -D_GNU_SOURCE \
    "$1".cpp \
    -quiet \
    -dumpbase "$SUFFIX".cpp \
    -dumpbase-ext .cpp -march=core-avx2 -O3 -std=c++26 \
    -frandom-seed=foo \
    -o "$SUFFIX".s

uftrace replay -d uftrace.data."$SUFFIX" -f none --no-merge --no-event > etrace."$SUFFIX"

md5sum etrace."$SUFFIX"
wc -l etrace."$SUFFIX"
