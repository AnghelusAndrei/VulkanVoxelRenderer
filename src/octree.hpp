#ifndef OCTREE_HPP
#define OCTREE_HPP 1

#include <stack>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <glm/vec4.hpp>

#include "vulkaninstance.hpp"


#define maxDepth 15

typedef struct leaf;
struct leaf{
    glm::uvec4 data;
    uint32_t size;
    uint32_t index;
};

uint64_t OCTREE_INDEX = 8;

class Octree{
    public:
        Octree(int depth);

        void Insert(glm::uvec4 pos, glm::uvec4 data, uint32_t type);
        void Insert(glm::uvec4 pos, glm::uvec4 data, uint32_t type, bool (*typeCondition)(uint32_t));
        void Remove(glm::uvec4 pos);
        void Remove(glm::uvec4 pos, bool (*typeCondition)(uint32_t));
        void Move(glm::uvec4 from, glm::uvec4 to);
        void Move(glm::uvec4 from, glm::uvec4 to, bool (*typeCondition)(uint32_t));
        leaf Lookup(glm::uvec4 pos);

        void upload();
    public:
        enum Enums{
            LEAF  = 0,
            NODE  = 1,
        };
        
        uint32_t depth_;
        uint32_t n;
        glm::uvec4 * octree;
        bool upToDate = true;

    private:
        std::stack<int> free_mem;
        uint64_t OCTREE_MAX_LENGTH = 0;

        int Locate(glm::uvec4 pos, uint32_t depth, int p2){
            int a=n/p2;
            int b=n/(p2*2);

            return (int)((pos.x%a)/b + 
                2*((pos.y%a)/b)+
                4*((pos.z%a)/b));
        }

};

#endif