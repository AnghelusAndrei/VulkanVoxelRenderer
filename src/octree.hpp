#pragma once

#include <array>

#include <cstdint>

#include <glm/vec3.hpp>

#include "logging.hpp"
class Octree
{
public:
    #pragma pack(push, 1)
    struct __attribute__((packed)) Leaf
    {
        unsigned isLeaf : 1;
        unsigned deleted : 1;
        unsigned _reserved : 2;
        unsigned data : 28;
    };
    Octree(uint32_t depth);
    size_t capacity();
    size_t size();
    ~Octree();
    void insert(glm::uvec3 position, Leaf data);

private:
    std::tuple<Octree::Leaf, uint32_t, int> utils_lookup(glm::uvec3 position);
    int utils_locate(glm::uvec3 position, uint32_t depth);
    const uint32_t depth_;
    std::vector<Leaf> data_;
    uint32_t utils_p2r[32]; // Reversed powers of two
};