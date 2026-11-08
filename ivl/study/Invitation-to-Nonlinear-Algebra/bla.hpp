#pragma once

#include <algorithm>
#include <meta>
#include <ranges>
#include <tuple>
#include <vector>

// TODO: failed experiment, but maybe can be recovered with
// ....: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0672r0.pdf

template<typename... Ts>
struct vector_tie {
  struct storage_layout;
  struct reference_layout;
  struct const_reference_layout;
  struct iterator_layout;
  struct const_iterator_layout;

  template<typename T>
  using get_iterator = T::iterator;
  template<typename T>
  using get_const_iterator = T::const_iterator;

  consteval {
    std::vector<std::meta::info> storage_vec;
    std::vector<std::meta::info> reference_vec;
    std::vector<std::meta::info> const_reference_vec;
    std::vector<std::meta::info> iterator_vec;
    std::vector<std::meta::info> const_iterator_vec;
    for (auto t : {^^Ts...}) {
      storage_vec.push_back(data_member_spec(t, {}));

      t = remove_reference(t);
      bool is_const = is_const_type(t);
      if (is_const) remove_const(t);

      iterator_vec.push_back(
        data_member_spec(decay(substitute(is_const ? (^^get_const_iterator) : (^^get_iterator), {t})), {})
      );
      const_iterator_vec.push_back(data_member_spec(decay(substitute((^^get_const_iterator), {t})), {}));

      t = template_arguments_of(t)[0];
      if (is_const) add_const(t);
      t = add_lvalue_reference(t);
      reference_vec.push_back(data_member_spec(t, {}));
      t = remove_reference(t);
      t = add_const(t);
      t = add_lvalue_reference(t);
      const_reference_vec.push_back(data_member_spec(t, {}));
    }
    define_aggregate(^^storage_layout, storage_vec);
    define_aggregate(^^iterator_layout, iterator_vec);
    define_aggregate(^^const_iterator_layout, const_iterator_vec);
    define_aggregate(^^reference_layout, reference_vec);
    define_aggregate(^^const_reference_layout, const_reference_vec);
  }

  struct reference {
    reference_layout l;

    friend void swap(reference& a, reference& b) {
      using std::swap;
      auto& [... as] = a.l;
      auto& [... bs] = b.l;
      (swap(as, bs), ...);
    }

    bool operator==(const reference&) const = default;
  };

  struct const_reference {
    const_reference_layout l;
    bool operator==(const const_reference&) const = default;
  };

  friend reference& operator=(reference& a, const_reference b) { return a; }

  struct iterator {
    iterator_layout l;

    reference operator*() const {
      auto&& [... its] = l;
      return reference{*its...};
    }

    bool operator==(const iterator&) = default;

    auto operator<=>(const iterator& o) const {
      if constexpr (sizeof...(Ts) == 0) {
        return std::strong_ordering::equal;
      } else {
        auto&& [... its] = l;
        auto&& [... oits] = o.l;
        return its...[0] <=> oits...[0];
      }
    }
  };

  storage_layout l;

  vector_tie(Ts... vs) : l{vs...} {
    if constexpr (sizeof...(Ts) != 0) {
      auto sz = vs...[0].size();
      contract_assert((vs.size() == sz) && ...);
    }
  }

  auto range() {
    auto& [... vs] = storage;
    return std::views::zip(vs...);
  }

  auto range() const {
    auto& [... vs] = storage;
    return std::views::zip(std::as_const(vs)...);
  }

  auto begin(this auto&& self) { return range().begin(); }
  auto end(this auto&& self) { return range().end(); }

  void push_back(auto&& arg) {
    auto&& [... els] = FWD(arg);
    auto&& [... vs] = storage;
    (vs.push_back(FWD(els)), ...);
  }
};

template<typename... Ts>
vector_tie(Ts&&...) -> vector_tie<Ts&...>;
