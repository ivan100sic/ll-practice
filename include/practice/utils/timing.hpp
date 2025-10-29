#pragma once

#include <chrono>

namespace practice::utils {

class Timing {
public:
  Timing();

  double get() const;

private:
  std::chrono::steady_clock::time_point m_start;
};

} // namespace practice::utils
