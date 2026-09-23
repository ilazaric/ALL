#pragma once

#include <ivl/format>
#include <ivl/utility>
#include <boost/json.hpp>

namespace boost::json {
namespace ivl_detail {
  // https://www.boost.org/doc/libs/latest/libs/json/doc/html/examples.html#pretty
  inline void dump_impl(value const& jv, std::string& out, size_t indent, size_t inc) {
    switch (jv.kind()) {
    case kind::object: {
      out += "{\n";
      indent += inc;
      auto const& obj = jv.get_object();
      if (!obj.empty()) {
        auto it = obj.begin();
        for (;;) {
          out.append(indent, ' ');
          out += serialize(it->key());
          out += ": ";
          dump_impl(it->value(), out, indent, inc);
          if (++it == obj.end()) break;
          out += ",\n";
        }
      }
      out += '\n';
      indent -= inc;
      out.append(indent, ' ');
      out += '}';
      break;
    }

    case kind::array: {
      out += "[\n";
      indent += inc;
      auto const& arr = jv.get_array();
      if (!arr.empty()) {
        auto it = arr.begin();
        for (;;) {
          out.append(indent, ' ');
          dump_impl(*it, out, indent, inc);
          if (++it == arr.end()) break;
          out += ",\n";
        }
      }
      out += '\n';
      indent -= inc;
      out.append(indent, ' ');
      out += ']';
      break;
    }

    default:
      out += serialize(jv);
      break;
    }
  }
} // namespace ivl_detail

std::string dump(const value& v, int indent = -1) {
  if (indent == -1) return serialize(v);
  contract_assert(indent >= 0);
  std::string ret;
  ivl_detail::dump_impl(v, ret, 0, (size_t)indent);
  return ret;
}
} // namespace boost::json

template<>
struct ivl::fmt_raw::formatter<boost::json::value> {
  int indent = -1;

  constexpr auto parse(auto& ctx) {
    if (ctx.begin() == ctx.end() || *ctx.begin() == '}') return ctx.begin();
    auto res = std::from_chars(ctx.begin(), ctx.end(), indent);
    if (!res) ivl::panic("failed to parse number");
    if (res.ptr == ctx.end() || *res.ptr == '}') return res.ptr;
    ivl::panic("failed to parse boost::json::value spec");
  }

  auto format(const boost::json::value& v, auto& ctx) const {
    return ivl::fmt::formatter<std::string_view>{}.format(dump(v, indent), ctx);
  }
};
