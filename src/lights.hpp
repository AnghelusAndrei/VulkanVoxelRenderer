#ifndef LIGHTS_HPP
#define LIGHTS_HPP

#include "vulkaninstance.hpp"

class LightCollection{
public:
    LightCollection();
    ~LightCollection();

    struct Light{
        //data

        //address in collection
        uint32_t id;
    };

    void add(Light *m);//returns id

public:
    std::vector<Light*> collection;
private:

};

#endif