#include "vulkaninstance.hpp"

int main()
{
    Config config ={ 
        .window_width=800,
        .window_height=600,
        .window_title="VULKAN"
    };
    VoxelEngine engine;

    engine.run();
}