#include "octree.hpp"

Octree::Octree(){

}

void Octree::Setup(int depth){
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

void Octree::Insert(glm::uvec3 pos, glm::uvec4 data, uint32_t object_id){
    uint32_t d=1;
    int offset = 0;
    int j = 0;

    while(d<depth_){
        uint32_t i = offset+Locate(pos,d);

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
    }

    uint32_t i = offset+Locate(pos,d);
    if(octree[i].w <= 1)octree[j].z++;
    octree[i]={
        data.x,
        data.y,
        data.z,
        object_id
    };
}

void Octree::Insert(glm::uvec3 pos, glm::uvec4 data, uint32_t object_id, bool (*object_idCondition)(uint32_t)){
    uint32_t d=1;
    int offset = 0;
    int j = 0;
    std::stack<int> nodes;

    while(d<depth_){
        uint32_t i = offset+Locate(pos,d);

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
    }

    uint32_t i = offset+Locate(pos,d);
    if(object_idCondition(octree[j].w)){
        if(octree[i].w <= 1)octree[j].z++;
        octree[i]={
            data.x,
            data.y,
            data.z,
            object_id
        };
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

void Octree::Remove(glm::uvec3 pos){
    uint32_t d=1;
    int offset = 0;
    std::stack<int> nodes;

    while(d<=depth_){
        uint32_t i = offset+Locate(pos,d);

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

            return;
        }

        d++;
    }

}

void Octree::Remove(glm::uvec3 pos, bool (*object_idCondition)(uint32_t)){
    uint32_t d=1;
    int offset = 0;
    std::stack<int> nodes;

    while(d<=depth_){
        uint32_t i = offset+Locate(pos,d);

        switch (octree[i].w)
        {
        case LEAF:
            return;
        case NODE:
            nodes.push(i);
            offset = octree[i].x;
            break;
        default:
            if(!object_idCondition(octree[i].w))return;

            octree[i] = {0,0,0,0};
            octree[nodes.top()].z--;

            while(octree[nodes.top()].z <= 0 && !nodes.empty()){
                free_mem.push(octree[nodes.top()].x);
                octree[nodes.top()] = {0,0,0,0};
                nodes.pop();
                octree[nodes.top()].z--;
            }

            return;
        }

        d++;
    }

}

void Octree::Move(glm::uvec3 from, glm::uvec3 to){
    uint32_t d=1;
    int offset = 0;
    std::stack<int> nodes;
    glm::uvec4 data;

    while(d<=depth_){
        uint32_t i = offset+Locate(from,d);

        switch (octree[i].w)
        {
        case LEAF:
            return;
        case NODE:
            nodes.push(i);
            offset = octree[i].x;
            break;
        default:
            data = octree[i];
            octree[i] = {0,0,0,0};
            octree[nodes.top()].z--;

            while(octree[nodes.top()].z <= 0 && !nodes.empty()){
                free_mem.push(octree[nodes.top()].x);
                octree[nodes.top()] = {0,0,0,0};
                nodes.pop();
                octree[nodes.top()].z--;
                d--;
            }

            while(!inNode(to, from, d)){
                nodes.pop();
                d--;
            }
            offset = nodes.top();
            int k = 0;

            while(d<=depth_){
                uint32_t j = offset+Locate(to,d);

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
                        octree[j].x =offset;
                        octree[j].w = NODE;
                        octree[j].z = 0;
                        if(d>1)octree[j].z++;
                        k = j;
                        break;
                    case NODE:
                        k = j;
                        offset = octree[j].y;
                        break;
                }
                d++;
            }

            uint32_t q = offset+Locate(to,d);
            if(octree[q].w <= 1)octree[k].z++;
            octree[q]=data;
            return;
        }

        d++;
    }
}

Octree::leaf Octree::Lookup(glm::uvec3 pos){
    uint32_t d=1;
    int offset = 0;
    leaf l;

    while(d<=depth_){
        uint32_t i = offset+Locate(pos,d);
        int s = n/(1<<d);

        switch (octree[i].w)
        {
        case LEAF:
            l.data = (glm::uvec4){(pos.x/s)*s,(pos.y/s)*s,(pos.z/s)*s, octree[i].w};
            l.size = s;
            l.id = i;
            return l;
        case NODE:
            offset = octree[i].x;
            break;
        default:
            l.data = octree[i];
            l.size = 1;
            l.id = i;
            return l;
        }

        d++;
    }
}