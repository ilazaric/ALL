#include "wav"
#include <cmath>
// #include <complex.h>
#include <complex>
#include <fftw3.h>
// #include <raylib/raylib.h>
#include <algorithm>
#include <ranges>

#include <raylib/raylib.h>

// IVL add_compiler_flags_tail("-lfftw3 -lm")

// IVL add_compiler_flags("-fno-strict-aliasing")
// IVL add_compiler_flags_tail("-L/home/ilazaric/repos/ALL/submodules/objdir/raylib/raylib/ -lraylib")
// IVL add_compiler_flags_tail("-lm  -lpthread -lOpenGL  -lGLX  -lGLU  -lm  -lrt  -lm  -ldl")

// "The standard audible frequency range for humans spans from 20 Hz to 20,000 Hz"

static_assert(sizeof(std::complex<double>) == sizeof(fftw_complex));

struct fft_executor {
  size_t n;
  std::complex<double>*in, *out;
  fftw_plan p;

  explicit fft_executor(size_t n) : n(n) {
    in = (std::complex<double>*)fftw_malloc(sizeof(fftw_complex) * n);
    out = (std::complex<double>*)fftw_malloc(sizeof(fftw_complex) * n);
    p = fftw_plan_dft_1d(
      n, reinterpret_cast<fftw_complex*>(in), reinterpret_cast<fftw_complex*>(out), FFTW_FORWARD, FFTW_ESTIMATE
    );
  }

  void execute() { fftw_execute(p); }

  std::vector<double> amps(std::span<const double> input) {
    contract_assert(input.size() == n);
    for (size_t i = 0; i < n; ++i) in[i] = input[i];
    execute();
    std::vector<double> ret(n);
    for (size_t i = 0; i < n; ++i) ret[i] = std::abs(out[i]);
    return ret;
  }

  ~fft_executor() {
    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);
  }
};

std::vector<double> stft_amps(std::span<const double> in, size_t window_size, size_t hop) {
  fft_executor ex(window_size);
  std::vector<double> acc(window_size);
  // a.insert_range(a.begin(), std::views::repeat(0, window_size - 1));
  for (size_t i = 0; i + window_size <= in.size(); i += hop) {
    std::print("{} / {}\r", i + window_size, in.size());
    auto b = ex.amps(in.subspan(i, window_size));
    for (size_t j = 0; j < window_size; ++j) acc[j] += b[j];
  }
  std::println();
  return acc;
}

void stft_visualise(std::span<const double> input, double sample_rate) {
  contract_assert(!input.empty());
  std::vector<size_t> indices(std::from_range, std::views::iota(0ull, input.size()));
  std::ranges::sort(indices, std::ranges::greater{}, [&](size_t i) { return input[i]; });
  for (size_t i = 0; i < indices.size() && i < 200; ++i) {
    LOG(i, indices[i], input[indices[i]]);
  }
  {
    const int screenWidth = 1000;
    const int screenHeight = 1000;
    std::vector<Vector2> points;
    double my = std::ranges::max(input);
    double min_freq = 20.0;
    double max_freq = 20'000.0;
    double min_log_freq = std::log(min_freq);
    double max_log_freq = std::log(max_freq);
    for (size_t i = 0; i < input.size() / 2; ++i) {
      double freq = sample_rate * (double)i / (double)input.size();
      if (freq < min_freq) continue;
      if (freq > max_freq) break;
      double log_freq = std::log(freq);
      auto x = (double)screenWidth * (log_freq - min_log_freq) / max_log_freq;
      auto y = (double)screenHeight * double(input[i]) / my;
      points.emplace_back(x, y);
    }
    InitWindow(screenWidth, screenHeight, "visualiser");
    SetTargetFPS(30);
    RenderTexture2D RT = LoadRenderTexture(screenWidth, screenHeight);
    {
      BeginTextureMode(RT);
      ClearBackground(WHITE);
      DrawLineStrip(points.data(), (int)points.size(), BLUE);
      DrawLine(0, 0, screenWidth, screenHeight, GRAY);
      EndTextureMode();
    }
    contract_assert(points.size() < (1ull << 31));
    while (!WindowShouldClose()) {
      BeginDrawing();
      ClearBackground(PINK); // to see mistakes
      DrawTexture(RT.texture, 0, 0, WHITE);
      EndDrawing();
    }
    CloseWindow();
    UnloadRenderTexture(RT);
  }
}

int ivl_main(const std::filesystem::path& file) {
  auto p = ivl::wav::load(file);
  contract_assert(p.format_type == 1);
  contract_assert(p.bytes_per_block % p.channel_count == 0);
  contract_assert(p.bytes_per_block / p.channel_count == 2);
  save(p, "copy.wav");

  {
    auto v = channel_split(p);
    auto q = ivl::wav::channel_merge(v);
    contract_assert(q == p);
    for (size_t i = 0; i < v.size(); ++i) save(v[i], std::format("channel_{}.wav", i));
  }

  {
    contract_assert(p.channel_count == 2);
    auto a = extract(p, 0);
    auto b = extract(p, 1);
    contract_assert(a.size() == b.size());
    std::vector<double> c(a.size());
    double freq = 3.0;
    for (size_t i = 0; i < c.size(); ++i) {
      double t = i;
      t /= p.sample_rate;
      double s = std::sin(t * freq);
      double u = (1 + s) / 2;
      c[i] = a[i] * u + b[i] * (1 - u);
    }
    auto q = synthesize(c, p);
    save(q, "mix.wav");
  }

  if (0) {
    auto a = extract(p, 0);
    size_t window_size = 1024;
    fft_executor ex(window_size);
    std::vector<double> acc(window_size);
    // a.insert_range(a.begin(), std::views::repeat(0, window_size - 1));
    for (size_t i = 0; i + window_size <= a.size(); ++i) {
      std::print("{} / {}\r", i + window_size, a.size());
      auto b = ex.amps(std::span(a).subspan(i, window_size));
      // auto m = std::ranges::max_element(b);
      // auto f = m - b.begin();
      // LOG(f, b[f]);
      for (size_t j = 0; j < window_size; ++j) acc[j] += b[j];
    }
    std::println();
  }

  {
    auto a = extract(p, 0);
    auto b = stft_amps(a, 1ull << 16, 1ull << 14);
    stft_visualise(b, (double)p.sample_rate);
  }

  return 0;
}
