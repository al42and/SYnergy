#pragma once

#include <future>
#include <thread>
#include <vector>

#include "device.hpp"
#include "kernel.hpp"
#include "profilers.hpp"

namespace synergy {

namespace detail {

class profiling_manager {
public:
  friend class fine_grained_profiler<profiling_manager>;
  friend class coarse_grained_profiler<profiling_manager>;

  profiling_manager(device& device) : device{device} {
    device_profiler = std::thread{coarse_grained_profiler<profiling_manager>{*this}};
  }

  ~profiling_manager() {
    finished.store(true, std::memory_order_release);
    device_profiler.join();
  }

  void profile_kernel(sycl::event event) {
    kernels.push_back(kernel{event});
    auto future = std::async(std::launch::async, fine_grained_profiler<profiling_manager>{*this, kernels.back()});
    // this will automatically serialize all profiled kernels
    // since the std::future destructor will wait until the async thread ends
  }

  double kernel_energy(const sycl::event& event) const {
    kernel dummy{event};
    auto it = std::find(kernels.begin(), kernels.end(), dummy);
    if (it == kernels.end()) {
      throw std::runtime_error("synergy::queue error: kernel was not submitted to the queue");
    }

    return it->energy;
  }

  double device_energy() const {
    return device_energy_consumption;
  }

private:
  device device;
  double device_energy_consumption = 0.0;
  std::vector<kernel> kernels;
  std::atomic<bool> finished = false;

  std::thread device_profiler;
};

} // namespace detail

} // namespace synergy
