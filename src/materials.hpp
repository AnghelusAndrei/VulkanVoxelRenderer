#ifndef MATERIALS_HPP
#define MATERIALS_HPP

#include "vulkaninstance.hpp"

class MaterialCollection{
public:
    MaterialCollection();
    ~MaterialCollection();

    struct Material{
        //data

        //address in collection
        uint32_t id;
    };

    void add(Material *m);//returns id

public:
    std::vector<Material> collection;
private:

};

#endif