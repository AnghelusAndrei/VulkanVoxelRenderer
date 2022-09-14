#include "object.hpp"

Object::Object(Octree *octree, glm::uvec3 position_, glm::uvec3 rotation_, glm::uvec3 scale_) : octree_p(octree), position(position_), rotation(rotation_), scale(scale_){

}

Object::~Object(){
    remove();
}

bool Object::loadWavefrontObj(std::string filename, bool hasTexture, bool hasNormal){

}

void Object::update(){
    for(int i = 0; i<voxels_p[1].size(); i++){
        octree_p->Move(voxels_p[0][i], voxels_p[1][i], voxels_data_p[1][i], id);
        voxels_p[0][i]=voxels_p[1][i];
        voxels_data_p[0][i]=voxels_data_p[1][i];
    }
    octree_p->upToDate = false;
}
void Object::remove(){
    for(int i=voxels_p[1].size();i>0;i--){
        octree_p->Remove(voxels_p[1][i]);
        voxels_p[1].pop_back();
        voxels_data_p[1].pop_back();
        voxels_p[0].pop_back();
        voxels_data_p[0].pop_back();
    }
}

void Object::setPosition(glm::uvec3 p){
    for(glm::uvec3 & voxel : voxels_p[1]){
        voxel-=position;
        voxel+=p;
    }
}
void Object::setRotation(glm::vec3 r){
    for(int i = 0; i<voxels_p[1].size(); i++){
        glm::vec3 normal = octree_p->GetVNormal(voxels_data_p[1][i]);
    }
}