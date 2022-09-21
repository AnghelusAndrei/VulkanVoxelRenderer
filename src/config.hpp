#pragma once

#include <cstdint>
#include <string>

struct Config {
    uint32_t window_width;
    uint32_t window_height;
    std::string window_title;   
    bool debugging_enabled;
    const uint32_t MAX_FRAMES_IN_FLIGHT;
};

