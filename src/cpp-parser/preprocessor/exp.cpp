
template <typename... Ts>
  requires(sizeof...(Ts) > 0)
struct Var {};

// Var<> is problematic

void consume(auto&&) {
}

template <typename... Ts>
void consume(const Var<Ts...>&) {
}

template <typename = void>
void noop() {
}

int main() {
  consume(&noop);
}
