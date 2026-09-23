#include <ivl/reflection/test_attribute>
#include <ivl/testing>
#include "default"

// IVL test_only()

#ifdef IVL_KIND_TEST
[[= ivl::test]] void test1() {
  auto state = ivl::parsing::make::parse_text(R"make(
a=1
b=2 # comment
# c=3\
e=4
d=x\
  y\
 #z
)make");
  ivl::testing::contract_assert_json(state.variables, R"json(
    [{
      "contents": "1",
      "name": "a",
      "overriden": false,
      "recursively_expanded": true
    },
     {
       "contents": "2",
       "name": "b",
       "overriden": false,
       "recursively_expanded": true
     },
     {
       "contents": "x  y",
       "name": "d",
       "overriden": false,
       "recursively_expanded": true
     }]
  )json");
}
#endif // IVL_KIND_TEST
