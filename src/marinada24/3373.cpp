#include <ivl/logger>

const uint64_t N = 999;
uint64_t       attempt(uint64_t a, uint64_t b) { return a * (N - b) * b * (N - a) * 2; }

int main() {
  {
    uint64_t maks = 0;
    for (uint64_t a = 0; a <= N; ++a)
      for (uint64_t b = 0; b <= N; ++b)
        maks = std::max(maks, attempt(a, b));
    LOG(maks);
  }

  {
    auto max3 = N / 3 * N / 3 * N / 3 * N / 3 * 6;
    LOG(max3);
  }

  {
    auto mxi3 = N / 3 * N / 3 * 3 * (2 * N / 3) * (2 * N / 3);
    LOG(mxi3);
  }
}
