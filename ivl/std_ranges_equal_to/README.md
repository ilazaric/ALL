# figuring out what `std::ranges::equal_to` is all about

looking at c++ draft around jan-2024

located in `<functional>`

[functional.sym](https://eel.is/c++draft/functional.syn) :
```
  namespace ranges {
    // [range.cmp], concept-constrained comparisons
    struct equal_to;                                                                // freestanding
    struct not_equal_to;                                                            // freestanding
    struct greater;                                                                 // freestanding
    struct less;                                                                    // freestanding
    struct greater_equal;                                                           // freestanding
    struct less_equal;                                                              // freestanding
  }
```

[range.cmp](https://eel.is/c++draft/range.cmp) :
```
struct ranges::equal_to {
  template<class T, class U>
    constexpr bool operator()(T&& t, U&& u) const;

  using is_transparent = unspecified;
};
```

`operator()` has a constraint: `std::equality_comparable_with<T, U>`

[range.cmp#3.2](https://eel.is/c++draft/range.cmp#3.2) :
`operator()` is equivalent to `FWD(t) == FWD(u)`

remains to be seen, what exactly is `std::equality_comparable_with<T, U>` ?

## `equality_comparable_with`

[equality_comparable_with](https://eel.is/c++draft/concept.equalitycomparable#concept:equality_comparable_with) :
```
template<class T, class U>
  concept equality_comparable_with =
    equality_comparable<T> && equality_comparable<U> &&
    comparison-common-type-with<T, U> &&
    equality_comparable<
      common_reference_t<
        const remove_reference_t<T>&,
        const remove_reference_t<U>&>> &&
    weakly-equality-comparable-with<T, U>;
```

[comparison-common-type-with](https://eel.is/c++draft/concept.comparisoncommontype)


[equality_comparable](https://eel.is/c++draft/concept.equalitycomparable#concept:equality_comparable) :
```
template<class T>
  concept equality_comparable = weakly-equality-comparable-with<T, T>;
```

[weakly-equality-comparable-with](https://eel.is/c++draft/concept.equalitycomparable#concept:weakly-equality-comparable-with) :
```
template<class T, class U>
  concept weakly-equality-comparable-with = // exposition only
    requires(const remove_reference_t<T>& t,
             const remove_reference_t<U>& u) {
      { t == u } -> boolean-testable;
      { t != u } -> boolean-testable;
      { u == t } -> boolean-testable;
      { u != t } -> boolean-testable;
    };
```
nice and simple
