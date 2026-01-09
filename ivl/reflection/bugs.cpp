#include <concepts>
#include <meta>

// IVL add_compiler_flags("-freflection -static")

namespace bug1 {

struct[[= 1]] A {};
static_assert(type_of(annotations_of(^^A)[0]) == ^^int);
struct[[= A{}]] B {};
static_assert(type_of(annotations_of(^^B)[0]) == ^^const A);

} // namespace bug1
