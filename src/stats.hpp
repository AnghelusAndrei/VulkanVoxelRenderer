#pragma once

#include "voxelengine.hpp"

struct Stats{
    double FPS;
    double MS;

    double time1, time2 = glfwGetTime();
    void Update(){
        time1 = time2;
        time2 = glfwGetTime();

        MS = time2-time1;
        FPS = 1000/MS;
    }
};
