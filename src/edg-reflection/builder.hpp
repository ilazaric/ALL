#pragma once

#include <experimental/meta>
#include <ranges>
#include <tuple>
#include <vector>

#define FWD(x) std::forward<decltype(x)>(x)

namespace ivl::refl {

  namespace detail {

    template <typename T>
    using Fixup = std::conditional_t<std::is_reference_v<T>, T, std::remove_cv_t<T>>;

    consteval std::meta::info data_type(std::meta::info type, size_t idx) {
      return type_of(nonstatic_data_members_of(type)[idx]);
    }

    consteval std::string_view data_name(std::meta::info type, size_t idx) {
      return identifier_of(nonstatic_data_members_of(type)[idx]);
    }

    template <typename T, size_t... nsdm_idxs>
    struct Builder {
      std::tuple<Fixup<typename[:data_type(^T, nsdm_idxs):]>...> data;

      consteval {
        for (size_t idx = 0; idx < nonstatic_data_members_of(^T).size(); ++idx) {
          if ((false || ... || (idx == nsdm_idxs))) {
            queue_injection(^{
              constexpr Builder \id(data_name(^T, idx))(auto&&) const {
                static_assert(false, "Can't write to same field twice!");
                return *this;
              };
            });
          } else {
            auto left_idxs = ^{
            };
            auto right_idxs = ^{
            };
            auto left_tuple_gets = ^{
            };
            auto right_tuple_gets = ^{
            };
            for (auto [tuple_idx, nsdm_idx] :
                 std::vector<size_t> {nsdm_idxs...} | std::views::enumerate) {
              if (nsdm_idx < idx) {
                left_idxs = ^{
                  \tokens(left_idxs), \(nsdm_idx)
                };
                left_tuple_gets = ^{
                  \tokens(left_tuple_gets) std::get<\(tuple_idx)>(FWD(self).data),
                };
              } else {
                right_idxs = ^{
                  \tokens(right_idxs), \(nsdm_idx)
                };
                right_tuple_gets = ^{
                  \tokens(right_tuple_gets) std::get<\(tuple_idx)>(FWD(self).data),
                };
              }
            }
            queue_injection(^{
                constexpr Builder<T \tokens(left_idxs), \(idx) \tokens(right_idxs)> \id(data_name(^T, idx))(this auto&& self, auto&& arg){
                return {{ \tokens(left_tuple_gets) FWD(arg), \tokens(right_tuple_gets) }};
          }
        });
      }
    }

  } // namespace detail

  constexpr T build(this auto&& self) {
    consteval {
      auto concat = ^{
      };
      size_t tuple_idx = 0;
      ((concat = ^ { \tokens(concat).\id(data_name(^T, nsdm_idxs)) =
                       std::get<\(tuple_idx++)>(FWD(self).data),
        }),
       ...);
      queue_injection(^{
        return typename[: \(^T):] {\tokens(concat)};
      });
    }
  }
}; // namespace ivl::refl

} // namespace detail

template <typename T>
constexpr auto builder() {
  return detail::Builder<T> {};
}

} // namespace ivl::refl
