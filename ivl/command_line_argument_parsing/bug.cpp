template<typename... Args>
struct panic {
  [[noreturn]] constexpr explicit panic() {
    if consteval {
      throw 123;
    } else {
      throw 456;
    }
  }
};

struct todo {
  [[noreturn]] constexpr explicit todo() { panic<>(); }
};

// IVL disable_ivl_main_handler()

int main() {}
