#pragma once

#include <stack>
#include <functional>
#include <cstdlib>

#include "vulkaninstance.hpp"

#include <glm/vec3.hpp>

#define maxDepth 32

class VulkanInstance;
class Octree
{
public:

    struct Node {
        unsigned isNode:1;
        unsigned count: 3;
        unsigned next : 28;
    };

    struct Leaf {
        unsigned isNode:1;
        unsigned type:7;
        unsigned data:24;
    };
    
    enum TYPES{
        EMPTY = 0,
        DEFAULT = 1
    };

    Node *nodes_;

    Octree(uint32_t depth);
    void upload(VulkanInstance *instance);

    uint32_t lookup(glm::uvec3 position);
    void insert(glm::uvec3 position, Leaf data);
    //void insert(glm::uvec3 position, Leaf data, std::function<bool(uint8_t)> func);
    //void remove(glm::uvec3 position);
    //void remove(glm::uvec3 position, std::function<bool(uint8_t)> func);
    //void move(glm::uvec3 position1, glm::uvec3 position2);
    //void move(glm::uvec3 position1, glm::uvec3 position2, std::function<int(uint8_t, uint8_t)> func); // func = 0 -> canceled, func = 1 -> delete v1, func = 2 -> success

    static uint32_t utils_rgb(uint8_t r, uint8_t g ,uint8_t b) { return ((r << 16) | (g<<8) | b);}
    size_t capacity_ = 0;
private:
    
    const uint32_t depth_;
    uint32_t newNode = 0;
    std::stack<uint32_t> freeNodes;

    uint32_t utils_p2r[maxDepth];
    uint32_t utils_locate(glm::uvec3 position, uint32_t depth);
    bool contained(glm::uvec3 position1, glm::uvec3 position2, uint32_t depth);
};