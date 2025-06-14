#include "incrementer.hpp"

namespace ivl::refl {

  // represents a persistent and change-able state
  // for metaprogramming
  // API:
  // - load() -> info
  // - store(info)
  // each load() returns the info from the last store()
  // load() before any store() is not supported (compilation error)
  //
  // recommended use:
  // 1) using Cell = MemoryCell<>;
  // - Cell::load();
  // - Cell::store(i);
  // 2) static constexpr MemoryCell<> cell;
  // - cell.load();
  // - cell.store(i);
  template <typename = decltype([] {})>
  struct MemoryCell {
  private:
    using Inc = Incrementer<>;
    template <std::size_t>
    struct Map;
    template <std::meta::info>
    struct Data {};

    static consteval std::meta::info map() {
      std::size_t idx  = Inc::get();
      auto        idxi = std::meta::reflect_value(idx);
      return std::meta::substitute(^Map, {
                                           idxi});
    }

  public:
    static consteval std::meta::info load() {
      auto mapi    = map();
      auto wrapper = std::meta::type_of(std::meta::nonstatic_data_members_of(mapi)[0]);
      auto tmp     = std::meta::template_arguments_of(wrapper)[0];
      return std::meta::extract<std::meta::info>(tmp);
    }

    static consteval void store(std::meta::info i) {
      Inc::advance();
      auto mapi    = map();
      auto wrapper = std::meta::substitute(^Data, {
                                                    std::meta::reflect_value(i)});
      std::meta::define_class(mapi, {std::meta::nsdm_description(wrapper)});
    }
  };

} // namespace ivl::refl
