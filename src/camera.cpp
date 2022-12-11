#include "camera.hpp"


Camera::Camera(GLFWwindow *window,  glm::vec3 position_, glm::vec3 direction_){
    window_ = window;
    position = position_;
    direction = direction_;
    speed=SETTINGS->get<double>("speed", 10.0);
}

float Camera::FixX(float ang)
{
    if (ang > 359)
    {
        float mult = floor(ang / 360);
        ang = ang - (360 * mult);
    }
    if (ang < 0)
    {
        float ap = abs(ang);
        float mult = ceil(ap / 360);
        ang = (360 * mult) - ap;
    }
    return ang;
}

float Camera::FixY(float ang){
    if (ang > 89)
    {
        ang = 89;
    }
    if (ang < -89)
    {
        ang = -89;
    }
    return ang;
}


void Camera::input(){
    std::lock_guard<std::mutex> lock(uboMutex_);
    glm::vec3 up = glm::vec3(0,1,0);

    
    double time=glfwGetTime();
    double deltaTime=time-lastTime_;
    lastTime_=glfwGetTime();
    glm::vec2 dir2d = glm::normalize(glm::vec2(direction.x,direction.z));
    glm::vec3 dir3d = glm::vec3(dir2d.x,0,dir2d.y);
    if(glfwGetKey(window_, SETTINGS->get<int64_t>("up",GLFW_KEY_SPACE)) == GLFW_PRESS)//up movement
        position += (float)(deltaTime*speed) * up;
    
    if(glfwGetKey(window_, SETTINGS->get<int64_t>("down",GLFW_KEY_LEFT_SHIFT)) == GLFW_PRESS)//down movement
        position -= (float)(deltaTime*speed) * up;

    if(glfwGetKey(window_, SETTINGS->get<int64_t>("forward",GLFW_KEY_W)) == GLFW_PRESS)//forward movement
        position -= (float)(deltaTime*speed) * dir3d;

    if(glfwGetKey(window_, SETTINGS->get<int64_t>("backward", GLFW_KEY_S)) == GLFW_PRESS)//backward movement
        position += (float)(deltaTime*speed) * dir3d;

    if(glfwGetKey(window_, SETTINGS->get<int64_t>("left",GLFW_KEY_A)) == GLFW_PRESS)//left movement
        position += (float)(deltaTime*speed) * glm::normalize(glm::cross(up, dir3d));

    if(glfwGetKey(window_, SETTINGS->get<int64_t>("right",GLFW_KEY_D)) == GLFW_PRESS)//right movement
        position += (float)(deltaTime*speed) * glm::normalize(glm::cross(dir3d, up));

    static glm::dvec2 mouse;

    glfwGetCursorPos(window_, &mouse.x, &mouse.y);
    
    //normal x: up
    //normal y: glm::normalize(glm::cross({0,1,0}, direction))

    if(first_frame){
        first_frame = false;

        start_angle = glm::vec2((float)mouse.x,(float)mouse.y);

        rotation = glm::vec2(0,0);
    }else{
        rotation.x = FixX(rotation.x - (mouse.x - start_angle.x) * 0.1);
        rotation.y = FixY(rotation.y + (mouse.y - start_angle.y) * 0.1);

        direction.x = cosf(glm::radians(rotation.x))*cosf(glm::radians(rotation.y));
        direction.y = sinf(glm::radians(rotation.y));
        direction.z = sinf(glm::radians(rotation.x))*cosf(glm::radians(rotation.y));

        start_angle = glm::vec2((float)mouse.x,(float)mouse.y);
    }

    //LOGGING->print(VERBOSE) << direction.x << ' ' << direction.y << ' ' << direction.z << '\n';

}

CameraUBO Camera::getUBO(){
    std::lock_guard<std::mutex> lock(uboMutex_);
    CameraUBO cameraData;
    cameraData.position = glm::vec4(position,0);
    //int width,height;
    //glfwGetWindowSize(window_, &width, &height);
    float aspectRatio = 600.0f/800.0f;
    cameraData.direction = glm::vec4(direction,0);
    cameraData.cameraPlanVector = glm::vec4(direction,0);
    cameraData.cameraPlanSurfaceRightVector =glm::vec4(glm::normalize(glm::cross(glm::vec3(0, 1, 0), direction)) * tanf(glm::radians((float)90.0f/2)),0);
    cameraData.cameraPlanSurfaceUpVector = glm::vec4(glm::normalize(glm::cross(glm::vec3(cameraData.cameraPlanSurfaceRightVector.x, cameraData.cameraPlanSurfaceRightVector.y, cameraData.cameraPlanSurfaceRightVector.z), direction)) * tanf(glm::radians((float)90.0f/2)) * aspectRatio,0);
    
    //LOGGING->print(VERBOSE) << "POS: "<< '[' << cameraData.position.x << ',' << cameraData.position.y << ',' << cameraData.position.z << "]\n";
    //LOGGING->print(VERBOSE) << "DIR: "<< '[' << cameraData.direction.x << ',' << cameraData.direction.y << ',' << cameraData.direction.z << "]\n";
    //LOGGING->print(VERBOSE) << "UP: "<< '[' << cameraData.cameraPlanSurfaceUpVector.x << ',' << cameraData.cameraPlanSurfaceUpVector.y << ',' << cameraData.cameraPlanSurfaceUpVector.z << "]\n";
    //LOGGING->print(VERBOSE) << "RIGHT: "<< '[' << cameraData.cameraPlanSurfaceRightVector.x << ',' << cameraData.cameraPlanSurfaceRightVector.y << ',' << cameraData.cameraPlanSurfaceRightVector.z << "]\n";
    
    return cameraData;
}