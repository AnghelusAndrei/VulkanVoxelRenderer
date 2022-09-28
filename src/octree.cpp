#include "octree.hpp"

Octree::Octree(uint32_t depth) : depth_(depth)
{
    uint32_t p2r = 1;
    for (int i = depth_; i >= 1; i++)
    {
        utils_p2r[i] = p2r;
        p2r <<= 1;
    }
    // data_.push_back((Leaf)0);
}

size_t Octree::capacity()
{
    return data_.capacity();
}

size_t Octree::size()
{
    return data_.size() * 4;
}
void Octree::insert(glm::uvec3 position, Leaf data)
{
    std::tuple<Octree::Leaf, uint32_t, int> last_node = utils_lookup(position);
    
}
Octree::~Octree()
{
}

std::tuple<Octree::Leaf, uint32_t, int> Octree::utils_lookup(glm::uvec3 position)
{
    uint32_t offset = 0;
    for (int depth = 1; depth <= depth_; depth++)
    {
        Leaf leaf = data_[(offset += utils_locate(position, depth))];
        if (leaf.isLeaf)
            return std::make_tuple(leaf, offset, depth);
        offset = leaf.data;
    }
}
int Octree::utils_locate(glm::uvec3 position, uint32_t depth)
{
    return ((position.x & utils_p2r[depth]) << 2) | ((position.y & utils_p2r[depth]) << 1) | (position.z & utils_p2r[depth]);
}