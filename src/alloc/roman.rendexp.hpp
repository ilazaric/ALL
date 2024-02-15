
/////////////////////////////////////////////////////////////////////////
// RENDERED WITH `https://github.com/ilazaric/ALL` - `render-includes` //
/////////////////////////////////////////////////////////////////////////


////////////////////
// ORIGINAL CODE: //
////////////////////


///////////////
// roman.cpp //
///////////////

// #include <ivl/alloc/small_ptr_allocator.hpp>
// #include <ivl/alloc/mmap_fixed_storage.hpp>
// #include <vector>
// 
// template<std::size_t N>
// struct DynArray {
//   std::vector<char> vec;
//   DynArray() : vec(N){}
//   constexpr char* data(){return vec.data();}
//   constexpr std::size_t size(){return N;}
// };
// 
// struct AllocTraits {
//   // inline static DynArray<(128ULL<<20)> storage;
//   inline static ivl::alloc::MmapFixedStorage<0x0000'0300'0000'0000, (128ULL<<20)> storage;
//   static constexpr std::size_t segment_tree_chunk_size = 64;
//   static constexpr std::size_t free_list_limit = 256;
//   static constexpr std::size_t free_list_steal_coef = 64;
// };
// 
// struct AllocDynTraits {
//   inline static DynArray<(128ULL<<20)> storage;
//   // inline static ivl::alloc::MmapFixedStorage<0x0000'0300'0000'0000, (128ULL<<20)> storage;
//   static constexpr std::size_t segment_tree_chunk_size = 64;
//   static constexpr std::size_t free_list_limit = 256;
//   static constexpr std::size_t free_list_steal_coef = 64;
// };
// 
// template<typename T>
// using Alloc = ivl::alloc::SmallPtrAllocator<T, AllocTraits>;
// 
// template<typename T>
// using DynAlloc = ivl::alloc::SmallPtrAllocator<T, AllocDynTraits>;
// 
// void consume(int);
// 
// void deref_fancy(Alloc<int>::pointer ptr){
//   consume(*ptr);
// }
// 
// void deref_fancy_dyn(DynAlloc<int>::pointer ptr){
//   consume(*ptr);
// }
// 
// void deref_raw(int* ptr){
//   consume(*ptr);
// }
// 
// void deref_fancy2(Alloc<int>::pointer ptr1, Alloc<int>::pointer ptr2){
//   consume(*ptr1);
//   consume(*ptr2);
// }
// 
// void deref_raw2(int* ptr1, int* ptr2){
//   consume(*ptr1);
//   consume(*ptr2);
// }
// 
// template<typename P>
// int accum(P ptr, std::size_t len){
//   int res = 0;
//   for (std::size_t i = 0; i < len; ++i)
//     res += ptr[i];
//   return res;
// }
// 
// template int accum<int*>(int*, std::size_t);
// template int accum<Alloc<int>::pointer>(Alloc<int>::pointer, std::size_t);

/////////////////////////////
// RENDER OF roman.cpp [0] //
/////////////////////////////


///////////////////////////////////////////////////////
// RENDER OF <ivl/alloc/small_ptr_allocator.hpp> [1] //
///////////////////////////////////////////////////////


#include <cstddef>
#include <array>
#include <cassert>


///////////////////////////////////////
// RENDER OF "segment_tree2.hpp" [2] //
///////////////////////////////////////


#include <algorithm>
#include <array>
#include <bit>
#include <climits>
#include <cstdint>
#include <tuple>
#include <type_traits>

namespace ivl::alloc {

// like SegmentTree, but more compact and template-y
template <std::size_t LeafCount> struct SegmentTree2 {
  static constexpr std::size_t LENGTH = std::bit_ceil(LeafCount);

  static_assert(LENGTH <= (1ULL << 31));

  template <std::size_t N, std::size_t W> struct Packed {
    using Word = std::uint64_t;
    inline static constexpr std::size_t WordWidth = sizeof(Word) * CHAR_BIT;
    // small memory pessimization
    inline static constexpr std::size_t FixedW = std::bit_ceil(W);
    inline static constexpr std::size_t ElemWidth = 2 * FixedW;
    inline static constexpr std::size_t TotalBitCount = N * ElemWidth;
    inline static constexpr std::size_t WordCount =
        (TotalBitCount + WordWidth - 1) / WordWidth;
    inline static constexpr std::size_t ElemCountPerWord =
        WordWidth / ElemWidth;

    std::array<Word, WordCount> words;

    std::pair<std::uint32_t, std::uint32_t> load(std::uint32_t idx) const {
      auto wordidx = idx / ElemCountPerWord;
      // if constexpr (N == 4)
      //   LOG(N, W, FixedW, ElemWidth, ElemCountPerWord, idx, wordidx);
      auto word = words[wordidx];
      word >>= ((idx % ElemCountPerWord) * ElemWidth);
      auto a = word & ((1ULL << FixedW) - 1);
      word >>= FixedW;
      auto b = word & ((1ULL << FixedW) - 1);
      return {a, b};
    }

    void store(std::uint32_t idx, std::uint32_t a, std::uint32_t b) {
      auto shift = (idx % ElemCountPerWord) * ElemWidth;
      auto el = (((Word)b << FixedW) | a) << shift;
      auto elmask = ((1ULL << FixedW << FixedW) - 1) << shift;
      auto wordidx = idx / ElemCountPerWord;
      auto word = words[wordidx];
      auto newword = el | ~elmask & word;
      words[wordidx] = newword;
    }
  };

  /*
    hmm
    number of states
    0 <= left <= max <= 2^K
    1) max <= 2^(K-1)
    --> left is anything between 0 and max
    --> 2^(K-2) * (2^(K-1) + 1) in total
    2) max > 2^(K-1)
    --> left == max
    --> 2^(K-1) in total
   */

  template <std::size_t I> static consteval auto garbage() {
    if constexpr (I + 1 == 0) {
      return std::tuple<>();
    } else {
      return std::tuple_cat(garbage<I - 1>(),
                            std::tuple<Packed<(LENGTH >> I), I + 1>>());
    }
  }

  // template<std::size_t I>
  // struct DataImpl {
  //   using type = std::conditional_t<
  //     I+1==0,
  //     std::tuple<>,
  //     decltype(std::tuple_cat(std::declval<DataImpl<I-1>::type>(),
  //     std::declval<std::tuple<Packed<(LENGTH>>I), I+1>>>()))
  //     >;
  // };

  // using Data = typename DataImpl<std::countr_zero(LENGTH)>::type;

  using Data = decltype(garbage<std::countr_zero(LENGTH)>());

  Data data;

  template <std::size_t Level, bool Kind>
  void modify_single(std::uint32_t idx) {
    if (Kind) {
      std::get<Level>(data).store(idx, 1u << Level, 1u << Level);
    } else {
      std::get<Level>(data).store(idx, 0, 0);
    }
  }

  template <std::size_t Level> void rebuild(std::uint32_t idx) {
    auto [ll, lm] = std::get<Level - 1>(data).load(idx * 2);
    auto [rl, rm] = std::get<Level - 1>(data).load(idx * 2 + 1);
    if (ll == (1 << (Level - 1))) {
      std::get<Level>(data).store(idx, (1u << (Level - 1)) + rl,
                                  (1u << (Level - 1)) + rl);
    } else {
      std::get<Level>(data).store(idx, ll, std::max(lm, rm));
    }
  }

  template <std::size_t I> void init_bigger_levels() {
    if constexpr (I <= std::countr_zero(LENGTH)) {
      for (std::uint32_t i = 0; i < (LENGTH >> I); ++i)
        rebuild<I>(i);
      init_bigger_levels<I + 1>();
    }
  }

  template <std::size_t Level> std::uint64_t sum_level() {
    std::uint64_t res = 0;
    // LOG(LENGTH >> Level);
    for (std::uint32_t i = 0; i < (LENGTH >> Level); ++i) {
      res += LOG(i, std::get<Level>(data).load(i)).second;
    }
    return res;
  }

  SegmentTree2() {
    for (std::uint32_t idx = 0; idx < LeafCount; ++idx)
      modify_single<0, true>(idx);
    for (std::uint32_t idx = LeafCount; idx < LENGTH; ++idx)
      modify_single<0, false>(idx);
    init_bigger_levels<1>();
  }

  static constexpr std::uint32_t FAILURE = -1;

  template <std::size_t Level> void refresh_upwards(std::uint32_t idx) {
    if constexpr (Level <= std::countr_zero(LENGTH)) {
      rebuild<Level>(idx);
      refresh_upwards<Level + 1>(idx / 2);
    }
  }

  template <std::size_t Level, bool Kind>
  void modify_prefix(std::uint32_t idx, std::uint32_t leftlen) {
    if constexpr (Level == 0) {
      if (leftlen)
        modify_single<0, Kind>(idx);
      refresh_upwards<1>(idx / 2);
    } else {
      if (leftlen >= (1u << (Level - 1))) {
        modify_single<Level - 1, Kind>(idx * 2);
        leftlen -= (1u << (Level - 1));
        modify_prefix<Level - 1, Kind>(idx * 2 + 1, leftlen);
      } else {
        modify_prefix<Level - 1, Kind>(idx * 2, leftlen);
      }
    }
  }

  template <std::size_t Level>
  std::uint32_t take_impl(std::uint32_t idx, std::uint32_t reqlen) {
    if constexpr (Level + 1 != 0) {
      if (std::get<Level>(data).load(idx).first >= reqlen) {
        modify_prefix<Level, false>(idx, reqlen);
        return idx << Level;
      }
      if constexpr (Level != 0) {
        return take_impl<Level - 1>(
            idx * 2 + (std::get<Level - 1>(data).load(idx * 2).second < reqlen),
            reqlen);
      } else {
        return -1;
      }
    } else {
      // does not happen :D
      // std::unreachable();
      return -1;
    }
  }

  std::uint32_t take(std::uint32_t reqlen) {
    if (std::get<std::countr_zero(LENGTH)>(data).load(0).second < reqlen)
      return FAILURE;
    return take_impl<std::countr_zero(LENGTH)>(0, reqlen);
  }

  template <std::size_t Level>
  void give_impl(std::uint32_t idx, std::uint32_t reqlen, std::uint32_t loc) {
    if constexpr (Level != -1) {
      if (loc == 0) {
        modify_prefix<Level, true>(idx, reqlen);
      }
      if constexpr (Level != 0) {
        if (loc >= (1u << (Level - 1))) {
          loc -= (1u << (Level - 1));
          idx = idx * 2 + 1;
        } else {
          idx = idx * 2;
        }
        give_impl<Level - 1>(idx, reqlen, loc);
      }
    }
  }

  void give(std::uint32_t reqlen, std::uint32_t loc) {
    give_impl<std::countr_zero(LENGTH)>(0, reqlen, loc);
  }
};

} // namespace ivl::alloc

////////////////////////////////////
// FINISH "segment_tree2.hpp" [2] //
////////////////////////////////////



///////////////////////////////
// RENDER OF <ivl/debug> [2] //
///////////////////////////////



//////////////////////////////////////////
// RENDER OF <ivl/debug/assert.hpp> [3] //
//////////////////////////////////////////


// #ifndef IVL_DBG_BACKTRACE_BUFFER_SIZE
// # define IVL_DBG_BACKTRACE_BUFFER_SIZE 20
// #endif

// #ifndef LOG
// # define LOG(...)
// #endif


////////////////////////////////
// RENDER OF <ivl/logger> [4] //
////////////////////////////////



///////////////////////////////////////////
// RENDER OF <ivl/logger/logger.hpp> [5] //
///////////////////////////////////////////


/*
  requires c++20
  
  this implements a supercool macro fn `LOG(...)`
  through that macro fn the strings of expressions we want to log
  become accessible to us during compile time, in a pretty nice way

  if you are lazy just include this header and do
  `using namespace ivl::logger::default_logger;`

  if you are ambitious please refer to the implementation of
  `default_logger::logger` class template at the bottom, that should
  clear up how you can implement your own logger,
  you can dump to a specific file instead of stderr,
  add timestamps, whatever you can think of

  the jist is that a class template called `logger` should be visible from
  the point of invocation of `LOG(..)` macro fn
  template args of `logger` should be two types:
  - first type representing the names of all expressions we want to log
  - second type representing the location of `LOG(...)` invocation
  furthermore, it should provide a static member function template
  called `print` that is capable of accepting all arguments passed
  to `LOG(...)`
  most likely you want it to be variadic

  `logger` is a class template bc it seemed to be easiest
  to pass both compile time args (names, location) and runtime args

  you might want to implement an additional class that
  holds the state you need for logging

  this is a pretty simple header, if you don't like something
  just change it in your repository copy :)
 */

#include <array>
#include <iostream> // only used for `default_logger`, remove if unnecessary
#include <string_view>
#include <cstdint>

namespace ivl::logger {

  consteval std::size_t find_comma(std::string_view names) {
    std::size_t openparencount = 0;
    char instr = 0;
    for (std::size_t idx = 0; idx < names.size(); ++idx){
      if (!instr){
        if (names[idx] == ',' && openparencount == 0)
          return idx;
        if (names[idx] == '(' || names[idx] == '[' || names[idx] == '{')
          ++openparencount;
        if (names[idx] == ')' || names[idx] == ']' || names[idx] == '}')
          --openparencount;
      }
      if (names[idx] == '\'' || names[idx] == '"'){
        if (!instr){
          instr = names[idx];
          continue;
        }
        if (instr != names[idx]) continue;
        if (names[idx-1] == '\\') continue;
        instr = 0;
      }
    }
    return std::string_view::npos;
  }

static_assert(find_comma("(,),x") == 3);
static_assert(find_comma("',',x") == 3);
static_assert(find_comma("\",\",x") == 3);
static_assert(find_comma("find_counterexample(6, 3, ((1<<6)-1) & 0xAB, 1)") == std::string_view::npos);

consteval void callback_names(std::string_view allnames, auto &callback) {
  while (true) {
    std::size_t commaloc = find_comma(allnames);
    std::string_view name = allnames.substr(0, commaloc);
    if (name.starts_with(' '))
      name = name.substr(1);
    callback(name);
    if (commaloc == std::string_view::npos)
      return;
    allnames = allnames.substr(commaloc + 1);
  }
}

consteval std::size_t count_names(std::string_view allnames) {
  struct {
    std::size_t count = 0;
    consteval void operator()(std::string_view) { ++count; }
  } callback;
  callback_names(allnames, callback);
  return callback.count;
}

// constexpr has become pretty powerful, love to see it
template <std::size_t N>
consteval auto generate_names(std::string_view allnames) {
  struct {
    std::size_t index = 0;
    std::array<std::string_view, N> names;
    consteval void operator()(std::string_view name) { names[index++] = name; }
  } callback;
  callback_names(allnames, callback);
  return callback.names;
}

// a string that can be passed via template args
template <unsigned N> struct fixed_string {
  char buf[N + 1]{};
  consteval fixed_string(char const *s) {
    for (unsigned i = 0; i != N; ++i)
      buf[i] = s[i];
  }
  consteval operator char const *() const { return buf; }
};
template <unsigned N> fixed_string(char const (&)[N]) -> fixed_string<N - 1>;

consteval std::size_t length(const char *ptr) {
  std::size_t len = 0;
  while (ptr[len])
    ++len;
  return len;
}

template <fixed_string T> struct name_storage {
  inline static constexpr std::string_view allnames{(const char*)T, length((const char*)T)};
  inline static constexpr std::size_t namecount{count_names(allnames)};
  inline static constexpr auto names{generate_names<namecount>(allnames)};
};

// a std::source_location that can be passed via template args
template <std::uint_least32_t linet, // std::uint_least32_t columnt,
          fixed_string file_namet, fixed_string function_namet>
struct fixed_source_location {
  inline constexpr static auto line = linet;
  // constexpr static inline auto column = columnt;
  inline constexpr static auto file_name = file_namet;
  inline constexpr static auto function_name = function_namet;
};

  template<typename T>
  constexpr decltype(auto) discardable_forward(std::remove_reference_t<T>& t){
    return std::forward<T>(t);
  }

  constexpr decltype(auto) discardable_forward_last(auto&& ... ts){
    return (discardable_forward<decltype(ts)>(ts), ...);
  }

namespace default_logger {

template <typename NS, typename CSL> struct logger_hook {
  template <typename... Args> [[maybe_unused]] static decltype(auto) print(Args&& ...args) {
    static_assert(NS::namecount == sizeof...(Args));
    std::cerr << "[LOG] " << CSL::file_name << ":" << CSL::function_name << "(" << CSL::line << "):";
    std::size_t index = 0;
    ( // should be easy to inline
     [](std::size_t &index, const Args &arg) {
       std::cerr << " " << NS::names[index++] << "=" << (arg);
     }(index, args),
     ...);
    std::cerr << std::endl;
    // this makes `LOG` usable as a wrapper around sub-expressions
    // wrapper around `std::forward` due to `[[nodiscard]]`
    // technically not correct bc comma overloadable
    return (discardable_forward<Args>(args), ...);
  };
};

} // namespace default_logger

} // namespace ivl::logger

#ifdef IVL_LOCAL
# define LOG(...)                                                        \
  logger_hook<::ivl::logger::name_storage<#__VA_ARGS__>,                \
               ::ivl::logger::fixed_source_location<__LINE__, __FILE__, __func__>> \
  ::print(__VA_ARGS__)
#else
# define LOG(...) (ivl::logger::discardable_forward_last(__VA_ARGS__))
#endif

////////////////////////////////////////
// FINISH <ivl/logger/logger.hpp> [5] //
////////////////////////////////////////


using namespace ivl::logger::default_logger;

/////////////////////////////
// FINISH <ivl/logger> [4] //
/////////////////////////////


#ifdef IVL_DBG_MODE

#if __has_include(<stacktrace>)
# include <stacktrace>
# define IVL_DBG_ASSERT_STACKTRACE              \
  std::cerr << "[IVL] stacktrace:\n"            \
  << std::stacktrace::current() << "\n"
#elif __has_include(<boost/stacktrace.hpp>)
# include <boost/stacktrace.hpp>
# define IVL_DBG_ASSERT_STACKTRACE              \
  std::cerr << "[IVL] stacktrace:\n"            \
  << boost::stacktrace::stacktrace() << "\n"
#else
# define IVL_DBG_ASSERT_STACKTRACE
#endif

#include <iostream>
#define IVL_DBG_ASSERT(expr, ...) do {                  \
    if (!(expr)) [[unlikely]] {                         \
      std::cerr << "[IVL] ERROR: ASSERTION FAILED\n"    \
                << "[IVL] LINE: " << __LINE__ << "\n"   \
                << "[IVL] EXPR: " << #expr << "\n";     \
      IVL_DBG_ASSERT_STACKTRACE;                        \
      LOG("data" __VA_OPT__(,) __VA_ARGS__);            \
      exit(-1);                                         \
    }} while (false)

#else // IVL_DBG_MODE
#define IVL_DBG_ASSERT(...)
#endif // IVL_DBG_MODE

///////////////////////////////////////
// FINISH <ivl/debug/assert.hpp> [3] //
///////////////////////////////////////


////////////////////////////
// FINISH <ivl/debug> [2] //
////////////////////////////


namespace ivl::alloc {

  /*
    storage needs a data() and size() fn
   */

  namespace spa_detail {

    template<typename T, typename Traits>
    struct Pointer {
      static_assert(!std::is_volatile_v<T>);
      static_assert(!std::is_reference_v<T>);
      
      // pointer_traits
      using element_type = T;
      using size_type = std::uint32_t;
      using difference_type = std::int32_t;

      template<typename U>
      using rebind = Pointer<U, Traits>;

      // TODO: pointer_to
      // should be done, check

      // iterator_traits
      using value_type = std::remove_cv_t<T>;
      // hate this
      // TODO: think even more about this
      using pointer = T*;
      // using pointer = Pointer;
      // TODO: should this be a proxy that is convertible to T& ?
      // contiguous_iterator concept seems to indicate "NO"
      using reference = std::add_lvalue_reference_t<std::conditional_t<std::is_void_v<T>, int, T>>;
      using iterator_category = std::random_access_iterator_tag;
      using iterator_concept = std::contiguous_iterator_tag;

      static Pointer pointer_to(reference r){
        auto ri = reinterpret_cast<std::uintptr_t>(&r);
        Pointer out{};
        auto zi = reinterpret_cast<std::uintptr_t>(Traits::storage.data());
        IVL_DBG_ASSERT(ri >= zi && ri < zi + Traits::storage.size() && "bad reference, not from our alloc", zi, ri, zi+Traits::storage.size(), &r);
        auto delta = ri - zi;
        out.offset += delta;
        return out;
      }
      
      static constexpr bool isConst = std::is_const_v<T>;
      static constexpr bool isVoid = std::is_void_v<T>;
      
      std::uint32_t offset;

      // TODO: check if needed
      // Pointer() : offset(0){}
      Pointer() = default;
      Pointer(std::nullptr_t) : offset(0){}

      template<typename U>
      requires (!std::is_same_v<T, U> &&
                isConst >= std::is_const_v<U> &&
                isVoid >= std::is_void_v<U>)
      Pointer(Pointer<U, Traits> p) : offset(p.offset){}

      // oof :'(
      template<std::same_as<void> U> requires(!isVoid)
      explicit Pointer(Pointer<U, Traits> p) : offset(p.offset){}
      explicit Pointer(Pointer<const void, Traits> p) requires(isConst && !isVoid) : offset(p.offset){}

      explicit operator bool() const {return offset != 0;}

      // TODO: stuff noexcept and constexpr all over the place
      auto operator<=>(const Pointer&) const = default;
      bool operator==(const Pointer&) const = default;

      Pointer& operator++(){offset += sizeof(T); return *this;}
      Pointer operator++(int){Pointer cpy = *this; ++*this; return cpy;}
      Pointer& operator--(){offset -= sizeof(T); return *this;}
      Pointer operator==(int){Pointer cpy = *this; --*this; return cpy;}
      Pointer& operator+=(difference_type n){offset += n * sizeof(T); return *this;}
      friend Pointer operator+(Pointer p, difference_type n){p += n; return p;}
      friend Pointer operator+(difference_type n, Pointer p){p += n; return p;}
      Pointer& operator-=(difference_type n){offset -= n * sizeof(T); return *this;}
      friend Pointer operator-(Pointer p, difference_type n){p -= n; return p;}
      friend difference_type operator-(Pointer p, Pointer q){
        return (difference_type)(p.offset - q.offset) / sizeof(T);
      }
      reference operator[](difference_type n) const {return *(*this + n);}

      // TODO: pretty sure smth is broken with nullptr, explore
      
      // TODO: should i cast to char* just in case?
      reference operator*() const {return *reinterpret_cast<T*>(Traits::storage.data() + offset);}
      // seems that `pointer{}.operator->()` does not actually have
      // to be equal to `(T*)nullptr`, haven't seen anything in the standard
      T* operator->() const {return /*offset == 0 ? nullptr :*/ &**this;}
    };

    template<typename T, typename Traits>
    std::ostream& operator<<(std::ostream& out, const Pointer<T, Traits>& p){
      return out << p.offset;
    }

    template<typename Traits>
    struct Allocator {
      using pointer = Pointer<void, Traits>;

      // -1 bc first chunk is not used bc nullptr lives there
      using segment_tree_type = SegmentTree2<Traits::storage.size() / Traits::segment_tree_chunk_size>;

      static std::uint32_t chunk_count(std::uint32_t n){
        return (n + Traits::segment_tree_chunk_size - 1) / Traits::segment_tree_chunk_size;
      }

      // it turns out i prefer shoving metadata into the storage
      // bc having a static variable bloats the executable size
      // added benefit, it occupies nullptr slot
      static segment_tree_type& initialize_segment_tree(){
        static_assert(sizeof(segment_tree_type) < Traits::storage.size());
        // xD i suppose
        auto ptr = std::construct_at<segment_tree_type>(static_cast<segment_tree_type*>(static_cast<void*>(Traits::storage.data())));
        // this is kinda cute tbh
        ptr->take(chunk_count(sizeof(segment_tree_type)));
        return *ptr;
      }

      inline static segment_tree_type& segment_tree = initialize_segment_tree();

      static pointer segment_tree_allocate(std::uint32_t n){
        auto alloc = segment_tree.take(chunk_count(n));
        if (alloc == segment_tree_type::FAILURE) [[unlikely]] {
          throw std::bad_alloc{};
        }
        pointer out;
        out.offset = alloc * Traits::segment_tree_chunk_size;
        return out;
      }
      
      static void segment_tree_deallocate(pointer p, std::uint32_t n){
        auto x = p.offset / Traits::segment_tree_chunk_size;
        segment_tree.give(x, chunk_count(n));
      }

      inline static std::array<pointer, Traits::free_list_limit + 1> free_list_heads{};

      static pointer free_list_allocate(std::uint32_t n){
        pointer& head = free_list_heads[n];
        if (!head) [[unlikely]] {
          auto ptr = segment_tree_allocate(n * Traits::free_list_steal_coef);
          for (std::uint32_t i = 0; i < Traits::free_list_steal_coef; ++i){
            auto newhead = static_cast<Pointer<pointer, Traits>>(ptr);
            ptr.offset += n;
            std::construct_at(&*newhead, head);
            head = newhead;
          }
        }

        auto cpy = head;
        // this looks hilarious
        head = *static_cast<Pointer<pointer, Traits>>(cpy);
        std::destroy_at(&*static_cast<Pointer<pointer, Traits>>(cpy));
        return cpy;
      }

      static void free_list_deallocate(pointer p, std::uint32_t n){
        auto& head = free_list_heads[n];
        auto newhead = static_cast<Pointer<pointer, Traits>>(p);
        std::construct_at(&*newhead, head);
        head = newhead;
      }

      static std::uint32_t align32(std::uint32_t n){
        return (n + 3u) & ~3u;
      }

      static pointer allocate(std::uint32_t n){
        n = align32(n);
        // TODO
        if (n <= Traits::free_list_limit) [[likely]] {
          return free_list_allocate(n);
        } else {
          return segment_tree_allocate(n);
        }
      }
      
      static void deallocate(pointer p, std::uint32_t n){
        n = align32(n);
        // TODO
        if (n <= Traits::free_list_limit) [[likely]] {
          free_list_deallocate(p, n);
        } else {
          segment_tree_deallocate(p, n);
        }
      }
    };

  } // namespace spa_detail

  template<typename T, typename Traits>
  struct SmallPtrAllocator {

    // TODO
    using pointer = spa_detail::Pointer<T, Traits>;
    using const_pointer = spa_detail::Pointer<const T, Traits>;
    using void_pointer = spa_detail::Pointer<void, Traits>;
    using const_void_pointer = spa_detail::Pointer<const void, Traits>;

    using value_type = T;

    using size_type = std::uint32_t;
    using difference_type = std::int32_t;

    template<typename U>
    struct rebind {
      using other = SmallPtrAllocator<U, Traits>;
    };

    static pointer allocate(std::uint32_t n){
      return LOG(n, static_cast<pointer>(spa_detail::Allocator<Traits>::allocate(n * sizeof(T))));
    }
    
    static void deallocate(pointer p, std::uint32_t n){
      LOG(p, n);
      spa_detail::Allocator<Traits>::deallocate(static_cast<void_pointer>(p), n * sizeof(T));
    }
  };

} // namespace ivl::alloc

////////////////////////////////////////////////////
// FINISH <ivl/alloc/small_ptr_allocator.hpp> [1] //
////////////////////////////////////////////////////


//////////////////////////////////////////////////////
// RENDER OF <ivl/alloc/mmap_fixed_storage.hpp> [1] //
//////////////////////////////////////////////////////


#include <cstdint>
#include <stdexcept>
#include <cstdio>
#include <cassert>

#ifdef __unix__
# include <sys/mman.h>
# include <string.h>
#endif

#ifdef _WIN32
# include <memoryapi.h>
# include <windows.h>
#endif

namespace ivl::alloc {

  // pretty sure this should only be used with static storage duration
  template<std::uintptr_t Location, std::size_t Size = (1ULL << 32)>
  struct MmapFixedStorage {
    MmapFixedStorage(){
      fprintf(stderr, "IVL: MMAPING...\n");
      void* base_ptr = reinterpret_cast<void*>(Location);
      
#ifdef __unix__
      fprintf(stderr, "IVL: linux\n");
      void* mmap_ret = mmap(base_ptr, Size,
                            PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS
                            //| MAP_HUGETLB// | MAP_HUGE_2MB
                            | MAP_FIXED_NOREPLACE
                            ,
                            -1, 0);
      if (mmap_ret == MAP_FAILED){
        auto copy = errno;
        perror("mmap");
        fprintf(stderr, "ERR: %s\n", strerror(copy));
        fprintf(stderr, "ERR: %s\n", strerrorname_np(copy));
        fprintf(stderr, "ERR: maybe set /proc/sys/vm/overcommit_memory to 1 if size > total memory\n");
        throw std::runtime_error("failed to secure storage");
      }
#endif

#ifdef _WIN32
      fprintf(stderr, "IVL: windows\n");
      auto handle = CreateFileMappingA
        (INVALID_HANDLE_VALUE,
         nullptr,
         PAGE_READWRITE | SEC_COMMIT,// | SEC_LARGE_PAGES | SEC_COMMIT,
         (std::uint32_t)(Size>>32),
         (std::uint32_t)Size,
         nullptr);
      assert(handle);
      auto ret = MapViewOfFileEx
        (handle,
         FILE_MAP_WRITE,// | FILE_MAP_LARGE_PAGES,
         0,
         0,
         0,
         reinterpret_cast<LPVOID>(Location));
      assert(ret);
      assert(reinterpret_cast<std::uintptr_t>(ret) == Location);
#endif
      
    }

    constexpr char* data(){return reinterpret_cast<char*>(Location);}
    constexpr std::size_t size(){return Size;}

    // TODO: dropped this for now, think about readding
    // // this is probably unnecessary, if static storage duration this
    // // gets destroyed at end of program, and kernel takes the pages over
    // // just in case implementing, maybe other storage duration makes sense?
    // // TODO: probably drop throwing and noexcept(false)
    // ~MmapFixedStorage() noexcept(false){
    //   void* base_ptr = reinterpret_cast<void*>(Location);
    //   int munmap_ret = munmap(base_ptr, Size);
    //   if (munmap_ret == -1){
    //     perror("munmap");
    //     throw std::runtime_error("failed to release storage");
    //   }
    // }
  };

} // namespace ivl::alloc

///////////////////////////////////////////////////
// FINISH <ivl/alloc/mmap_fixed_storage.hpp> [1] //
///////////////////////////////////////////////////

#include <vector>

template<std::size_t N>
struct DynArray {
  std::vector<char> vec;
  DynArray() : vec(N){}
  constexpr char* data(){return vec.data();}
  constexpr std::size_t size(){return N;}
};

struct AllocTraits {
  // inline static DynArray<(128ULL<<20)> storage;
  inline static ivl::alloc::MmapFixedStorage<0x0000'0300'0000'0000, (128ULL<<20)> storage;
  static constexpr std::size_t segment_tree_chunk_size = 64;
  static constexpr std::size_t free_list_limit = 256;
  static constexpr std::size_t free_list_steal_coef = 64;
};

struct AllocDynTraits {
  inline static DynArray<(128ULL<<20)> storage;
  // inline static ivl::alloc::MmapFixedStorage<0x0000'0300'0000'0000, (128ULL<<20)> storage;
  static constexpr std::size_t segment_tree_chunk_size = 64;
  static constexpr std::size_t free_list_limit = 256;
  static constexpr std::size_t free_list_steal_coef = 64;
};

template<typename T>
using Alloc = ivl::alloc::SmallPtrAllocator<T, AllocTraits>;

template<typename T>
using DynAlloc = ivl::alloc::SmallPtrAllocator<T, AllocDynTraits>;

// void consume(int);

// void deref_fancy(Alloc<int>::pointer ptr){
//   consume(*ptr);
// }

// void deref_fancy_dyn(DynAlloc<int>::pointer ptr){
//   consume(*ptr);
// }

// void deref_raw(int* ptr){
//   consume(*ptr);
// }

// void deref_fancy2(Alloc<int>::pointer ptr1, Alloc<int>::pointer ptr2){
//   consume(*ptr1);
//   consume(*ptr2);
// }

// void deref_raw2(int* ptr1, int* ptr2){
//   consume(*ptr1);
//   consume(*ptr2);
// }

// template<typename P>
// int accum(P ptr, std::size_t len){
//   int res = 0;
//   for (std::size_t i = 0; i < len; ++i)
//     res += ptr[i];
//   return res;
// }

// template int accum<int*>(int*, std::size_t);
// template int accum<Alloc<int>::pointer>(Alloc<int>::pointer, std::size_t);

//////////////////////////
// FINISH roman.cpp [0] //
//////////////////////////

