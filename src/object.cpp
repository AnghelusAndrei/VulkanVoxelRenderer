#include "object.hpp"

/*Object::Object(Octree *octree, glm::uvec3 position_, glm::uvec2 rotation_, glm::vec3 scale_) : octree_p(octree), position(position_), rotation(rotation_), scale(scale_), Utils(new utils){
}

Object::~Object(){
    remove();
}

void Object::update(){
    for(int i = 0; i<voxels_p.size(); i++){
        glm::uvec3 newVoxel;
        glm::vec3 normal;
        glm::uvec2 normalRot;
        glm::vec3 rawVoxel; // direction of voxel
        glm::uvec2 voxelRot;
        glm::vec3 fvoxel;


        normal = Utils->GetVNormal(voxels_data_p[i]); voxels_data_p[i] = Utils->RemoveNormal(voxels_data_p[i]);
        normalRot = Utils->vectorToEuler(normal) - oldRotation_p + rotation;
        normal = Utils->eulerToVector(normalRot); voxels_data_p[i] = Utils->AddNormal(voxels_data_p[i], Utils->SetVNormal(normal));

        fvoxel = glm::vec3(voxels_p[i]);
        float vlength = glm::length(fvoxel);
        rawVoxel = glm::normalize(fvoxel);
        voxelRot = Utils->vectorToEuler(rawVoxel) - oldRotation_p + rotation;
        rawVoxel = Utils->eulerToVector(voxelRot) * vlength; newVoxel = SetCurrent(voxels_p[i]);

        newVoxel -= oldPosition_p;
        newVoxel += position;


        octree_p->Move(voxels_p[i], newVoxel, voxels_data_p[i], id);
        oldPosition_p = position;
    }
    octree_p->upToDate = false;
}

void Object::custom(void (*customVoxelIntruction)(uint32_t voxel_Id, Octree *octree))
{
    for(int i = 0; i<voxels_p.size(); i++){
        customVoxelIntruction(i, octree_p);
    }
}

void Object::remove(){
    for(int i=voxels_p.size()-1;i>=0;i--){
        octree_p->Remove(voxels_p[i]);
        voxels_p.pop_back();
        voxels_data_p.pop_back();
    }
}


bool Object::loadWavefrontObj(std::string filename, bool hasTexture, bool hasNormal){
    std::ifstream inFile(filename); 
    int length = std::count(std::istreambuf_iterator<char>(inFile), std::istreambuf_iterator<char>(), '\n');
    int k = 0,q = 0;

	std::ifstream f(filename);
	if (!f.is_open())
		return false;

    mesh_p.hasTexture = hasTexture;
    mesh_p.hasNormal = hasNormal;



	std::vector<glm::vec3> verts;
	std::vector<glm::vec3> texs;
    std::vector<glm::vec3> normals;
    std::vector<triangle> tris;

	while (!f.eof())
	{
		char line[128];
		f.getline(line, 128);

		std::stringstream s;
		s << line;

		char junk;

		if (line[0] == 'v')
		{
			if (line[1] == 't')
			{
				glm::vec3 v;
				s >> junk >> junk >> v.x >> v.y;
				texs.push_back(v);
			}
            else if(line[1] == 'n'){
				glm::vec3 v;
				s >> junk >> v.x >> v.y >> v.z;
                normals.push_back(v);
            }
			else
			{
				glm::vec3 v;
				s >> junk >> v.x >> v.y >> v.z;
				verts.push_back(v);

                if(!hasNormal){
                    normals.push_back(glm::vec3(0,0,0));
                }

                if(v.x > BoundingBox_p.MAX.x)BoundingBox_p.MAX.x = v.x;
                if(v.y > BoundingBox_p.MAX.y)BoundingBox_p.MAX.y = v.y;
                if(v.z > BoundingBox_p.MAX.z)BoundingBox_p.MAX.z = v.z;

                if(v.x < BoundingBox_p.MIN.x)BoundingBox_p.MIN.x = v.x;
                if(v.y < BoundingBox_p.MIN.y)BoundingBox_p.MIN.y = v.y;
                if(v.z < BoundingBox_p.MIN.z)BoundingBox_p.MIN.z = v.z;
			}
		}
		

		if (!hasTexture)
		{
			if (line[0] == 'f')
			{
                triangle tr;

                if(!hasNormal){
                    int f[3];
                    s >> junk >> f[0] >> f[1] >> f[2];
                    tr = { verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] };
                    tr.vertex_id[0] = f[0] - 1;
                    tr.vertex_id[1] = f[1] - 1;
                    tr.vertex_id[2] = f[2] - 1;
                }else{
                    s >> junk;

                    std::string tokens[6];
                    int nTokenCount = -1;


                    while (!s.eof())
                    {
                        char c = s.get();
                        if (c == ' ' || c == '/')
                            nTokenCount++;
                        else
                            tokens[nTokenCount].append(1, c);
                    }

                    tokens[nTokenCount].pop_back();


                    tr.v[0] = verts[stoi(tokens[0]) - 1];
                    tr.v[1] = verts[stoi(tokens[2]) - 1];
                    tr.v[2] = verts[stoi(tokens[4]) - 1];
                    
                    tr.normal[0] = normals[stoi(tokens[1]) - 1];
                    tr.normal[1] = normals[stoi(tokens[3]) - 1];
                    tr.normal[2] = normals[stoi(tokens[5]) - 1];

                    tr.vertex_id[0] = stoi(tokens[0]) - 1;
                    tr.vertex_id[1] = stoi(tokens[2]) - 1;
                    tr.vertex_id[2] = stoi(tokens[4]) - 1;
                }

                if(!hasNormal){
                    glm::vec3 line1 = tr.v[1] - tr.v[0];
                    glm::vec3 line2 = tr.v[2] - tr.v[0];
                    glm::vec3 line3 = tr.v[2] - tr.v[1];

                    glm::vec3 normal = glm::normalize(glm::cross(line1, line2));

                    normals[tr.vertex_id[0]] +=  normal;
                    normals[tr.vertex_id[1]] +=  normal;
                    normals[tr.vertex_id[2]] +=  normal;
                }

                tris.push_back(tr);
			}
        }
		else
		{
			if (line[0] == 'f')
			{
                triangle tr;

                if(!hasNormal){
                    s >> junk;

                    std::string tokens[6];
                    int nTokenCount = -1;


                    while (!s.eof())
                    {
                        char c = s.get();
                        if (c == ' ' || c == '/')
                            nTokenCount++;
                        else
                            tokens[nTokenCount].append(1, c);
                    }

                    tokens[nTokenCount].pop_back();


                    tr = { verts[stoi(tokens[0]) - 1], verts[stoi(tokens[2]) - 1], verts[stoi(tokens[4]) - 1], texs[stoi(tokens[1]) - 1], texs[stoi(tokens[3]) - 1], texs[stoi(tokens[5]) - 1] };

                    tr.vertex_id[0] = stoi(tokens[0]) - 1;
                    tr.vertex_id[1] = stoi(tokens[2]) - 1;
                    tr.vertex_id[2] = stoi(tokens[4]) - 1;
                }else{
                    s >> junk;

                    std::string tokens[20];
                    int nTokenCount = -1;


                    while (!s.eof())
                    {
                        char c = s.get();
                        if (c == ' ' || c == '/')
                            nTokenCount++;
                        else
                            tokens[nTokenCount].append(1, c);
                    }

                    tokens[nTokenCount].pop_back();


                    tr.v[0] = verts[stoi(tokens[0]) - 1];
                    tr.v[1] = verts[stoi(tokens[3]) - 1];
                    tr.v[2] = verts[stoi(tokens[6]) - 1];

                    tr.t[0] = texs[stoi(tokens[1]) - 1];
                    tr.t[1] = texs[stoi(tokens[4]) - 1];
                    tr.t[2] = texs[stoi(tokens[7]) - 1];
                    
                    tr.normal[0] = normals[stoi(tokens[2]) - 1];
                    tr.normal[1] = normals[stoi(tokens[5]) - 1];
                    tr.normal[2] = normals[stoi(tokens[8]) - 1];

                    tr.vertex_id[0] = stoi(tokens[0]) - 1;
                    tr.vertex_id[1] = stoi(tokens[3]) - 1;
                    tr.vertex_id[2] = stoi(tokens[6]) - 1;
                }

                if(!hasNormal){
                    glm::vec3 line1 = tr.v[1] - tr.v[0];
                    glm::vec3 line2 = tr.v[2] - tr.v[0];
                    glm::vec3 line3 = tr.v[2] - tr.v[1];

                    glm::vec3 normal = glm::normalize(glm::cross(line1, line2));

                    normals[tr.vertex_id[0]] +=  normal;
                    normals[tr.vertex_id[1]] +=  normal;
                    normals[tr.vertex_id[2]] +=  normal;
                }


                tris.push_back(tr);
			}
		}
	}

    for(int i = 0; i < tris.size(); i++){
        for(int j = 0; j < 3; j++){
            if(!hasNormal){
                if(glm::length(normals[tris[i].vertex_id[j]]) != 0){
                    tris[i].normal[j] = glm::normalize(normals[tris[i].vertex_id[j]]);
                }
            }else{
                tris[i].normal[j] = glm::normalize(tris[i].normal[j]);
            }

            float vlength = glm::length(tris[i].v[j]);
            glm::uvec2 voxelRot;
            tris[i].v[j] = glm::normalize(tris[i].v[j]);
            voxelRot = Utils->vectorToEuler(tris[i].v[j]) - oldRotation_p + rotation;
            tris[i].v[j] = Utils->eulerToVector(voxelRot) * vlength;
            tris[i].v[j] = (tris[i].v[j] - BoundingBox_p.MIN) * scale + glm::vec3(position);
        }

        Rasterize(tris[i]);
    }
}

void Object::Rasterize(triangle t){
    
}


//private

glm::vec3 Object::GetRaw(glm::uvec3 voxel){
        glm::vec3 rawVoxel;
        rawVoxel = glm::vec3(voxel - oldPosition_p);

        rawVoxel.x = rawVoxel.x / scale.x + BoundingBox_p.MIN.x;
        rawVoxel.y = rawVoxel.y / scale.y + BoundingBox_p.MIN.y;
        rawVoxel.z = rawVoxel.z / scale.z + BoundingBox_p.MIN.z;
}

glm::uvec3 Object::SetCurrent(glm::vec3 raw){
        glm::vec3 rawVoxel;

        rawVoxel.x = (raw.x + BoundingBox_p.MIN.x)*scale.x;
        rawVoxel.x = (raw.y + BoundingBox_p.MIN.y)*scale.y;
        rawVoxel.x = (raw.z + BoundingBox_p.MIN.z)*scale.z;

        rawVoxel += oldPosition_p;
}*/