#include "octree.hpp"



Octree::Octree(uint32_t depth) : depth_(depth)
{
    assert(sizeof(Node)==4);
    uint32_t p2r = 1;
    for (int i = depth_; i >= 1; i--)
    {
        utils_p2r[i] = p2r;
        p2r <<= 1;
    }
    size_t p8 = 8;
    for (int i = 0; i < depth_; i++)
    {
        capacity_ += p8;
        p8 *= 8;
    }
    LOGGING->info() << "Allocating nodes "<< capacity_ << '\n';
    nodes_ =(Node*)calloc(capacity_, sizeof(Node));
    LOGGING->info() << "Allocated nodes" << '\n';
}
#define DEBUG
void Octree::upload(VulkanInstance *instance)
{
    instance->uploadMutex_.lock();
    instance->upload_ = true;

    memcpy(instance->stagingBuffer_.allocationInfo.pMappedData, nodes_, capacity_*sizeof(Node));
    instance->uploadMutex_.unlock();
}
uint32_t Octree::utils_locate(glm::uvec3 position, uint32_t depth)
{
    return (((bool)(position.x & utils_p2r[depth])) << 2) | ((bool)((position.y & utils_p2r[depth])) << 1) |((bool)(position.z & utils_p2r[depth]));
}

bool Octree::contained(glm::uvec3 position1, glm::uvec3 position2, uint32_t depth){
    return ((position1.x / utils_p2r[depth] == position2.x / utils_p2r[depth]) && (position1.y / utils_p2r[depth] == position2.y / utils_p2r[depth]) && (position1.z / utils_p2r[depth] == position2.z / utils_p2r[depth]));
}
uint32_t Octree::lookup(glm::uvec3 position)
{
    uint32_t offset = 0;
    for (int depth = 1; depth <= depth_; depth++)
    {
        offset += utils_locate(position, depth);
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
    for (int depth = 1; depth < depth_; depth++)
    {
        offset += utils_locate(position, depth);
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
            if(depth>1)nodes_[lastNode].count++;
            lastNode = offset;
            offset = nextOffset;
        }else{
            offset = node.next;
        }
    }

    int i = offset + utils_locate(position, depth_);
    if((reinterpret_cast<Leaf*>(&nodes_[i]))->type == 0)nodes_[lastNode].count++;
    nodes_[i] = *reinterpret_cast<Node*>(&data);
}

void Octree::insert(glm::uvec3 position, Leaf data, std::function<bool(uint8_t)> func)
{
    uint32_t offset = 0;
    uint32_t lastNode = 0;
    data.isNode=false;
    for (int depth = 1; depth < depth_; depth++)
    {
        offset += utils_locate(position, depth);
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
            if(depth>1)nodes_[lastNode].count++;
            lastNode = offset;
            offset = nextOffset;
        }else{
            offset = node.next;
        }
    }

    int i = offset + utils_locate(position, depth_);
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
    
    for (int depth = 1; depth < depth_; depth++)
    {
        int i = offset + utils_locate(position, depth);
        Node node = nodes_[i];
        if(!node.isNode){
            return;
        }else{
            localNodes.push(offset);
            offset = node.next;
        }
    }

    int i = offset + utils_locate(position, depth_);
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
    
    for (int depth = 1; depth < depth_; depth++)
    {
        int i = offset + utils_locate(position, depth);
        Node node = nodes_[i];
        if(!node.isNode){
            return;
        }else{
            localNodes.push(offset);
            offset = node.next;
        }
    }

    int i = offset + utils_locate(position, depth_);
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
/*
void Octree::move(glm::uvec3 position1, glm::uvec3 position2){
    uint32_t offset = 0;
    std::stack<uint32_t> localNodes;
    int depth = 1;
    
    for (depth; depth < depth_; depth++)
    {
        int i = offset + utils_locate(position1, depth);
        Node leaf = nodes_[i];
        if(!leaf.isNode){
            return;
        }else{
            localNodes.push(offset);
            offset = leaf.node.next;
        }
    }

    int i = offset + utils_locate(position1, depth_);
    Node removed = {
        .isNode = 0,
        .leaf = {
            0,0
        }
    };
    Node data = nodes_[i];
    if(!nodes_[i].leaf.type == 0){
        nodes_[i] = removed;
        nodes_[localNodes.top()].node.count--;
    }

    while(nodes_[localNodes.top()].node.count <= 0 && !localNodes.empty()){
        freeNodes.push(nodes_[localNodes.top()].node.next);
        nodes_[localNodes.top()] = removed;
        localNodes.pop();
        nodes_[localNodes.top()].node.count--;
        depth--;
    } 

    while(!contained(position1, position2, depth)){
        localNodes.pop();
        depth--;
    }
    offset = nodes_[localNodes.top()].node.next;

    for (depth; depth < depth_; depth++)
    {
        i = offset + utils_locate(position2, depth);
        Node leaf = nodes_[i];
        if(!leaf.isNode){
            return;
        }else{
            localNodes.push(offset);
            offset = leaf.node.next;
        }
    }

    i = offset + utils_locate(position2, depth_);
    if(nodes_[i].leaf.type == 0)nodes_[localNodes.top()].node.count++;
    nodes_[i] = data;
}
void Octree::move(glm::uvec3 position1, glm::uvec3 position2, std::function<int(uint8_t, uint8_t)> func){
    uint32_t offset = 0;
    std::stack<uint32_t> localNodes;
    int depth = 1, i;
    
    for (depth; depth < depth_; depth++)
    {
        i = offset + utils_locate(position1, depth);
        Node leaf = nodes_[i];
        if(!leaf.isNode){
            return;
        }else{
            localNodes.push(offset);
            offset = leaf.node.next;
        }
    }

    Node removed = {
        .isNode = 0,
        .leaf = {
            0,0
        }
    };
    uint32_t v1_id = offset + utils_locate(position1, depth_);

    //search for position2
    while(!contained(position1, position2, depth)){
        localNodes.pop();
        depth--;
    }
    offset = nodes_[localNodes.top()].node.next;
    uint32_t lastNode;

    for (depth; depth < depth_; depth++)
    {
        i = offset + utils_locate(position2, depth);
        Node leaf = nodes_[i];
        if(!leaf.isNode){
            return;
        }else{
            lastNode = offset;
            offset = leaf.node.next;
        }
    }

    uint32_t v2_id = offset + utils_locate(position2, depth_);

    switch (func(nodes_[v1_id].leaf.type, nodes_[v2_id].leaf.type))
    {
    case 0:
        return;
    case 1:
        offset = nodes_[localNodes.top()].node.next;
        for (depth; depth < depth_; depth++)
        {
            i = offset + utils_locate(position1, depth);
            Node leaf = nodes_[i];
            if(!leaf.isNode){
                return;
            }else{
                localNodes.push(offset);
                offset = leaf.node.next;
            }
        }
        i = v1_id;

        if(!nodes_[i].leaf.type == 0){
            nodes_[i] = removed;
            nodes_[localNodes.top()].node.count--;
        }

        while(nodes_[localNodes.top()].node.count <= 0 && !localNodes.empty()){
            freeNodes.push(nodes_[localNodes.top()].node.next);
            nodes_[localNodes.top()] = removed;
            localNodes.pop();
            nodes_[localNodes.top()].node.count--;
            depth--;
        } 
        break;
    case 2:
        if(nodes_[v2_id].leaf.type == 0)nodes_[lastNode].node.count++;
        nodes_[v2_id] = nodes_[v1_id];

        offset = nodes_[localNodes.top()].node.next;
        for (depth; depth < depth_; depth++)
        {
            i = offset + utils_locate(position1, depth);
            Node leaf = nodes_[i];
            if(!leaf.isNode){
                return;
            }else{
                localNodes.push(offset);
                offset = leaf.node.next;
            }
        }
        i = v1_id;

        if(!nodes_[i].leaf.type == 0){
            nodes_[i] = removed;
            nodes_[localNodes.top()].node.count--;
        }

        while(nodes_[localNodes.top()].node.count <= 0 && !localNodes.empty()){
            freeNodes.push(nodes_[localNodes.top()].node.next);
            nodes_[localNodes.top()] = removed;
            localNodes.pop();
            nodes_[localNodes.top()].node.count--;
            depth--;
        } 
        break;
    }
}
*/