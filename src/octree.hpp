#pragma once

#include <stack>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <glm/vec4.hpp>

#include "vulkaninstance.hpp"


#define maxDepth 15

enum OctreeEnums{
    LEAF  = 0,
    NODE  = 1,
    VOXEL = 2
};

typedef struct leaf;
struct leaf{
    glm::uvec4 data;
    unsigned int size;
    unsigned int index;
};

unsigned long OCTREE_INDEX = 8;

class Octree{
    public:
        Octree(int depth);

        void Insert(glm::uvec4 pos, glm::uvec4 data, unsigned int type);
        void Insert(glm::uvec4 pos, glm::uvec4 data, unsigned int type, bool (*typeCondition)(unsigned int));
        void Remove(glm::uvec4 pos);
        void Remove(glm::uvec4 pos, bool (*typeCondition)(unsigned int));
        void Move(glm::uvec4 from, glm::uvec4 to);
        void Move(glm::uvec4 from, glm::uvec4 to, bool (*typeCondition)(unsigned int));
        leaf Lookup(glm::uvec4 pos);

        void upload(int error);
    public:
        vk::Buffer buffer;
        
        int depth_;
        int n;
        glm::uvec4 * octree;
        bool upToDate = true;

    private:
        std::stack<int> free_mem;
        long OCTREE_MAX_LENGTH = 0;

        int Locate(glm::uvec4 pos, int depth, int p2){
            int a=n/p2;
            int b=n/(p2*2);

            return (int)((pos.x%a)/b + 
                2*((pos.y%a)/b)+
                4*((pos.z%a)/b));
        }

};
