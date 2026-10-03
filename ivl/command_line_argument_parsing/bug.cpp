#include <ivl/utility/scope_exit>
#include <exception>
#include <ivl/format>
#include <memory>
#include <ivl/format>
#include <source_location>
#include <vector>

namespace ivl {
struct base_exception : std::exception {
  struct detail_handle {
    base_exception* ptr;
    int idx;

    inline detail_handle(base_exception& e) : ptr(&e), idx(std::uncaught_exceptions()) {}
  };

  inline static thread_local std::vector<detail_handle> inflight_exceptions{};

  struct context {
    std::source_location location;
    std::string text;
  };

  std::string throw_text;
  std::source_location throw_location;
  std::vector<context> added_context;
  mutable std::unique_ptr<std::string> cached_what;

  inline base_exception(
    std::string_view throw_text = "", std::source_location throw_location = std::source_location::current()
  )
      : throw_text(throw_text), throw_location(throw_location) {
    inflight_exceptions.emplace_back(*this);
  }

  inline ~base_exception() { inflight_exceptions.pop_back(); }

  inline static bool is_in_flight() {
    return !inflight_exceptions.empty() && inflight_exceptions.back().idx + 1 == std::uncaught_exceptions();
  }

  inline void dump(std::FILE* stream = stdout) const {
  }

  virtual inline const char* what() const noexcept {
  }
};
} // namespace ivl


// IVL disable_ivl_main_handler()

int main() {}

// EDG: /usr/bin/ld -plugin /opt/GCC-release/libexec/gcc/x86_64-pc-linux-gnu/17.0.0/liblto_plugin.so -plugin-opt=/opt/GCC-release/libexec/gcc/x86_64-pc-linux-gnu/17.0.0/lto-wrapper -plugin-opt=-fresolution=/tmp/cchPgjUQ.res -plugin-opt=-pass-through=-lgcc -plugin-opt=-pass-through=-lgcc_s_asneeded -plugin-opt=-pass-through=-latomic_asneeded -plugin-opt=-pass-through=-lc -plugin-opt=-pass-through=-lgcc -plugin-opt=-pass-through=-lgcc_s_asneeded --eh-frame-hdr -m elf_x86_64 -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o /home/ilazaric/repos/ALL/ivl/command_line_argument_parsing/bug /lib/x86_64-linux-gnu/crt1.o /lib/x86_64-linux-gnu/crti.o /opt/GCC-release/lib/gcc/x86_64-pc-linux-gnu/17.0.0/crtbegin.o -L/usr/lib/x86_64-linux-gnu -L/home/ilazaric/repos/ALL/build/libraries -L/home/ilazaric/repos/ALL/submodules/objdir/edg/lib -L/opt/GCC-release/lib/gcc/x86_64-pc-linux-gnu/17.0.0 -L/opt/GCC-release/lib/gcc/x86_64-pc-linux-gnu/17.0.0/../../../../lib64 -L/lib/x86_64-linux-gnu -L/lib/../lib64 -L/usr/lib/x86_64-linux-gnu -L/usr/lib/../lib64 -L/opt/GCC-release/lib/gcc/x86_64-pc-linux-gnu/17.0.0/../../.. -L/lib -L/usr/lib -rpath=/opt/GCC-release/lib64 empty.o -lfmt -lpugixml -lraylib -lboost_json -lm -lpthread -lGLU -lm -lrt -lm -ldl -lstdc++ -lgcc_s -lpthread -lC -lgcc -lgcc_s_asneeded -latomic_asneeded -lc -lgcc -lgcc_s_asneeded /opt/GCC-release/lib/gcc/x86_64-pc-linux-gnu/17.0.0/crtend.o /lib/x86_64-linux-gnu/crtn.o
// GCC: /usr/bin/ld -plugin /opt/GCC-release/libexec/gcc/x86_64-pc-linux-gnu/17.0.0/liblto_plugin.so -plugin-opt=/opt/GCC-release/libexec/gcc/x86_64-pc-linux-gnu/17.0.0/lto-wrapper -plugin-opt=-fresolution=/tmp/ccV02nbb.res -plugin-opt=-pass-through=-lgcc_s -plugin-opt=-pass-through=-lgcc -plugin-opt=-pass-through=-latomic_asneeded -plugin-opt=-pass-through=-lc -plugin-opt=-pass-through=-lgcc_s -plugin-opt=-pass-through=-lgcc --eh-frame-hdr -m elf_x86_64 -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o /home/ilazaric/repos/ALL/ivl/command_line_argument_parsing/bug /lib/x86_64-linux-gnu/crt1.o /lib/x86_64-linux-gnu/crti.o /opt/GCC-release/lib/gcc/x86_64-pc-linux-gnu/17.0.0/crtbegin.o -L/home/ilazaric/repos/ALL/build/libraries -L/opt/GCC-release/lib/gcc/x86_64-pc-linux-gnu/17.0.0 -L/opt/GCC-release/lib/gcc/x86_64-pc-linux-gnu/17.0.0/../../../../lib64 -L/lib/x86_64-linux-gnu -L/lib/../lib64 -L/usr/lib/x86_64-linux-gnu -L/usr/lib/../lib64 -L/opt/GCC-release/lib/gcc/x86_64-pc-linux-gnu/17.0.0/../../.. -L/lib -L/usr/lib -rpath=/opt/GCC-release/lib64 /tmp/ccOKgL4G.o -lfmt -lpugixml -lraylib -lboost_json -lpthread -lGLU -lm -lrt -lm -ldl -lstdc++exp -lstdc++ -lm -lgcc_s -lgcc -latomic_asneeded -lc -lgcc_s -lgcc /opt/GCC-release/lib/gcc/x86_64-pc-linux-gnu/17.0.0/crtend.o /lib/x86_64-linux-gnu/crtn.o
