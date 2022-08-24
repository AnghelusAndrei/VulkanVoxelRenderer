#pragma once

#include <stack>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <glm/vec4.hpp>
#define LEAF 0
#define NODE 1

typedef struct leaf;
struct leaf{
    unsigned int data[4];
    unsigned int size;
    unsigned int index;
};

unsigned long OCTREE_INDEX = 8;

class Octree{
    public:
        Octree(int depth);

        bool Insert();
        bool InsertIfFree();
        leaf Lookup();
        bool Remove();
    public:
        int depth_;
        int n;
        glm::u32vec4 * octree;
        bool upToDate = true;

    private:
        std::stack<int> free_mem;
        long OCTREE_LENGTH = 0;

        int Locate(glm::vec3 pos, int depth, int p2){
            int a=n/p2;
            int b=n/(p2*2);

            return (pos.x%a)/b + 
                2*((pos.y%a)/b)+
                4*((pos.z%a)/b);
        }

}

Octree::Octree(int depth){
    depth_ = depth;
    n = 1<<depth;
    int o_m = 8;

    for(int i = 0;i<depth;i++){
        OCTREE_LENGTH += o_m;
        o_m *= 8;
    }

    octree = new glm::uint4[OCTREE_LENGTH];
} 


void Octree::Insert(glm::u32vec4 pos, unsigned char col[4], unsigned int type){
    int d=1;
    int offset = 0;
    int p2 = 1;
    int j = 0;

    while(d<depth_){
        int i = offset+Locate(pos,d,p2);

        switch (octree[i][3])
        {
            case LEAF:
                if(free_mem.empty()){
                    offset = OCTREE_INDEX;
                    OCTREE_INDEX+=8;
                }else{
                    offset = free_mem.top();
                    free_mem.pop();
                }
                octree[i][0]=offset;
                octree[i][3] = NODE;
                octree[i][2] = 0;
                if(d>1)octree[j][2]++;
                j = i;
                break;
            case NODE:
                j = i;
                offset = octree[i][1];
                break;
        }

        d++;
        p2*=2;
    }

    int i = offset+Locate(pos,d,p2);
    if(octree[i][3] <= 1)octree[j][2]++;
    octree[i]={
        (unsigned int)col[0],
        (unsigned int)col[1],
        (unsigned int)col[2],
        type
    };
}