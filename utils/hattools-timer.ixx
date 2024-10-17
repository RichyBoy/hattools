export module hattools.utils:timer;

import <chrono>;
import <ratio>;

export namespace hattools::utils {
  class Timer
  {
  private:
    using Clock = std::chrono::steady_clock;
    using Second = std::chrono::duration<double, std::ratio<1>>;

    std::chrono::time_point<Clock> moment{ Clock::now() };

  public:
    void reset() noexcept;
    double elapsed() const noexcept;
  };
}
