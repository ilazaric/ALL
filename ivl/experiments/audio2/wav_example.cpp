#include <ivl/command_line_argument_parsing/implicit_exposed>
#include "bin"
#include "common"
#include "equalizer_config"
#include "limiter"
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
    LOG(a.size());
    while (!std::has_single_bit(a.size())) a.push_back(0);
    auto b = stft_amps(a, a.size(), a.size());
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
    auto a = extract(p, 0);
    LOG(a.size());
    auto sample_rate = (double)(p.sample_rate);
    window w(1000, 1000, "visualiser");
    w.set_target_fps(30);
    freq_plot p(w.width, w.height);
    p.y_range.first = p.y_range.second = 0;

    size_t len = 1 << 16;
    size_t base = (size_t)sample_rate * 30;
    fft_executor f(len);
    size_t mv = 1 << 17;
    auto bin_count = implicit_value("bin_count", 20);
    for (size_t i = 0; i * mv + base + len <= a.size() && i < bin_count; ++i) {
      LOG(i);
      auto input = f.amps(std::span(a).subspan(base + i * mv).subspan(0, len));
      // input = binned(input, 1 << 6);
      p.sequences.emplace_back();
      p.sequences.back().color = Fade(ColorLerp(RED, GREEN, (double)i / (double)(bin_count - 1)), 1.0f);
      auto& points = p.sequences.back().points;
      for (size_t i = 0; i < input.size() / 2; ++i) {
        double freq = sample_rate * (double)i / (double)input.size();
        if (freq < min_freq / 2) continue;
        if (freq > max_freq * 2) break;
        double log_freq = std::log(freq);
        points.emplace_back(log_freq, input[i]);
      }
      points = x_binned(points, (max_log_freq - min_log_freq) / 25.0);
      p.y_range.second = std::max(p.y_range.second, (double)std::ranges::max(points, {}, &Vector2::y).y);
    }

    p.internal_render();
    while (!w.should_close()) {
      BeginDrawing();
      ClearBackground(GREEN); // to see mistakes
      DrawTexture(p.RT.texture, 0, 0, WHITE);
      EndDrawing();
    }
  }

  // ./wav_example --equalizer_test --amp_reduce_db=0 --equalizer_config '+1db < 100hz < +0db < 250hz < -10db'
  // --limiter_limit=0.95 --limiter_decay=1.0001 never-fade-away.wav
  if (implicit_flag("equalizer_test")) {
    auto lambda = [&](auto&& a) {
      auto eqcfg = implicit_value<equalizer_config>("equalizer_config");
      auto db = [](double x) { return std::pow(10.0, x / 10.0); };
      // auto a = extract(p, 0);
      auto sample_rate = (double)p.sample_rate;
      LOG(sample_rate);
      LOG(a.size());
      size_t window = 1 << 12;
      size_t hop = window / 2;
      contract_assert(hop * 2 == window);
      fft_executor f(window);
      auto front_padding = window;
      auto back_padding = window + (window - a.size() % window) % window;
      a.insert_range(a.begin(), std::views::repeat(0, front_padding));
      a.insert_range(a.end(), std::views::repeat(0, back_padding));
      contract_assert(a.size() % window == 0);
      std::vector<double> out(a.size(), 0.0);
      std::vector<double> hann(window, 0.0);
      for (size_t n = 0; n < window; ++n)
        hann[n] = (1.0 - std::cos(2 * std::numbers::pi * (double)n / (double)window)) / 2.0;
      LOG(hann[window / 2]);
      // const double coef_freq = implicit_value("coef_freq", 0.5);
      for (size_t i = 0; i + window <= a.size(); i += hop) {
        double t = (double)i / sample_rate;
        // double coef = (1.0 - std::cos(t * coef_freq * 2 * std::numbers::pi)) / 2.0;
        auto stft = f.forward(std::span(a).subspan(i).subspan(0, window));
        for (size_t j = 0; j < window; ++j) {
          auto& curr = stft[j];
          double freq = sample_rate * (double)(j < window / 2 ? j : window - j) / (double)window;
          curr *= db(eqcfg.get_db(freq) /* * coef */);
        }
        auto back = f.backward(stft);
        for (size_t j = 0; j < window; ++j) out[i + j] += back[j].real() * hann[j];
      }
      out.erase(out.begin(), out.begin() + front_padding);
      out.erase(out.end() - back_padding, out.end());
      for (auto&& el : out) el /= (double)window * db(implicit_value("amp_reduce_db", 2.5));
      out = limiter(out);
      auto q = synthesize(out, p);
      return q;
    };
    auto q = ivl::wav::channel_merge({lambda(extract(p, 0)), lambda(extract(p, 1))});
    save(q, "equalized.wav");
  }

  // doesnt seem to work, TODO
  if (implicit_flag("octave_test_failed")) {
    auto lambda = [&](auto&& a) {
      auto sample_rate = (double)p.sample_rate;
      LOG(sample_rate);
      LOG(a.size());
      size_t window = 1 << 17;
      size_t hop = window / 2;
      contract_assert(hop * 2 == window);
      fft_executor f(window);
      fft_executor fh(hop);
      auto front_padding = window;
      auto back_padding = window + (window - a.size() % window) % window;
      a.insert_range(a.begin(), std::views::repeat(0, front_padding));
      a.insert_range(a.end(), std::views::repeat(0, back_padding));
      contract_assert(a.size() % window == 0);
      std::vector<double> out(a.size(), 0.0);
      // std::vector<double> hann(window, 0.0);
      // for (size_t n = 0; n < window; ++n)
      //   hann[n] = (1.0 - std::cos(2 * std::numbers::pi * (double)n / (double)window)) / 2.0;
      std::vector<double> hannh(hop, 0.0);
      for (size_t n = 0; n < hop; ++n)
        hannh[n] = (1.0 - std::cos(2 * std::numbers::pi * (double)n / (double)hop)) / 2.0;
      // LOG(hann[window / 2]);
      for (size_t i = 0; i + window <= a.size(); i += hop / 2) {
        auto stft = f.forward(std::span(a).subspan(i).subspan(0, window));
        std::vector<std::complex<double>> stfth;
        stfth.insert_range(stfth.end(), std::span(stft).subspan(0, hop / 2));
        stfth.insert_range(stfth.end(), std::span(stft).subspan(hop + hop / 2));
        auto back = fh.backward(stfth);
        for (size_t j = 0; j < hop; ++j) out[i + j] += back[j].real() * hannh[j];
      }
      out.erase(out.begin(), out.begin() + front_padding);
      out.erase(out.end() - back_padding, out.end());
      for (auto&& el : out) el /= (double)window * 1.5;
      auto q = synthesize(out, p);
      return q;
    };
    auto q = ivl::wav::channel_merge({lambda(extract(p, 0)), lambda(extract(p, 1))});
    save(q, "octave.wav");
  }

  return 0;
}
