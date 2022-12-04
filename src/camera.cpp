#include "camera.hpp"


Camera::Camera(GLFWwindow *window,  glm::vec3 position_, glm::vec3 direction_){
    window_ = window;
    position = position_;
    direction = direction_;
    speed=SETTINGS->get<double>("speed", 10.0);
}


void Camera::input(){
    std::lock_guard<std::mutex> lock(uboMutex_);
    glm::vec3 up = glm::vec3(0,1,0);

    
    double time=glfwGetTime();
    double deltaTime=time-lastTime_;
    lastTime_=glfwGetTime();
    if(glfwGetKey(window_, SETTINGS->get<int64_t>("up",GLFW_KEY_SPACE)) == GLFW_PRESS)//up movement
        position += (float)(deltaTime*speed) * up;
    
    if(glfwGetKey(window_, SETTINGS->get<int64_t>("down",GLFW_KEY_LEFT_SHIFT)) == GLFW_PRESS)//down movement
        position -= (float)(deltaTime*speed) * up;

    if(glfwGetKey(window_, SETTINGS->get<int64_t>("forward",GLFW_KEY_W)) == GLFW_PRESS)//forward movement
        position += (float)(deltaTime*speed) * direction;

    if(glfwGetKey(window_, SETTINGS->get<int64_t>("backward", GLFW_KEY_S)) == GLFW_PRESS)//backward movement
        position -= (float)(deltaTime*speed) * direction;

    if(glfwGetKey(window_, SETTINGS->get<int64_t>("left",GLFW_KEY_A)) == GLFW_PRESS)//left movement
        position += (float)(deltaTime*speed) * glm::normalize(glm::cross(up, direction));

    if(glfwGetKey(window_, SETTINGS->get<int64_t>("right",GLFW_KEY_D)) == GLFW_PRESS)//right movement
        position += (float)(deltaTime*speed) * glm::normalize(glm::cross(direction, up));

    static glm::dvec2 newMouse;

    glfwGetCursorPos(window_, &newMouse.x, &newMouse.y);
    glfwGetCursorPos(window_, &newMouse.y, &newMouse.x);

    glm::dvec2 delta = {newMouse.x - mousePosition_.x, newMouse.y - mousePosition_.y};
    mousePosition_ = {newMouse.x,newMouse.y};

    if((rotation_ + delta * 0.1 * deltaTime).y > 80 || (rotation_ + delta * 0.1 * deltaTime).y < -80) return;
    
    rotation_+= delta * 0.1 * deltaTime;
    direction = glm::rotate({1,0,0}, (float)-glm::radians(rotation_.x), up);
    direction = glm::rotate(direction, (float)glm::radians(rotation_.y), glm::normalize(glm::cross({0,1,0}, direction)));

    //LOGGING->print(VERBOSE) << position.x << ' ' << position.y << ' ' << position.z << '\n';

}

CameraUBO Camera::getUBO(){
    std::lock_guard<std::mutex> lock(uboMutex_);
    CameraUBO cameraData;
    cameraData.position = position;
    cameraData.direction = direction;
    int width,height;
    glfwGetWindowSize(window_, &width, &height);
    cameraData.cameraPlanVector = direction * (float)(SETTINGS->get<double>("FOV", 90.0)/2) * (width/2.0f);
    cameraData.cameraPlanSurfaceRightVector = glm::normalize(glm::cross(glm::vec3(0,-1,0), cameraData.cameraPlanVector));
    cameraData.cameraPlanSurfaceUpVector = glm::normalize(glm::cross(cameraData.cameraPlanSurfaceRightVector, cameraData.cameraPlanVector));
    return cameraData;
}