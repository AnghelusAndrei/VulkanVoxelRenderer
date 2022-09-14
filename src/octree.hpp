#ifndef OCTREE_HPP
#define OCTREE_HPP

#include "vulkaninstance.hpp"


#define maxDepth 15

class Octree{
    public:
        struct leaf{
            glm::uvec4 data;
            uint32_t size;
            uint32_t id;
        };
        Octree();
        void Setup(int depth);

        void Insert(glm::uvec3 pos, glm::uvec4 data, uint32_t object_id);
        void Insert(glm::uvec3 pos, glm::uvec4 data, uint32_t object_id, bool (*object_idCondition)(uint32_t));
        void Remove(glm::uvec3 pos);
        void Remove(glm::uvec3 pos, bool (*object_idCondition)(uint32_t));
        void Move(glm::uvec3 from, glm::uvec3 to);
        void Move(glm::uvec3 from, glm::uvec3 to, bool (*object_idCondition)(uint32_t uint32_t));
        Octree::leaf Lookup(glm::uvec3 pos);

        void upload();
    public:
        enum Enums{
            LEAF  = 0,
            NODE  = 1,
        };
        
        uint64_t OCTREE_INDEX = 8;
        glm::uvec4 * octree;

        uint32_t depth_;
        uint32_t n;
        bool upToDate = true;

    private:
        std::stack<int> free_mem;
        uint64_t OCTREE_MAX_LENGTH = 0;

        int Locate(glm::uvec3 pos, uint32_t depth){
            int p2 = 1<<(depth-1);
            int a=n/p2;
            int b=n/(p2*2);

            return (int)((pos.x%a)/b + 
                2*((pos.y%a)/b)+
                4*((pos.z%a)/b));
        }

        bool inNode(glm::uvec3 targetPos, glm::uvec3 nodePos, uint32_t depth){
            int p2 = 1<<(depth-1);
            int a=n/p2;
            int b=n/(p2*2);

            if(
                targetPos.x < (nodePos.x/a)*a || targetPos.x > (nodePos.x/a+1)*a ||
                targetPos.y < (nodePos.y/a)*a || targetPos.y > (nodePos.y/a+1)*a ||
                targetPos.z < (nodePos.z/a)*a || targetPos.z > (nodePos.z/a+1)*a
            ) return false;
            return true;
        }

};

#endif