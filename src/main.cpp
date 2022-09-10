#include "voxelengine.hpp"
#include "config.hpp"

int main()
{
    Config config { 
        800,
        600,
        "VULKAN",
        true
    };
    VoxelEngine engine(config);
    engine.run();
}