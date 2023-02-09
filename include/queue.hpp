#pragma once

#include <sycl/sycl.hpp>

#include "profiling_interface.hpp"
#include "runtime.hpp"
#include "scaling_interface.hpp"
#include "types.h"
#include "utils.hpp"
#include "vendor_implementations.hpp"

namespace synergy {

class queue : public sycl::queue {
public:
  using base = sycl::queue;

  template <typename... Args, std::enable_if_t<!::details::is_present_v<sycl::property_list, Args...> && !::details::has_property_v<Args...>, bool> = true>
  queue(Args&&... args)
      : base(std::forward<Args>(args)..., sycl::property::queue::enable_profiling{})
  {
    if (!synergy::runtime::is_initialized)
      synergy::runtime::initialize();

    runtime& syn = runtime::get_instance();

    initialize_queue();
  }

  template <typename... Args, std::enable_if_t<::details::is_present_v<sycl::property_list, Args...> || ::details::has_property_v<Args...>, bool> = true>
  queue(Args&&... args)
      : base(std::forward<Args>(args)...)
  {
    initialize_queue();
  }

  template <typename... Args>
  inline sycl::event submit(Args&&... args)
  {
    auto&& event = sycl::queue::submit(std::forward<Args>(args)...);

    // start profiling using profiler

    return event;
  }

  inline double energy_consumption()
  {
    return 0.0;
  }

  inline device<std::any> get_synergy_device()
  {
  }

private:
  device<std::any> device;
  // need a structure to store per-kernel energy consumption
  // need a structure to store queue energy consumption

  void initialize_queue();
};

} // namespace synergy
