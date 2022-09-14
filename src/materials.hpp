#ifndef MATERIALS_HPP
#define MATERIALS_HPP

#include "vulkaninstance.hpp"

class MaterialCollection{
    public:
        MaterialCollection();
        ~MaterialCollection();

        struct Material{

        };

        int add(const Material &m);//returns id

    public:
        std::vector<Material> collection;
    private:

};

#endif