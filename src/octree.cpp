#include "octree.hpp"


Octree::Octree(int depth){
    if(depth > maxDepth){
        LOGGING->error() << "Octree depth cannot be higher than " << maxDepth <<std::endl;
    }

    depth_ = depth;
    n = 1<<depth;
    int o_m = 8;

    for(int i = 0;i<depth;i++){
        OCTREE_MAX_LENGTH += o_m;
        o_m *= 8;
    }

    octree = new glm::uvec4[OCTREE_MAX_LENGTH];
} 

void Octree::upload(){
    if(upToDate)return;
    upToDate = false;

    //
    // interesting vkMemoryAllocator stuff happens here later
    //
}

void Octree::Insert(glm::uvec4 pos, glm::uvec4 data, unsigned int type){
    int d=1;
    int offset = 0;
    int p2 = 1;
    int j = 0;

    while(d<depth_){
        int i = offset+Locate(pos,d,p2);

        switch (octree[i].w)
        {
            case LEAF:
                if(free_mem.empty()){
                    offset = OCTREE_INDEX;
                    OCTREE_INDEX+=8;
                }else{
                    offset = free_mem.top();
                    free_mem.pop();
                }
                octree[i].x =offset;
                octree[i].w = NODE;
                octree[i].z = 0;
                if(d>1)octree[j].z++;
                j = i;
                break;
            case NODE:
                j = i;
                offset = octree[i].y;
                break;
        }


        d++;
        p2*=2;
    }

    int i = offset+Locate(pos,d,p2);
    if(octree[i].w <= 1)octree[j].z++;
    octree[i]={
        data.x,
        data.y,
        data.z,
        type
    };
    upToDate = false;
}

void Octree::Insert(glm::uvec4 pos, glm::uvec4 data, unsigned int type, bool (*typeCondition)(unsigned int)){
    int d=1;
    int offset = 0;
    int p2 = 1;
    int j = 0;
    std::stack<int> nodes;

    while(d<depth_){
        int i = offset+Locate(pos,d,p2);

        switch (octree[i].w)
        {
            case LEAF:
                if(free_mem.empty()){
                    offset = OCTREE_INDEX;
                    OCTREE_INDEX+=8;
                }else{
                    offset = free_mem.top();
                    free_mem.pop();
                }
                octree[i].x =offset;
                octree[i].w = NODE;
                octree[i].z = 0;
                if(d>1)octree[j].z++;
                j = i;
                break;
            case NODE:
                j = i;
                offset = octree[i].y;
                break;
        }

        nodes.push(i);

        d++;
        p2*=2;
    }

    int i = offset+Locate(pos,d,p2);
    if(typeCondition(octree[j].w)){
        if(octree[i].w <= 1)octree[j].z++;
        octree[i]={
            data.x,
            data.y,
            data.z,
            type
        };
        upToDate = false;
    }else{
        if(octree[i].w == LEAF){
            while(octree[nodes.top()].z <= 0 && !nodes.empty()){
                free_mem.push(octree[nodes.top()].x);
                octree[nodes.top()] = {0,0,0,0};
                nodes.pop();
                octree[nodes.top()].z--;
            }
        }
    }
}

void Octree::Remove(glm::uvec4 pos){
    int d=1;
    int offset = 0;
    int p2=1;
    std::stack<int> nodes;

    while(d<=depth_){
        int i = offset+Locate(pos,d,p2);

        switch (octree[i].w)
        {
        case LEAF:
            return;
        case NODE:
            nodes.push(i);
            offset = octree[i].x;
            break;
        default:
            octree[i] = {0,0,0,0};
            octree[nodes.top()].z--;

            while(octree[nodes.top()].z <= 0 && !nodes.empty()){
                free_mem.push(octree[nodes.top()].x);
                octree[nodes.top()] = {0,0,0,0};
                nodes.pop();
                octree[nodes.top()].z--;
            }

            upToDate = false;
            return;
        }

        d++;
        p2*=2;
    }

}

void Octree::Remove(glm::uvec4 pos, bool (*typeCondition)(unsigned int)){
    int d=1;
    int offset = 0;
    int p2=1;
    std::stack<int> nodes;

    while(d<=depth_){
        int i = offset+Locate(pos,d,p2);

        switch (octree[i].w)
        {
        case LEAF:
            return;
        case NODE:
            nodes.push(i);
            offset = octree[i].x;
            break;
        default:
            if(!typeCondition(octree[i].w))return;

            octree[i] = {0,0,0,0};
            octree[nodes.top()].z--;

            while(octree[nodes.top()].z <= 0 && !nodes.empty()){
                free_mem.push(octree[nodes.top()].x);
                octree[nodes.top()] = {0,0,0,0};
                nodes.pop();
                octree[nodes.top()].z--;
            }

            upToDate = false;
            return;
        }

        d++;
        p2*=2;
    }

}

leaf Octree::Lookup(glm::uvec4 pos){
    int d=1;
    int offset = 0;
    int p2=1;
    while(d<=depth_){
        int i = offset+Locate(pos,d,p2);
        int s = n/(p2*2);

        switch (octree[i].w)
        {
        case LEAF:
            return (leaf){
                (glm::uvec4){(pos.x/s)*s,(pos.y/s)*s,(pos.z/s)*s, octree[i].w},
                (uint){s},
                i
            };
        case NODE:
            offset = octree[i].x;
            break;
        default:
            return (leaf){
                octree[i],
                (uint){1},
                i
            };
        }

        d++;
        p2*=2;
    }
}