#include <practice/utils/timing.hpp>

namespace practice::utils {

Timing::Timing() { m_start = std::chrono::steady_clock::now(); }

double Timing::get() const {
  auto now = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_start)
             .count() *
         1e-9;
}

} // namespace practice::utils
