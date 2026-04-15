#pragma once

#include <functional>
#include <vector>
#include <string>

using PeriapsisCallback = std::function<void(std::string body_name, double time)>;

class EventBus {
public:
    // A list of all functions waiting for a periapsis event
    static std::vector<PeriapsisCallback> OnPeriapsisReached;

    // The trigger function called by the physics engine
    static void BroadcastPeriapsis(std::string body_name, double time) {
        for (const auto& listener : OnPeriapsisReached) {
            listener(body_name, time); // Fire the event!
        }
    }
};