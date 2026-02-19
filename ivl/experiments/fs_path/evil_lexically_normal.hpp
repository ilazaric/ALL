#include <sstream>
#define private public
#define protected public
#include <filesystem>
#undef private
#undef protected
#include <algorithm>
#include <ranges>
#include <string_view>
#include <vector>

using namespace std::literals::string_view_literals;

namespace fs = std::filesystem;
using fs::path;

struct path::_List::_Impl {
  using value_type = _Cmpt;

  _Impl(int cap) : _M_size(0), _M_capacity(cap) {}

  alignas(value_type) int _M_size;
  int _M_capacity;

  using iterator = value_type*;
  using const_iterator = const value_type*;

  iterator begin() { return reinterpret_cast<value_type*>(this + 1); }
  iterator end() { return begin() + size(); }

  const_iterator begin() const { return reinterpret_cast<const value_type*>(this + 1); }
  const_iterator end() const { return begin() + size(); }

  const value_type& front() const { return *begin(); }
  const value_type& back() const { return end()[-1]; }

  int size() const { return _M_size; }
  int capacity() const { return _M_capacity; }
  bool empty() const { return _M_size == 0; }

  void clear() {
    std::destroy_n(begin(), _M_size);
    _M_size = 0;
  }

  void pop_back() {
    back().~_Cmpt();
    --_M_size;
  }

  void _M_erase_from(const_iterator pos) {
    iterator first = begin() + (pos - begin());
    iterator last = end();
    std::destroy(first, last);
    _M_size -= last - first;
  }

  unique_ptr<_Impl, _Impl_deleter> copy() const {
    const auto n = size();
    void* p = ::operator new(sizeof(_Impl) + n * sizeof(value_type));
    unique_ptr<_Impl, _Impl_deleter> newptr(::new (p) _Impl{n});
    std::uninitialized_copy_n(begin(), n, newptr->begin());
    newptr->_M_size = n;
    return newptr;
  }

  // Clear the lowest two bits from the pointer (i.e. remove the _Type value)
  static _Impl* notype(_Impl* p) {
    constexpr uintptr_t mask = ~(uintptr_t)0x3;
    return reinterpret_cast<_Impl*>(reinterpret_cast<uintptr_t>(p) & mask);
  }
};

void path::_List::_Impl_deleter::operator()(_Impl* p) const noexcept {
  p = _Impl::notype(p);
  if (p) {
    __glibcxx_assert(p->_M_size <= p->_M_capacity);
    p->clear();
    ::operator delete(p, sizeof(*p) + p->_M_capacity * sizeof(value_type));
  }
}

path::_List::_List() : _M_impl(reinterpret_cast<_Impl*>(_Type::_Filename)) {}

path::_List::_List(const _List& other) {
  if (!other.empty()) _M_impl = other._M_impl->copy();
  else type(other.type());
}

path::_List& path::_List::operator=(const _List& other) {
  if (!other.empty()) {
    // copy in-place if there is capacity
    const int newsize = other._M_impl->size();
    auto impl = _Impl::notype(_M_impl.get());
    if (impl && impl->capacity() >= newsize) {
      const int oldsize = impl->_M_size;
      auto to = impl->begin();
      auto from = other._M_impl->begin();
      const int minsize = std::min(newsize, oldsize);
      for (int i = 0; i < minsize; ++i) to[i]._M_pathname.reserve(from[i]._M_pathname.length());
      if (newsize > oldsize) {
        std::uninitialized_copy_n(from + oldsize, newsize - oldsize, to + oldsize);
        impl->_M_size = newsize;
      } else if (newsize < oldsize) impl->_M_erase_from(impl->begin() + newsize);
      std::copy_n(from, minsize, to);
      type(_Type::_Multi);
    } else _M_impl = other._M_impl->copy();
  } else {
    clear();
    type(other.type());
  }
  return *this;
}

inline void path::_List::type(_Type t) noexcept {
  auto val = reinterpret_cast<uintptr_t>(_Impl::notype(_M_impl.release()));
  _M_impl.reset(reinterpret_cast<_Impl*>(val | (unsigned char)t));
}

inline int path::_List::size() const noexcept {
  if (auto* ptr = _Impl::notype(_M_impl.get())) return ptr->size();
  return 0;
}

inline int path::_List::capacity() const noexcept {
  if (auto* ptr = _Impl::notype(_M_impl.get())) return ptr->capacity();
  return 0;
}

inline bool path::_List::empty() const noexcept { return size() == 0; }

inline auto path::_List::begin() noexcept -> iterator {
  __glibcxx_assert(!empty());
  if (auto* ptr = _Impl::notype(_M_impl.get())) return ptr->begin();
  return nullptr;
}

inline auto path::_List::end() noexcept -> iterator {
  __glibcxx_assert(!empty());
  if (auto* ptr = _Impl::notype(_M_impl.get())) return ptr->end();
  return nullptr;
}

auto path::_List::begin() const noexcept -> const_iterator {
  __glibcxx_assert(!empty());
  if (auto* ptr = _Impl::notype(_M_impl.get())) return ptr->begin();
  return nullptr;
}

auto path::_List::end() const noexcept -> const_iterator {
  __glibcxx_assert(!empty());
  if (auto* ptr = _Impl::notype(_M_impl.get())) return ptr->end();
  return nullptr;
}

inline auto path::_List::front() noexcept -> value_type& { return *_M_impl->begin(); }

inline auto path::_List::back() noexcept -> value_type& { return _M_impl->begin()[_M_impl->size() - 1]; }

inline auto path::_List::front() const noexcept -> const value_type& { return *_M_impl->begin(); }

inline auto path::_List::back() const noexcept -> const value_type& { return _M_impl->begin()[_M_impl->size() - 1]; }

inline void path::_List::pop_back() {
  __glibcxx_assert(size() > 0);
  _M_impl->pop_back();
}

inline void path::_List::_M_erase_from(const_iterator pos) { _M_impl->_M_erase_from(pos); }

inline void path::_List::clear() {
  if (auto ptr = _Impl::notype(_M_impl.get())) ptr->clear();
}

void path::_List::reserve(int newcap, bool exact = false) {
  // __glibcxx_assert(type() == _Type::_Multi);

  _Impl* curptr = _Impl::notype(_M_impl.get());

  int curcap = curptr ? curptr->capacity() : 0;

  if (curcap < newcap) {
    if (!exact && newcap < int(1.5 * curcap)) newcap = 1.5 * curcap;

    void* p = ::operator new(sizeof(_Impl) + newcap * sizeof(value_type));
    std::unique_ptr<_Impl, _Impl_deleter> newptr(::new (p) _Impl{newcap});
    const int cursize = curptr ? curptr->size() : 0;
    if (cursize) {
      std::uninitialized_move_n(curptr->begin(), cursize, newptr->begin());
      newptr->_M_size = cursize;
    }
    std::swap(newptr, _M_impl);
  }
}

namespace impl {
namespace {
  const fs::path::value_type dot = '.';

  inline bool is_dot(fs::path::value_type c) { return c == dot; }

  inline bool is_dot(const fs::path& path) {
    const auto& filename = path.native();
    return filename.size() == 1 && is_dot(filename[0]);
  }

  inline bool is_dotdot(const fs::path& path) {
    const auto& filename = path.native();
    return filename.size() == 2 && is_dot(filename[0]) && is_dot(filename[1]);
  }
} // namespace

path lexically_normal(const path& self) {
  /*
  C++17 [fs.path.generic] p6
  - If the path is empty, stop.
  - Replace each slash character in the root-name with a preferred-separator.
  - Replace each directory-separator with a preferred-separator.
  - Remove each dot filename and any immediately following directory-separator.
  - As long as any appear, remove a non-dot-dot filename immediately followed
    by a directory-separator and a dot-dot filename, along with any immediately
    following directory-separator.
  - If there is a root-directory, remove all dot-dot filenames and any
    directory-separators immediately following them.
  - If the last filename is dot-dot, remove any trailing directory-separator.
  - If the path is empty, add a dot.
  */
  path ret;
  bool last_dot = false;
  // If the path is empty, stop.
  if (self.empty()) return ret;

  std::vector<const path*> pieces;
  auto my_append = [&](const path& p) { pieces.push_back(&p); };
  auto my_has_filename = [&] {
    if (pieces.empty()) return false;
    return true;
  };
  auto my_remove_filename = [&] {
    pieces.pop_back();
    last_dot = true;
  };
  auto my_filename = [&] { return *pieces.back(); };

  if (self.is_absolute()) {
    for (auto& p : std::views::drop(self, 1)) {
      last_dot = false;
      if (is_dotdot(p)) {
        if (my_has_filename()) {
          // remove a non-dot-dot filename immediately followed by /..
          if (!is_dotdot(my_filename())) my_remove_filename();
          else my_append(p);
        }
      } else if (is_dot(p)) last_dot = true;
      else my_append(p);
    }
  } else {
    for (auto& p : self) {
      last_dot = false;
      if (is_dotdot(p)) {
        if (my_has_filename()) {
          // remove a non-dot-dot filename immediately followed by /..
          if (!is_dotdot(my_filename())) my_remove_filename();
          else my_append(p);
        } else my_append(p);
      } else if (is_dot(p)) last_dot = true;
      else my_append(p);
    }
  }

  // for (auto& p : std::views::drop(self, self.is_absolute())) {
  //   last_dot = false;
  //   if (is_dotdot(p)) {
  //     if (my_has_filename()) {
  //       // remove a non-dot-dot filename immediately followed by /..
  //       if (!is_dotdot(my_filename())) my_remove_filename();
  //       else my_append(p);
  //     } else {
  //       // remove a dot-dot filename immediately after root-directory
  //       if (!self.has_root_directory()) my_append(p);
  //     }
  //   } else if (is_dot(p)) last_dot = true;
  //   else my_append(p);
  // }

  auto runit = [&](auto&& callback) {
    if (self.is_absolute()) callback(self.begin()->_M_pathname);
    for (size_t i = 0; i < pieces.size(); ++i) {
      if (i != 0) callback("/"sv);
      callback(pieces[i]->_M_pathname);
    }
    if (last_dot && !pieces.empty()) callback("/"sv);
  };

  size_t total = 0;
  runit([&](auto&& str) { total += std::ranges::size(str); });
  ret._M_pathname.reserve(total);
  runit([&](auto&& str) { ret._M_pathname += str; });
  ret._M_split_cmpts();

  if (ret._M_cmpts.size() >= 2) {
    auto back = std::prev(ret.end());
    // If the last filename is dot-dot, ...
    if (back->empty() && is_dotdot(*std::prev(back)))
      // ... remove any trailing directory-separator.
      ret = ret.parent_path();
  }
  // If the path is empty, add a dot.
  else if (ret.empty())
    ret = ".";

  return ret;
}
} // namespace impl
