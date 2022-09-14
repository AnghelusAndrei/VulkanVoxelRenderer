#include "objectCollection.hpp"

ObjectCollection::ObjectCollection(){

}

ObjectCollection::~ObjectCollection(){
    
}

void ObjectCollection::add(Object *m, uint32_t materialId){
    m->id = collection.size();
    collection.push_back(*m);
    mIdCollection.push_back(materialId);
}