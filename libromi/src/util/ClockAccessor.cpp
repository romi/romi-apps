#include "util/ClockAccessor.h"

namespace romi {
    std::shared_ptr< IClock > ClockAccessor::g_clock;
    std::mutex ClockAccessor::clock_mutex;
}
