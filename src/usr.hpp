#ifndef USER_HPP
#define USER_HPP 1

#include "vulkaninstance.hpp"

class usr{
public:
    usr(VoxelEngine *engine);
    void Interactive();
    void Scene();
private:
    VoxelEngine *engine_;
public:
    usr();
};

#endif