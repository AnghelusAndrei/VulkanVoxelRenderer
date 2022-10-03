#include "octree.hpp"

//#define DEBUG

Octree::Octree(uint32_t depth) : depth_(depth)
{
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
    capacity_ = 65536;
    nodes_ = new Node[capacity_];
    LOGGING->info() << "Allocated nodes" << '\n';
}

void Octree::upload(VulkanInstance *instance)
{
    instance->uploadMutex_.lock();
    instance->upload_ = true;
#ifdef DEBUG
    for(int i=0;i<newNode;i++)
    {
        if(!nodes_[i].isNode)
        {
            LOGGING->print(VERBOSE) << std::hex << nodes_[i].leaf.data << ' ';

        }else LOGGING->print(VERBOSE) << nodes_[i].node.next << ' ';
        if((i-1)%8==0&&i>1) LOGGING->print(VERBOSE) << '\n';
    }
#endif
    memcpy(instance->stagingBuffer_.allocationInfo.pMappedData, nodes_, capacity_);
    instance->uploadMutex_.unlock();
}
uint32_t Octree::utils_locate(glm::uvec3 position, uint32_t depth)
{
#ifdef DEBUG
    LOGGING->verbose() << "AND:  " << (position.x & utils_p2r[depth]) << (position.y & utils_p2r[depth]) << (position.z & utils_p2r[depth])<<'\n';
#endif
    return (((bool)(position.x & utils_p2r[depth])) << 2) | ((bool)((position.y & utils_p2r[depth])) << 1) |((bool)(position.z & utils_p2r[depth]));
}
bool Octree::areContained(glm::uvec3 position1, glm::uvec3 position2, uint32_t depth){
    return ((position1.x / utils_p2r[depth] == position2.x / utils_p2r[depth]) && (position1.y / utils_p2r[depth] == position2.y / utils_p2r[depth]) && (position1.z / utils_p2r[depth] == position2.z / utils_p2r[depth]));
}
uint32_t Octree::lookup(glm::uvec3 position)
{
    uint32_t offset = 0;
    for (int depth = 1; depth <= depth_; depth++)
    {
        offset += utils_locate(position, depth);
        Node leaf = nodes_[offset];
        if (!leaf.isNode)
            return offset;
        offset = leaf.node.next;
    }
}
void Octree::insert(glm::uvec3 position, Node data)
{
    uint32_t offset = 0;
    uint32_t lastNode = 0;
    data.isNode=false;
    for (int depth = 1; depth < depth_; depth++)
    {
        offset += utils_locate(position, depth);
#ifdef DEBUG
        LOGGING->verbose() << "Offset: " << offset <<'\n'; 
#endif
        Node leaf = nodes_[offset];
        if(!leaf.isNode){
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
            node.node.next = nextOffset;
            node.node.count = 0;
            nodes_[offset] = node;
            if(depth>1)nodes_[lastNode].node.count++;
            lastNode = offset;
            offset = nextOffset;
        }else{
            offset = leaf.node.next;
        }
    }

    int i = offset + utils_locate(position, depth_);
    if(nodes_[i].leaf.type == 0)nodes_[lastNode].node.count++;
    nodes_[i] = data;
}
void Octree::insert(glm::uvec3 position, Node data, std::function<bool(uint8_t)> func)
{
    uint32_t offset = 0;
    uint32_t lastNode = 0;
    data.isNode=false;
    for (int depth = 1; depth < depth_; depth++)
    {
        offset += utils_locate(position, depth);
#ifdef DEBUG
        LOGGING->verbose() << "Offset: " << offset <<'\n'; 
#endif
        Node leaf = nodes_[offset];
        if(!leaf.isNode){
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
            node.node.next = nextOffset;
            node.node.count = 0;
            nodes_[offset] = node;
            if(depth>1)nodes_[lastNode].node.count++;
            lastNode = offset;
            offset = nextOffset;
        }else{
            offset = leaf.node.next;
        }
    }

    int i = offset + utils_locate(position, depth_);
    if(func(nodes_[i].leaf.type)){
        if(nodes_[i].leaf.type == 0)
            nodes_[lastNode].node.count++;
        nodes_[i] = data;
    }
}
void Octree::remove(glm::uvec3 position)
{
    uint32_t offset = 0;
    std::stack<uint32_t> localNodes;
    
    for (int depth = 1; depth < depth_; depth++)
    {
        int i = offset + utils_locate(position, depth);
        Node leaf = nodes_[i];
        if(!leaf.isNode){
            return;
        }else{
            localNodes.push(offset);
            offset = leaf.node.next;
        }
    }

    int i = offset + utils_locate(position, depth_);
    Node removed = {
        .isNode = 0,
        .leaf = {
            0,0
        }
    };
    nodes_[i] = removed;
    nodes_[localNodes.top()].node.count--;

    while(nodes_[localNodes.top()].node.count <= 0 && !localNodes.empty()){
        freeNodes.push(nodes_[localNodes.top()].node.next);
        nodes_[localNodes.top()] = removed;
        localNodes.pop();
        nodes_[localNodes.top()].node.count--;
    }
}
void Octree::remove(glm::uvec3 position, std::function<bool(uint8_t)> func)
{
    uint32_t offset = 0;
    std::stack<uint32_t> localNodes;
    
    for (int depth = 1; depth < depth_; depth++)
    {
        int i = offset + utils_locate(position, depth);
        Node leaf = nodes_[i];
        if(!leaf.isNode){
            return;
        }else{
            localNodes.push(offset);
            offset = leaf.node.next;
        }
    }

    int i = offset + utils_locate(position, depth_);
    Node removed = {
        .isNode = 0,
        .leaf = {
            0,0
        }
    };
    if(func(nodes_[i].leaf.type)){
        nodes_[i] = removed;
        nodes_[localNodes.top()].node.count--;

        while(nodes_[localNodes.top()].node.count <= 0 && !localNodes.empty()){
            freeNodes.push(nodes_[localNodes.top()].node.next);
            nodes_[localNodes.top()] = removed;
            localNodes.pop();
            nodes_[localNodes.top()].node.count--;
        }
    }
}
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

    while(!areContained(position1, position2, depth)){
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
    while(!areContained(position1, position2, depth)){
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
