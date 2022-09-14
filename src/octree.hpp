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
        void Setup(int depth_);

        void Insert(glm::uvec3 pos, glm::uvec3 data, uint32_t object_id);
        void Insert(glm::uvec3 pos, glm::uvec3 data, uint32_t object_id, bool (*object_idCondition)(uint32_t));
        void Remove(glm::uvec3 pos);
        void Remove(glm::uvec3 pos, bool (*object_idCondition)(uint32_t));
        void Move(glm::uvec3 from, glm::uvec3 to);
        void Move(glm::uvec3 from, glm::uvec3 to, glm::uvec3 data, uint32_t object_id);
        void Move(glm::uvec3 from, glm::uvec3 to, bool (*object_idCondition)(uint32_t, uint32_t));
        void Move(glm::uvec3 from, glm::uvec3 to, glm::uvec3 data, uint32_t object_id, bool (*object_idCondition)(uint32_t, uint32_t));
        Octree::leaf Lookup(glm::uvec3 pos);

        void upload();
    public:
        glm::vec3 GetVNormal(glm::uvec3 data);
        glm::uvec3 SetVNormal(glm::vec3 data);
    public:
        enum Enums{
            LEAF  = 0,
            NODE  = 1,
        };
        
        uint64_t octreeIndex = 8;
        glm::uvec4 *octree;

        uint32_t depth;
        uint32_t n;
        bool upToDate = true;

    private:
        std::stack<int> freeMem_p;
        uint64_t octreeMaxLength_p = 0;

        int Locate(glm::uvec3 pos_, uint32_t depth_){
            int p2 = 1<<(depth-1);
            int a=n/p2;
            int b=n/(p2*2);

            return (int)((pos_.x%a)/b + 
                2*((pos_.y%a)/b)+
                4*((pos_.z%a)/b));
        }

        bool inNode(glm::uvec3 targetPos_, glm::uvec3 nodePos_, uint32_t depth_){
            int p2 = 1<<(depth-1);
            int a=n/p2;
            int b=n/(p2*2);

            if(
                targetPos_.x < (nodePos_.x/a)*a || targetPos_.x > (nodePos_.x/a+1)*a ||
                targetPos_.y < (nodePos_.y/a)*a || targetPos_.y > (nodePos_.y/a+1)*a ||
                targetPos_.z < (nodePos_.z/a)*a || targetPos_.z > (nodePos_.z/a+1)*a
            ) return false;
            return true;
        }

};

#endif