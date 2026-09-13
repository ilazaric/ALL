#pragma once

#include <cmath>

// "The standard audible frequency range for humans spans from 20 Hz to 20,000 Hz"

constexpr double min_freq = 20.0;
constexpr double max_freq = 20'000.0;
constexpr double min_log_freq = std::log(min_freq);
constexpr double max_log_freq = std::log(max_freq);
 
