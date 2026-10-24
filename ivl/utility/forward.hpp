#pragma once

#include <utility>

#define FWD(x) std::forward<decltype(x)>(x)
