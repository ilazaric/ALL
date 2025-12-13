

struct dp1_tag_type {
} dp1_tag;
struct dp2_tag_type {
} dp2_tag;

struct All : DP<int(int, int), dp1_tag>, DP<int(int, int, int), dp2_tag> {
  // int recursive_expression(dp1_tag_type,
};

// normal
struct Env {
  int dp1(int, int) { dp2(...); }

  int dp2(int, int) { dp1(...); }
};

struct Env {
  auto recursive_expression(int a, int b, dp1_tag_type, auto

  auto dp1 = DP{this, [](int a, int b, auto&& Callback){
    return Callback->dp1(a, b);
  }};
};
