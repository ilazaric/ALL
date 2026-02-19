
namespace very_long_stupid_namespace {
template <typename, typename, typename>
struct very_long_stupid_name {};
template <typename>
struct very_long_stupid_wrapper_1 {};
template <typename>
struct very_long_stupid_wrapper_2 {};
template <typename>
struct very_long_stupid_wrapper_3 {};
} // namespace very_long_stupid_namespace

template <typename>
void consume();

template <typename T, int N>
void fn() {
  if constexpr (N != 0)
    fn<
      very_long_stupid_namespace::very_long_stupid_name<
        very_long_stupid_namespace::very_long_stupid_wrapper_1<T>,
        very_long_stupid_namespace::very_long_stupid_wrapper_2<T>,
        very_long_stupid_namespace::very_long_stupid_wrapper_3<T>>,
      N - 1>();
  else consume<T>();
}

void instantiate() { fn<int, 5>(); }
