#pragma once

#include <ranges>
#include <vector>
#include <algorithm>

// emacs hates the syntax
#define SLASH(...) \(__VA_ARGS__)

namespace ivl::ctx {

  namespace detail {

    // "does big contain all elements of small?"
    consteval contains(const std::vector<std::meta::info>& big,
                       const std::vector<std::meta::info>& small){
      for (auto s : small){
        if (std::ranges::find(big, s) == big.end())
          return false;
      }
      return true;
    }

    constexpr std::string to_string(size_t num){
      if (num == 0) return "0";
      std::string out;
      for (; num; num /= 10) out += (char)(out % 10 + '0');
      std::ranges::reverse(out);
      return out;
    }

    // std::tuple<Ts&...> is not trivially copyable
    template<typename... Ts>
    struct reftuple {
      // Ts&... _$idx;
      consteval {
        size_t idx = 0;
        auto creator = [&]<typename T>{
          queue_injection(^{
              typename [:SLASH(^T):]& \id("_"sv, to_string(idx));
            });
          ++idx;
        };
        (creator.operator()<Ts>(), ...);
      }

      
    };
    
  } // detail

  // TODO: order doesn't matter, would be nice if I sorted types to reduce class instantiations
  //       that might also reduce conversions sometimes
  template<typename... Ts>
  class ContextClass {
    std::tuple<Ts&...> refs;

  public:
    template<typename T>
    constexpr T& get()
      requires(detail::contains({^Ts...}, {^T}))
    {
      return std::get<T&>(refs);
    }

    template<typename... Us>
    requires(!detail::contains({^Ts...}, {^Us}) && ...)
    constexpr ContextClass<Ts..., Us...> put(Us&... newrefs){
      return {std::get<Ts>(refs)..., newrefs...};
    }

    template<typename... Us>
    requires(detail::contains({^Ts...}, {^Us...}))
    constexpr operator ContextClass<Us...> (){
      return {std::get<Us&>(refs), ...};
    }
  };

  template<typename... Ts>
  ContextClass(Ts&...) -> ContextClass<Ts...>;

  template<typename T>
  concept Context = has_template_arguments(^T) && template_of(^T) == ^ContextClass;
  
  template<typename T, typename... Us>
  concept ContextWith = Context<T> && detail::contains(template_arguments_of(^T), {^Us...});
  
} // namespace ivl::ctx
