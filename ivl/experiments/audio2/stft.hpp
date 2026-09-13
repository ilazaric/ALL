#pragma once

#include <cmath>
#include <complex>
#include <fftw3.h>
#include <print>
#include <span>
#include <vector>

static_assert(sizeof(std::complex<double>) == sizeof(fftw_complex));

struct fft_executor {
  size_t n;
  std::complex<double>*in, *out;
  fftw_plan fp, bp;

  explicit fft_executor(size_t n) : n(n) {
    in = (std::complex<double>*)fftw_malloc(sizeof(fftw_complex) * n);
    out = (std::complex<double>*)fftw_malloc(sizeof(fftw_complex) * n);
    fp = fftw_plan_dft_1d(
      n, reinterpret_cast<fftw_complex*>(in), reinterpret_cast<fftw_complex*>(out), FFTW_FORWARD, FFTW_ESTIMATE
    );
    bp = fftw_plan_dft_1d(
      n, reinterpret_cast<fftw_complex*>(in), reinterpret_cast<fftw_complex*>(out), FFTW_BACKWARD, FFTW_ESTIMATE
    );
  }

  std::vector<std::complex<double>> forward(auto&& input) {
    contract_assert(input.size() == n);
    std::ranges::copy(input, in);
    fftw_execute(fp);
    return std::vector(out, out + n);
  }

  std::vector<std::complex<double>> backward(auto&& input) {
    contract_assert(input.size() == n);
    std::ranges::copy(input, in);
    fftw_execute(bp);
    return std::vector(out, out + n);
  }

  std::vector<double> amps(std::span<const double> input) {
    contract_assert(input.size() == n);
    for (size_t i = 0; i < n; ++i) in[i] = input[i];
    fftw_execute(fp);
    std::vector<double> ret(n);
    for (size_t i = 0; i < n; ++i) ret[i] = std::abs(out[i]);
    return ret;
  }

  ~fft_executor() {
    fftw_destroy_plan(fp);
    fftw_destroy_plan(bp);
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
