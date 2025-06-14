

struct debug_tag {
} debug;
struct normal_tag {
} normal;

template <template <typename> typename TT>
struct DP {};

template <typename T>
struct Choose : T {
  std::uint32_t relation(std::uint32_t a, std::uint32_t b, auto&& callback) {
    if (a < b)
      return 0;
    if (a == b || b == 0)
      return 1;
    return callback(a - 1, b - 1) + callback(a - 1, b);
  }
};
