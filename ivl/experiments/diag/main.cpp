consteval {
  __builtin_constexpr_diag(34, "", "error");
  __builtin_constexpr_diag(33, "", "warning");
  __builtin_constexpr_diag(32, "", "note");
}

consteval {
  __builtin_constexpr_diag(2, "", "error");
  __builtin_constexpr_diag(1, "", "warning");
  __builtin_constexpr_diag(0, "", "note");
}

consteval {
  __builtin_constexpr_diag(34, "", "error");
  __builtin_constexpr_diag(33, "", "warning");
  __builtin_constexpr_diag(32, "", "note");
}

consteval {
  __builtin_constexpr_diag(2, "", "error");
}

consteval {
  __builtin_constexpr_diag(34, "", "error");
  __builtin_constexpr_diag(33, "", "warning");
  __builtin_constexpr_diag(32, "", "note");
}

