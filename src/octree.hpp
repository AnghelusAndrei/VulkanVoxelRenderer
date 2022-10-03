#pragma once

#include <stack>
#include <functional>

#include "vulkaninstance.hpp"

#include <glm/vec3.hpp>

#define maxDepth 32

class VulkanInstance;
class Octree
{
public:
    struct __attribute__((packed)) Node
    {
        unsigned isNode : 1;
        union __attribute__((packed))
        {
            struct __attribute__((packed)) 
            {
                unsigned count: 3;
                unsigned next : 28;
            } node;
            struct __attribute__((packed)) 
            {
                unsigned type : 7;
                unsigned data: 24;
            } leaf;

        };
    };

    Octree(uint32_t depth);
    void upload(VulkanInstance *instance);

    uint32_t lookup(glm::uvec3 position);
    void insert(glm::uvec3 position, Node data);
    void insert(glm::uvec3 position, Node data, std::function<bool(uint8_t)> func);
    void remove(glm::uvec3 position);
    void remove(glm::uvec3 position, std::function<bool(uint8_t)> func);
    void move(glm::uvec3 position1, glm::uvec3 position2);
    void move(glm::uvec3 position1, glm::uvec3 position2, std::function<int(uint8_t, uint8_t)> func); // func = 0 -> canceled, func = 1 -> delete v1, func = 2 -> success

    static uint32_t utils_rgb(uint8_t r, uint8_t g ,uint8_t b) { return ((r << 16) | (g<<8) | b);}
private:
    size_t capacity_ = 0;
    const uint32_t depth_;
    uint32_t newNode = 0;
    Node *nodes_;
    std::stack<uint32_t> freeNodes;

    uint32_t utils_p2r[maxDepth];
    uint32_t utils_locate(glm::uvec3 position, uint32_t depth);
    bool areContained(glm::uvec3 position1, glm::uvec3 position2, uint32_t depth);
};