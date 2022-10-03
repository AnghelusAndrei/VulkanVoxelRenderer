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

    void insert(glm::uvec3 position, Node data);
    void insert(glm::uvec3 position, Node data, std::function<bool(uint32_t)> func);
    void remove(glm::uvec3 position);
    void remove(glm::uvec3 position, std::function<bool(uint32_t)> func);
    void move(glm::uvec3 position1, glm::uvec3 position2);
    void move(glm::uvec3 position1, glm::uvec3 position2, std::function<bool(uint32_t, uint32_t)> func);

    static uint32_t utils_rgb(uint8_t r, uint8_t g ,uint8_t b) { return ((r << 16) | (g<<8) | b);}
private:
    size_t capacity_;
    const uint32_t depth_;
    uint32_t newNode;
    Node *nodes_;
    std::stack<uint32_t> freeNodes;

    uint32_t utils_p2r[maxDepth];
    uint32_t utils_locate(glm::uvec3 position, uint32_t depth);
    uint32_t utils_lookup(glm::uvec3 position);
};