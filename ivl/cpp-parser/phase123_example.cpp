#include <ivl/cpp-parser/pp_tokens>
#include <ivl/cpp-parser/spliced_cxx_file>
#include <ivl/linux/utils>
#include <ivl/logger>
#include <cassert>
#include <filesystem>

int main(int argc, char* argv[]) {
  assert(argc == 2);

  ivl::spliced_cxx_file file(ivl::linux::read_file(argv[1]));
  LOG("\n" + file.post_splicing_contents);

  std::vector<std::filesystem::path> search_list{
    "/opt/GCC-REFL/lib/gcc/x86_64-pc-linux-gnu/16.0.0/../../../../include/c++/16.0.0",
    "/opt/GCC-REFL/lib/gcc/x86_64-pc-linux-gnu/16.0.0/../../../../include/c++/16.0.0/x86_64-pc-linux-gnu",
    "/opt/GCC-REFL/lib/gcc/x86_64-pc-linux-gnu/16.0.0/../../../../include/c++/16.0.0/backward",
    "/opt/GCC-REFL/lib/gcc/x86_64-pc-linux-gnu/16.0.0/include",
    "/usr/local/include",
    "/opt/GCC-REFL/include",
    "/opt/GCC-REFL/lib/gcc/x86_64-pc-linux-gnu/16.0.0/include-fixed",
    "/usr/include/x86_64-linux-gnu",
    "/usr/include",
  };

  auto state = file.parsing_start();

  std::vector<ivl::pp_token> tokens;
  while (!state.empty()) {
    std::optional<ivl::pp_token> parsed;

    if (!parsed) parsed = ivl::single_line_comment::try_parse(state);
    if (!parsed) parsed = ivl::multi_line_comment::try_parse(state);

    if (!parsed) parsed = ivl::whitespace::try_parse(state);
    if (!parsed) parsed = ivl::newline::try_parse(state);

    // TODO: spec is broken, add ud-suffix
    if (!parsed) parsed = ivl::raw_literal::try_parse(state);
    if (!parsed) parsed = ivl::preprocessing_op_or_punc::try_parse_digraph_exception_1(state);
    if (!parsed) parsed = ivl::preprocessing_op_or_punc::try_parse_digraph_exception_2(state);

    if (!parsed) parsed = ivl::preprocessing_op_or_punc::try_parse_symbolic(state);

    if (!parsed) parsed = ivl::try_parse_identifier_or_worded_op_or_punc(state);

    if (!parsed) {
      throw std::runtime_error(std::format("ICE: parsing failed\n{}", state.debug_context()));
    }

    tokens.push_back(std::move(*parsed));
  }

  for (auto&& token : tokens) {
    token.payload.visit([&]<typename T>(const T& unpacked) {
      if constexpr (std::same_as<T, ivl::newline>) {
        std::cout << "\n";
      } else if constexpr (std::same_as<T, ivl::whitespace>) {
        std::cout << unpacked.text;
      } else if constexpr (std::same_as<T, ivl::identifier>) {
        std::cout << unpacked.text;
      } else if constexpr (std::same_as<T, ivl::raw_literal>) {
        std::cout << encoding_prefix_str(unpacked.ep) << "R\"" << unpacked.delimiter << "(" << unpacked.payload << ")"
                  << unpacked.delimiter << "\"" << unpacked.ud_suffix;
      } else if constexpr (std::same_as<T, ivl::preprocessing_op_or_punc>) {
        std::cout << unpacked.kind;
      } else if constexpr (std::same_as<T, ivl::single_line_comment>) {
        std::cout << unpacked.text;
      } else if constexpr (std::same_as<T, ivl::multi_line_comment>) {
        std::cout << unpacked.text;
      } else {
        assert(false);
      }
    });
  }
}
