#ifndef OBJECTCOLLECTION_HPP
#define OBJECTCOLLECTION_HPP

#include "vulkaninstance.hpp"

class Object;
class ObjectCollection{
public:
    ObjectCollection();
    ~ObjectCollection();

    void add(Object *m, uint32_t materialId);
    Object *get(uint32_t objectId);

public:
    std::vector<Object*> collection;
    std::vector<uint32_t> mIdCollection;

private:
};
#endif