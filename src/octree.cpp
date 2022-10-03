#include "octree.hpp"

Octree::Octree(uint32_t depth) : depth_(depth)
{
    uint32_t p2r = 1;
    for (int i = depth_; i >= 1; i--)
    {
        utils_p2r[i] = p2r;
        p2r <<= 1;
    }
    int p8 = 8;
    for (int i = 0; i < depth_; i++)
    {
        capacity_ += p8;
        p8 *= 8;
    }
    capacity_=65536;
    LOGGING->info() << "Allocating nodes "<< capacity_ << '\n';
    nodes_ = new Node[capacity_];
    LOGGING->info() << "Allocated nodes" << '\n';
}

void Octree::upload(VulkanInstance *instance)
{
    instance->uploadMutex_.lock();
    instance->upload_ = true;
    for(int i=0;i<256;i++)
    {
        if(!nodes_[i].isNode)
        {
            LOGGING->print(VERBOSE) << std::hex << nodes_[i].leaf.data << ' ';

        }else LOGGING->print(VERBOSE) << nodes_[i].node.next << ' ';
        if((i-1)%8==0&&i>1) LOGGING->print(VERBOSE) << '\n';
    }
    memcpy(instance->stagingBuffer_.allocationInfo.pMappedData, nodes_, capacity_);
    instance->uploadMutex_.unlock();
}
uint32_t Octree::utils_locate(glm::uvec3 position, uint32_t depth)
{
    LOGGING->verbose() << "AND:  " << (position.x & utils_p2r[depth]) << (position.y & utils_p2r[depth]) << (position.z & utils_p2r[depth])<<'\n';
    return (((bool)(position.x & utils_p2r[depth])) << 2) | ((bool)((position.y & utils_p2r[depth])) << 1) |((bool)(position.z & utils_p2r[depth]));
}
uint32_t Octree::utils_lookup(glm::uvec3 position)
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
        LOGGING->verbose() << "Offset: " << offset <<'\n'; 
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
    nodes_[i] = data;
    nodes_[lastNode].node.count++;
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