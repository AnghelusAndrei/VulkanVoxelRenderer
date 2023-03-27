#include "octree.hpp"

#include "logging.hpp"

Octree::Octree(uint8_t depth_) : depth(depth_)
{
    assert(sizeof(Node)==4);
    uint32_t p2r = 1;
    for (int i = depth; i >= 1; i--)
    {
        utils_p2r[i] = p2r;
        p2r <<= 1;
    }
    size_t p8 = 8;
    length = 1<<depth;
    for (int i = 0; i < depth_; i++)
    {
        capacity += p8;
        p8 *= 8;
    }
    LOGGING->info() << "Allocating nodes "<< capacity << '\n';
    nodes_ =(Node*)calloc(capacity, sizeof(Node));
    LOGGING->info() << "Allocated nodes" << '\n';
}
#define DEBUG
void Octree::upload(VulkanInstance *instance)
{
    instance->uploadMutex_.lock();
    instance->upload_ = true;

    LOGGING->verbose()<<"Uploading octree to buffer memory"<<'\n';

    memcpy(instance->stagingBuffer_.allocationInfo.pMappedData, nodes_, capacity*sizeof(Node));
    instance->uploadMutex_.unlock();
}
uint32_t Octree::utils_locate(glm::uvec3 position, uint32_t depth_)
{
    return (((bool)(position.x & utils_p2r[depth_])) << 2) | ((bool)((position.y & utils_p2r[depth_])) << 1) |((bool)(position.z & utils_p2r[depth_]));
}

bool Octree::contained(glm::uvec3 position1, glm::uvec3 position2, uint32_t depth_){
    return ((position1.x / utils_p2r[depth_] == position2.x / utils_p2r[depth_]) && (position1.y / utils_p2r[depth_] == position2.y / utils_p2r[depth_]) && (position1.z / utils_p2r[depth_] == position2.z / utils_p2r[depth_]));
}
uint32_t Octree::lookup(glm::uvec3 position)
{
    uint32_t offset = 0;
    for (int depth_ = 1; depth_ <= depth; depth_++)
    {
        offset += utils_locate(position, depth_);
        Node node = nodes_[offset];
        if (!node.isNode)
            return offset;
        offset = node.next;
    }
}
void Octree::insert(glm::uvec3 position, Leaf data)
{
    uint32_t offset = 0;
    uint32_t lastNode = 0;
    data.isNode=false;
    for (int depth_ = 1; depth_ < depth; depth_++)
    {
        offset += utils_locate(position, depth_);
        Node node = nodes_[offset];

        if(!node.isNode){
            uint32_t nextOffset;
            if(freeNodes.empty()){
                newNode+=8;
                nextOffset = newNode;
            }else{
                nextOffset = freeNodes.top();
                freeNodes.pop();
            }
            Node node;
            node.isNode = 1;
            node.next = nextOffset;
            node.count = 0;
            nodes_[offset] = node;
            if(depth_>1)nodes_[lastNode].count++;
            lastNode = offset;
            offset = nextOffset;
        }else{
            offset = node.next;
        }
    }

    int i = offset + utils_locate(position, depth);
    if((reinterpret_cast<Leaf*>(&nodes_[i]))->type == 0)nodes_[lastNode].count++;
    nodes_[i] = *reinterpret_cast<Node*>(&data);
}

void Octree::insert(glm::uvec3 position, Leaf data, std::function<bool(uint8_t)> func)
{
    uint32_t offset = 0;
    uint32_t lastNode = 0;
    data.isNode=false;
    for (int depth_ = 1; depth_ < depth; depth_++)
    {
        offset += utils_locate(position, depth_);
        Node node = nodes_[offset];
        if(!node.isNode){
            uint32_t nextOffset;
            if(freeNodes.empty()){
                newNode+=8;
                nextOffset = newNode;
            }else{
                nextOffset = freeNodes.top();
                freeNodes.pop();
            }
            Node node;
            node.isNode = 1;
            node.next = nextOffset;
            node.count = 0;
            nodes_[offset] = node;
            if(depth_>1)nodes_[lastNode].count++;
            lastNode = offset;
            offset = nextOffset;
        }else{
            offset = node.next;
        }
    }

    int i = offset + utils_locate(position, depth);
    if(func((reinterpret_cast<Leaf*>(&nodes_[i]))->type)){
        if((reinterpret_cast<Leaf*>(&nodes_[i]))->type == 0)
            nodes_[lastNode].count++;
        nodes_[i] = *reinterpret_cast<Node*>(&data);
    }
}

void Octree::remove(glm::uvec3 position)
{
    uint32_t offset = 0;
    std::stack<uint32_t> localNodes;
    
    for (int depth_ = 1; depth_ < depth; depth_++)
    {
        int i = offset + utils_locate(position, depth_);
        Node node = nodes_[i];
        if(!node.isNode){
            return;
        }else{
            localNodes.push(offset);
            offset = node.next;
        }
    }

    int i = offset + utils_locate(position, depth);
    Node removed = {
        .isNode = 0,
        .count = 0,
        .next = 0
    };
    nodes_[i] = removed;
    nodes_[localNodes.top()].count--;

    while(nodes_[localNodes.top()].count <= 0 && !localNodes.empty()){
        freeNodes.push(nodes_[localNodes.top()].next);
        nodes_[localNodes.top()] = removed;
        localNodes.pop();
        nodes_[localNodes.top()].count--;
    }
}
void Octree::remove(glm::uvec3 position, std::function<bool(uint8_t)> func)
{
    uint32_t offset = 0;
    std::stack<uint32_t> localNodes;
    
    for (int depth_ = 1; depth_ < depth; depth_++)
    {
        int i = offset + utils_locate(position, depth_);
        Node node = nodes_[i];
        if(!node.isNode){
            return;
        }else{
            localNodes.push(offset);
            offset = node.next;
        }
    }

    int i = offset + utils_locate(position, depth);
    Node removed = {
        .isNode = 0,
        .count = 0,
        .next = 0
    };
    if(func((reinterpret_cast<Leaf*>(&nodes_[i]))->type)){
        nodes_[i] = removed;
        nodes_[localNodes.top()].count--;

        while(nodes_[localNodes.top()].count <= 0 && !localNodes.empty()){
            freeNodes.push(nodes_[localNodes.top()].next);
            nodes_[localNodes.top()] = removed;
            localNodes.pop();
            nodes_[localNodes.top()].count--;
        }
    }
}

void Octree::move(glm::uvec3 position1, glm::uvec3 position2){
    uint32_t offset = 0;
    std::stack<uint32_t> localNodes;
    int depth_ = 1;
    
    for (depth_; depth_ < depth; depth_++)
    {
        int i = offset + utils_locate(position1, depth_);
        Node leaf = nodes_[i];
        if(!leaf.isNode){
            return;
        }else{
            localNodes.push(offset);
            offset = node.next;
        }
    }

    int i = offset + utils_locate(position1, depth);
    Node removed = {
        .isNode = 0,
        .count = 0,
        .next = 0
    };
    Node data = nodes_[i];
    if(!(reinterpret_cast<Leaf*>(&nodes_[i]))->type == 0){
        nodes_[i] = removed;
        nodes_[localNodes.top()].count--;
    }

    while(nodes_[localNodes.top()].count <= 0 && !localNodes.empty()){
        freeNodes.push(nodes_[localNodes.top()].next);
        nodes_[localNodes.top()] = removed;
        localNodes.pop();
        nodes_[localNodes.top()].node.count--;
        depth_--;
    } 

    while(!contained(position1, position2, depth_)){
        localNodes.pop();
        depth_--;
    }
    offset = nodes_[localNodes.top()].next;

    for (depth_; depth_ < depth; depth_++)
    {
        i = offset + utils_locate(position2, depth_);
        Node leaf = nodes_[i];
        if(!leaf.isNode){
            return;
        }else{
            localNodes.push(offset);
            offset = node.next;
        }
    }

    i = offset + utils_locate(position2, depth);
    if(nodes_[i].leaf.type == 0)nodes_[localNodes.top()].node.count++;
    nodes_[i] = data;
}
void Octree::move(glm::uvec3 position1, glm::uvec3 position2, std::function<int(uint8_t, uint8_t)> func){
    uint32_t offset = 0;
    std::stack<uint32_t> localNodes;
    int depth_ = 1, i;
    
    for (depth_; depth_ < depth; depth_++)
    {
        i = offset + utils_locate(position1, depth_);
        Node leaf = nodes_[i];
        if(!leaf.isNode){
            return;
        }else{
            localNodes.push(offset);
            offset = node.next;
        }
    }

    Node removed = {
        .isNode = 0,
        .count = 0,
        .next = 0
    };
    uint32_t v1_id = offset + utils_locate(position1, depth);

    //search for position2
    while(!contained(position1, position2, depth_)){
        localNodes.pop();
        depth_--;
    }
    offset = nodes_[localNodes.top()].next;
    uint32_t lastNode;

    for (depth_; depth_ < depth; depth_++)
    {
        i = offset + utils_locate(position2, depth_);
        Node leaf = nodes_[i];
        if(!leaf.isNode){
            return;
        }else{
            lastNode = offset;
            offset = node.next;
        }
    }

    uint32_t v2_id = offset + utils_locate(position2, depth);

    switch (func((reinterpret_cast<Leaf*>(&nodes_[v1_id]))->type, (reinterpret_cast<Leaf*>(&nodes_[v2_id]))->type))
    {
    case 0:
        return;
    case 1:
        offset = nodes_[localNodes.top()].node.next;
        for (depth_; depth_ < depth; depth_++)
        {
            i = offset + utils_locate(position1, depth_);
            Node leaf = nodes_[i];
            if(!leaf.isNode){
                return;
            }else{
                localNodes.push(offset);
                offset = leaf.next;
            }
        }
        i = v1_id;

        if(!(reinterpret_cast<Leaf*>(&nodes_[i]))->type == 0){
            nodes_[i] = removed;
            nodes_[localNodes.top()].count--;
        }

        while(nodes_[localNodes.top()].count <= 0 && !localNodes.empty()){
            freeNodes.push(nodes_[localNodes.top()].next);
            nodes_[localNodes.top()] = removed;
            localNodes.pop();
            nodes_[localNodes.top()].node.count--;
            depth_--;
        } 
        break;
    case 2:
        if((reinterpret_cast<Leaf*>(&nodes_[v2_id]))->type == 0)nodes_[lastNode].count++;
        nodes_[v2_id] = nodes_[v1_id];

        offset = nodes_[localNodes.top()].node.next;
        for (depth_; depth_ < depth; depth_++)
        {
            i = offset + utils_locate(position1, depth_);
            Node leaf = nodes_[i];
            if(!leaf.isNode){
                return;
            }else{
                localNodes.push(offset);
                offset = node.next;
            }
        }
        i = v1_id;

        if(!(reinterpret_cast<Leaf*>(&nodes_[i]))->type == 0){
            nodes_[i] = removed;
            nodes_[localNodes.top()].count--;
        }

        while(nodes_[localNodes.top()].count <= 0 && !localNodes.empty()){
            freeNodes.push(nodes_[localNodes.top()].next);
            nodes_[localNodes.top()] = removed;
            localNodes.pop();
            nodes_[localNodes.top()].node.count--;
            depth_--;
        } 
        break;
    }
}