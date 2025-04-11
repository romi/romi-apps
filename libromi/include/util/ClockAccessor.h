#ifndef _ROMI_CLOCKACCESSOR_H
#define _ROMI_CLOCKACCESSOR_H

#include <memory>
#include <mutex>
#include "util/Clock.h"

namespace romi {

    class ClockAccessor
    {
    public:
            static const std::shared_ptr<IClock>& GetInstance()
            {
                    {
                            std::scoped_lock lock(clock_mutex);
                            if (g_clock == nullptr)
                                    g_clock = std::make_shared<romi::Clock>();
                    }
                    return g_clock;
            }
            
            static void SetInstance(const std::shared_ptr<IClock>& globalClock)
            {
                    std::scoped_lock lock(clock_mutex);
                    g_clock = globalClock;
            }
            
    private:
            static std::mutex clock_mutex;
            static std::shared_ptr<IClock> g_clock;
    };
}

#endif //_ROMI_CLOCKACCESSOR_H
