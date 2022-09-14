#ifndef LIGHTS_HPP
#define LIGHTS_HPP

#include "vulkaninstance.hpp"

class LightCollection{
    public:
        LightCollection();
        ~LightCollection();

        struct Light{

        };

        int add(const Light &m);//returns id

    public:
        std::vector<Light> collection;
    private:

};

#endif