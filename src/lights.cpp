#include "lights.hpp"

LightCollection::LightCollection(){

}

LightCollection::~LightCollection(){

}

void LightCollection::add(Light *m){
    m->id = collection.size();
    collection.push_back(*m);
}