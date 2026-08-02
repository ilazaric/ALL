#include <ivl/utility>
#include "define_aggregate_with_member_functions"
#include <format>
#include <print>
#include <ranges>

// IVL add_compiler_flags("-Wno-subobject-linkage")

struct foo;

consteval {
  ivl::reflection::define_aggregate_with_member_functions(
    (^^foo), //
    {
      data_member_spec((^^std::vector<uint64_t>), {.name = "data"}),
      data_member_spec((^^std::vector<std::vector<uint64_t>>), {.name = "history"}),
      data_member_spec((^^uint64_t), {.name = "bump_count"}),
      data_member_spec((^^uint64_t), {.name = "length_sum"}),
    },
    {
      {"current_length", [](auto&& self) { return self.data.size(); }},
      {
        "bump",
        ivl::util::Overload{
          [](auto&& self) {
            self.history.push_back(self.data);
            uint64_t curr = 0;
            while (!self.data.empty() && self.data.back() == curr) {
              self.data.pop_back();
              ++curr;
            }
            self.data.push_back(curr);
            self.length_sum += self.current_length();
            ++self.bump_count;
          },
          [](auto&& self, uint64_t n) {
            while (n--) self.bump();
          }
        },
      },
      {"dump", [](auto&& self) {
         auto hist = [&] {
           std::string ret;
           for (auto&& el : self.history) ret += std::format("  {},\n", el);
           return ret;
         };
         auto strify = [&] {
           return std::format(
             "\nbump_count: {}\nlength_sum: {}\ndata: {}\nhistory: [\n{}]", self.bump_count, self.length_sum, self.data,
             hist()
           );
         };
         if consteval {
           __builtin_constexpr_diag(32, "", strify());
         } else {
           std::println("{}", strify());
         }
       }},
    }
  );
}

consteval {
  foo f{};
  f.bump(10);
  f.dump();
}

int ivl_main(int x) {
  foo f{};
  f.bump(x);
  f.dump();
  return 0;
}
