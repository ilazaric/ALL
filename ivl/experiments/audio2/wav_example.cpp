#include <ivl/command_line_argument_parsing/implicit_exposed>
#include "common"
#include "stft"
#include "visuals"
#include "wav"
#include <algorithm>
#include <cmath>
#include <numbers>
#include <ranges>

// IVL add_compiler_flags_tail("-lfftw3 -lm")

// IVL add_compiler_flags("-fno-strict-aliasing")
// IVL add_compiler_flags_tail("-L/home/ilazaric/repos/ALL/submodules/objdir/raylib/raylib/ -lraylib")
// IVL add_compiler_flags_tail("-lm  -lpthread -lOpenGL  -lGLX  -lGLU  -lm  -lrt  -lm  -ldl")

// IVL add_compiler_flags("-Wno-non-template-friend -Wsfinae-incomplete=0")

int ivl_main(ivl::cmdline_parsing::implicit, const std::filesystem::path& file) {
  auto p = ivl::wav::load(file);
  contract_assert(p.format_type == 1);
  contract_assert(p.bytes_per_block % p.channel_count == 0);
  contract_assert(p.bytes_per_block / p.channel_count == 2);

  if (implicit_flag("save_test")) {
    save(p, "copy.wav");
  }

  if (implicit_flag("split_merge_test")) {
    auto v = channel_split(p);
    auto q = ivl::wav::channel_merge(v);
    contract_assert(q == p);
    for (size_t i = 0; i < v.size(); ++i) save(v[i], std::format("channel_{}.wav", i));
  }

  if (implicit_flag("synthesize_sine")) {
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

  if (implicit_flag("stft_test_1")) {
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

  if (implicit_flag("stft_visualise")) {
    auto a = extract(p, 0);
    auto b = stft_amps(a, 1ull << 16, 1ull << 14);
    stft_visualise(b, (double)p.sample_rate);
  }

  if (implicit_flag("stft_visualise_sine")) {
    std::vector<double> a;
    double freq = 200.0;
    double sample_rate = (double)p.sample_rate;
    for (size_t i = 0; i < (1ull << 24); ++i) {
      double t = (double)i / sample_rate;
      a.push_back(std::sin(t * freq * 2 * std::numbers::pi));
    }
    auto b = stft_amps(a, 1ull << 16, 1ull << 14);
    stft_visualise(b, sample_rate);
  }

  if (implicit_flag("stft_visualise_multi")) {
    window w(1000, 1000, "visualiser");
    w.set_target_fps(30);

    plot p(w.width, w.height);
    p.axis_font_size = 20;
    p.value_font_size = 20;
    p.set_all_padding(5);
    p.render_x_axis_label = true;
    p.x_axis_label = "frequency";
    p.render_x_axis_values = true;
    for (double freq : {20.0, 50.0, 100.0, 200.0, 500.0, 1'000.0, 2'000.0, 5'000.0, 10'000.0, 20'000.0}) {
      p.x_axis_values.emplace_back(std::log(freq), std::format("{}Hz", freq));
    }

 
  }

  return 0;
}
