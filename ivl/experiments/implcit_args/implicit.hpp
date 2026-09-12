#include <meta>
#include <format>
#include <ranges>
#include <string_view>
#include <charconv>
#include <stdexcept>
#include <exception>
#include <optional>

// basic friend injection
template<typename>
struct injection_declaration {
    friend consteval auto injection_function(injection_declaration);
};

template<typename T, std::meta::info V>
requires (V != std::meta::info{}) // null reflection represents "unset"
struct injection_definition {
    friend consteval auto injection_function(injection_declaration<T>) { return V; }
};

template<typename T, typename = decltype([]{})>
consteval std::meta::info injection_fetch() {
    if constexpr (requires { injection_function(injection_declaration<T>{}); }) {
        return injection_function(injection_declaration<T>{});
    } else {
        return {};
    }
}
// ~ basic friend injection

// "implicit command line arguments" library
// internal library impl 
template<size_t>
struct implicit_index {};

struct implicit_node {
    const char* name_begin;
    size_t name_length;
    std::meta::info type;

    consteval std::string_view name() const { return std::string_view(name_begin, name_length); }
};

consteval std::meta::info implicit_fetch_raw(size_t i) {
    auto key = substitute(^^implicit_index, {std::meta::reflect_constant(i)});
    auto fetch = substitute(^^injection_fetch, {key});
    auto value = extract<std::meta::info(*)()>(fetch)();
    return value;
}

consteval size_t implicit_size() {
    for (size_t i = 0; true; ++i) {
        auto stored = implicit_fetch_raw(i);
        if (stored == std::meta::info{}) return i;
    }
}

// precondition: i < implicit_size()
consteval implicit_node implicit_fetch(size_t i) {
    return extract<implicit_node>(implicit_fetch_raw(i));
}

consteval void implicit_store(size_t i, implicit_node node) {
    auto key = substitute(^^implicit_index, {std::meta::reflect_constant(i)});
    auto value = std::meta::reflect_constant(std::meta::reflect_constant(node));
    auto definer = substitute(^^injection_definition, {key, value});
    size_of(definer); // instantiate it
}

// returns index where it was stored
consteval size_t implicit_register(std::string_view name, std::meta::info type, std::source_location loc) {
    auto throw_error = [=]<typename... Ts>(std::format_string<Ts...> fmt, Ts&&... args) {
        auto base = std::format("[implicit] implicit_register({:?}, {:?})", name, display_string_of(type));
        auto message = std::format(fmt, static_cast<Ts&&>(args)...);
        auto full = std::format("{}: {}", base, message);
        throw std::meta::exception(full, ^^implicit_register, loc);
    };
    type = dealias(type);
    if (is_reference_type(type)) throw_error("type must not be a reference");
    if (is_const(type)) throw_error("type must not be const qualified");
    if (is_volatile(type)) throw_error("type must not be volatile qualified");
    size_t size = implicit_size();
    for (size_t i = 0; i < size; ++i) {
        auto node = implicit_fetch(i);
        if (node.name() != name) continue;
        if (is_same_type(node.type, type)) return i; // already set to correct value
        throw_error("name already associated with a different type: {:?}", display_string_of(node.type));
    }
    // name doesn't already exist in "container", adding it
    implicit_node node{
        .name_begin = name.data(),
        .name_length = name.size(),
        .type = type,
    };
    implicit_store(size, node);
    return size;
}

template<typename T, size_t /* index */>
struct implicit_parsed_storage {
  // TOOD: gcc bug here?
  static inline std::optional<T> value = std::nullopt;
};

template<typename T, typename U>
inline static consteval std::optional<T>* implicit_find_value_impl() {
  return &U::value;
}

template<typename T>
struct implicit_name {
  std::optional<T>* value_ptr;

  inline static consteval std::optional<T>* find_value(std::meta::info storage) {
    auto ret = extract<std::optional<T>*(*)()>(substitute(^^implicit_find_value_impl, {^^T, storage}))();
    return ret;
  }

  consteval implicit_name(std::string_view name, std::source_location loc = std::source_location::current()) {
    // if `name` is associated with a string literal, we cannot use it
    // as template argument, so laundering it first
    name = std::string_view(std::define_static_string(name));
    size_t index = implicit_register(name, ^^T, loc);
    auto storage = substitute(^^implicit_parsed_storage, {^^T, std::meta::reflect_constant(index)});
    value_ptr = find_value(storage);
  }

  consteval implicit_name(const char* name) : implicit_name(std::string_view(name)) {}
};

inline bool implicit_parsed = false;

void implicit_parse_into(std::span<const char* const>& args, std::optional<bool>& value) {
  if (args.empty()) { value = true; return; }
  std::string_view curr(args[0]);
  if (curr == "1" || curr == "on" || curr == "yes" || curr == "enable") {
    args = args.subspan(1);
    value = true;
  } else if (curr == "0" || curr == "off" || curr == "no" || curr == "disable") {
    args = args.subspan(1);
    value = false;
  } else {
    value = true;
  }
}

void implicit_parse_into(std::span<const char* const>& args, std::optional<int>& value) {
  if (args.empty())
    throw std::runtime_error("missing argument to option expecting int");
  std::string_view sv(args[0]);
  args = args.subspan(1);
  int v;
  auto ret = std::from_chars(sv.data(), sv.data() + sv.size(), v);
  if (ret && ret.ptr == sv.data() + sv.size()) value = v;
  else throw std::runtime_error(std::format("failed to parse int, argument: {:?}", sv));
}

template<size_t Start = 0> // delay instantiation
bool implicit_parse1(std::span<const char* const>& args) {
  if (args.empty()) return false;
  std::string_view curr(args[0]);
  if (!curr.starts_with("--")) return false;
  auto name = curr.substr(2);
  const char* eq = nullptr;
  if (auto loc = name.find('='); loc != std::string_view::npos) {
    eq = name.data() + loc + 1;
    name = name.substr(0, loc);
  }
  std::span<const char* const> eqargs(&eq, &eq + !!eq);
  template for (constexpr auto index : std::views::iota(Start, implicit_size())) {
    constexpr auto node = implicit_fetch(index);
    if (node.name() != name) continue;
    args = args.subspan(1);
    implicit_parse_into(eq ? eqargs : args, implicit_parsed_storage<typename [:node.type:], index>::value);
    if (eq && !eqargs.empty())
      throw std::runtime_error(std::format("failed to parse payload after '=': {:?}", curr));
    return true;
  }
  return false;
}
// ~ internal library impl

template<auto Parse1 = &implicit_parse1> // delay instantiation
void implicit_parse(std::span<const char* const>& args) {
  contract_assert(!implicit_parsed);
  implicit_parsed = true;
  while (Parse1(args));
}

template<typename T>
T implicit_value(implicit_name<std::remove_cvref_t<T>> name, T&& default_value) {
  contract_assert(implicit_parsed);
  return *name.value_ptr ? **name.value_ptr : static_cast<T&&>(default_value);
}

bool implicit_flag(implicit_name<bool> name, bool default_value = false) {
  return implicit_value(name, default_value);
}
// ~ "implicit command line arguments" library
