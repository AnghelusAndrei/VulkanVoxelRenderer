#include "object.hpp"

Object::Object(Octree *octree, glm::uvec3 position_, glm::uvec3 rotation_, glm::uvec3 scale_, uint32_t object_id) : octree_(octree), position(position_), rotation(rotation_), scale(scale_), id(object_id){

}

bool Object::loadWavefrontObj(std::string filename, bool hasTexture, bool hasNormal){

}

void Object::update(){
    for(int i = 0; i<voxels_[1].size(); i++){
        octree_->Move(voxels_[0][i], voxels_[1][i]);
        voxels_[0][i]=voxels_[1][i];
        voxels_data_[0][i]=voxels_data_[1][i];
    }
    octree_->upToDate = false;
}
void Object::remove(){
    for(int i = 0; i<voxels_[1].size(); i++){
        octree_->Remove(voxels_[0][i]);
    } 
}

void Object::setPosition(glm::uvec3 p){}
void Object::setRotation(glm::vec3 r){}
void Object::setScale(glm::vec3 s){}