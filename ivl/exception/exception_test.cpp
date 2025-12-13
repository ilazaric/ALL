#include <ivl/exception>
#include <cassert>

#ifdef NDEBUG
#error "i need assert() to work"
#endif

void test_basic() {
  {
    auto func = [](bool b) {
      EXCEPTION_CONTEXT("arg: {}", b);
      if (b) throw ivl::base_exception{"func"};
      else return 42;
    };
    assert(func(false));
    try {
      func(true);
    } catch (const ivl::base_exception& e) {
      assert(e.throw_text == "func");
      assert(e.added_context.size() == 1);
      assert(e.added_context[0].text == "arg: true");
    }
  }

  {
    EXCEPTION_CONTEXT("bla");
    try {
      EXCEPTION_CONTEXT("truc");
      throw ivl::base_exception{"ex"};
    } catch (const ivl::base_exception& e) {
      assert(e.throw_text == "ex");
      assert(e.added_context.size() == 1);
      assert(e.added_context[0].text == "truc");
    }
  }

  {
    std::exception_ptr eptr;
    try {
      EXCEPTION_CONTEXT("a");
      throw ivl::base_exception{"b"};
    } catch (const ivl::base_exception& e) {
      eptr = std::current_exception();
    }
    try {
      EXCEPTION_CONTEXT("c");
      std::rethrow_exception(eptr);
    } catch (const ivl::base_exception& e) {
      assert(e.added_context.size() == 2);
    }
  }
}

int main() {
  test_basic();
  std::println("ALL PASSED");
}
