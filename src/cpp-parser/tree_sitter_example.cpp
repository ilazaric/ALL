// ---------------- src/gram.cpp ----------------------------------------
#include "tree-sitter/api.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <ivl/logger>
#include <sstream>

extern "C" const TSLanguage* tree_sitter_latex(); // generated symbol

// Helper: load an UTF‑8 file into a std::string
static std::string slurp(const char* path) {
  std::ifstream      in(path, std::ios::binary);
  std::ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " source/std-gram.ext\n";
    return 1;
  }
  const std::string tex = slurp(argv[1]);

  std::cerr << __LINE__ << std::endl;

  // 1. Set up the parser ------------------------------------------------
  TSParser* parser = ts_parser_new();
  ts_parser_set_language(parser, tree_sitter_latex());

  TSTree* tree = ts_parser_parse_string(parser, // keep NULL because we do a full parse
                                        NULL, tex.c_str(), tex.size());

  auto root_node = ts_tree_root_node(tree);

  LOG(ts_node_child_count(root_node));
  LOG(ts_node_named_child_count(root_node));

  auto root_node_child_count = ts_node_named_child_count(root_node);
  for (uint32_t root_child_idx = 0; root_child_idx < root_node_child_count; ++root_child_idx) {
    LOG(root_child_idx);
    const auto printer = [&](this auto&& self, const TSNode node, size_t depth = 0) {
      if (ts_node_named_child_count(node) == 0) {
        size_t from = ts_node_start_byte(node);
        size_t to   = ts_node_end_byte(node);
        std::cout << std::string(depth, '+');
        std::cout.write(tex.data() + from, to - from);
        std::cout << std::endl;
        return;
      } else {
        auto len = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < len; ++i)
          self(ts_node_named_child(node, i), depth + 1);
      }
    };
    auto child_node = ts_node_named_child(root_node, root_child_idx);
    printer(child_node);
    // LOG(ts_node_named_child_count(child_node));
    // size_t from = ts_node_start_byte(child_node);
    // size_t to   = ts_node_end_byte(child_node);
    // std::cerr.write(tex.data() + from, to - from);
    // std::cerr << "\n\n";
    // LOG(ts_node_named_child_count(child_node));
    // auto bla = ts_node_named_child_count(child_node);
    // for (uint32_t i = 0; i < bla; ++i){
    //   auto child2_node = ts_node_named_child(child_node, i);
    //   from = ts_node_start_byte(child2_node);
    //   to   = ts_node_end_byte(child2_node);
    //   std::cerr << "+";
    //   std::cerr.write(tex.data() + from, to - from);
    //   std::cerr << "\n";
    // }
    std::cout << "\n";
  }

  ts_tree_delete(tree);
  ts_parser_delete(parser);
  return 0;

  const char* query_source = "(generic_environment                     " // whole block
                             "  (begin                                 " // its \begin
                             "    (curly_group_text) @env_name)        " //   {bnf}, {ncbnf}, …
                             ") @env";                                   // capture the whole env

  // -------- create the query with proper diagnostics ------------
  uint32_t     error_offset = 0;
  TSQueryError error_type   = TSQueryErrorNone;

  TSQuery* query = ts_query_new(tree_sitter_latex(), query_source, std::strlen(query_source),
                                &error_offset, &error_type);

  if (!query) {
    std::cerr << "TSQuery compile error at byte " << error_offset << " (" << (error_type) << ")\n";
    return 1;
  }

  TSQueryCursor* qcur = ts_query_cursor_new();
  ts_query_cursor_exec(qcur, query, ts_tree_root_node(tree));

  size_t       env_count = 0;
  TSQueryMatch m;
  while (ts_query_cursor_next_match(qcur, &m)) {
    ++env_count;
    std::string env_name;
    TSNode      env_node {};

    // Walk the captures for this match
    for (uint32_t i = 0; i < m.capture_count; ++i) {
      const TSQueryCapture& cap      = m.captures[i];
      uint32_t              length   = -1;
      const char*           cap_name = ts_query_capture_name_for_id(query, cap.index, &length);

      if (strcmp(cap_name, "env_name") == 0) {
        env_name.assign(tex.data() + ts_node_start_byte(cap.node),
                        ts_node_end_byte(cap.node) - ts_node_start_byte(cap.node));
      } else if (strcmp(cap_name, "env") == 0) {
        env_node = cap.node; // whole \begin…\end block
      }
    }

    // Only keep the grammar environments we care about
    if (env_name == "{bnf}" || env_name == "{ncbnf}") {
      size_t from = ts_node_start_byte(env_node);
      size_t to   = ts_node_end_byte(env_node);
      std::cout.write(tex.data() + from, to - from);
      std::cout << "\n\n";
    }
  }

  LOG(env_count);

  // 4. Clean‑up ---------------------------------------------------------
  ts_query_cursor_delete(qcur);
  ts_query_delete(query);
  ts_tree_delete(tree);
  ts_parser_delete(parser);
}
