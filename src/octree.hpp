#pragma once

#include <stack>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <glm/vec4.hpp>
#define LEAF 0
#define NODE 1

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

        void Insert(glm::uvec4 pos, glm::uvec4 col, unsigned int type);
        void InsertIfFree();
        leaf Lookup();
        void Remove();
    public:
        int depth_;
        int n;
        glm::uvec4 * octree;
        bool upToDate = true;

    private:
        std::stack<int> free_mem;
        long OCTREE_LENGTH = 0;

        int Locate(glm::uvec4 pos, int depth, int p2){
            int a=n/p2;
            int b=n/(p2*2);

            return (int)((pos.x%a)/b + 
                2*((pos.y%a)/b)+
                4*((pos.z%a)/b));
        }

};

Octree::Octree(int depth){
    depth_ = depth;
    n = 1<<depth;
    int o_m = 8;

    for(int i = 0;i<depth;i++){
        OCTREE_LENGTH += o_m;
        o_m *= 8;
    }

    octree = new glm::uvec4[OCTREE_LENGTH];
} 


void Octree::Insert(glm::uvec4 pos, glm::uvec4 col, unsigned int type){
    int d=1;
    int offset = 0;
    int p2 = 1;
    int j = 0;

    while(d<depth_){
        int i = offset+Locate(pos,d,p2);

        switch (octree[i].w)
        {
            case LEAF:
                if(free_mem.empty()){
                    offset = OCTREE_INDEX;
                    OCTREE_INDEX+=8;
                }else{
                    offset = free_mem.top();
                    free_mem.pop();
                }
                octree[i].x =offset;
                octree[i].w = NODE;
                octree[i].z = 0;
                if(d>1)octree[j].z++;
                j = i;
                break;
            case NODE:
                j = i;
                offset = octree[i].y;
                break;
        }

        d++;
        p2*=2;
    }

    int i = offset+Locate(pos,d,p2);
    if(octree[i].w <= 1)octree[j].z++;
    octree[i]={
        col.x,
        col.y,
        col.z,
        type
    };
}