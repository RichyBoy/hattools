import hattools.utils;

import <chrono>;

namespace hattools::utils {
  void Timer::reset() noexcept
  {
    moment = Clock::now();
  }

  double Timer::elapsed() const noexcept
  {
    return std::chrono::duration_cast<Second>(Clock::now() - moment).count();
  }
}
