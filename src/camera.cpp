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

    static glm::dvec2 mouse;

    glfwGetCursorPos(window_, &mouse.x, &mouse.y);
    
    //normal x: up
    //normal y: glm::normalize(glm::cross({0,1,0}, direction))

    if(first_frame){
        first_frame = false;

        start_angle = glm::vec2((float)mouse.x,(float)mouse.y);

        rotation = glm::vec2(0,0);
    }else{
        rotation.x = (rotation.x + (mouse.x - start_angle.x) * 0.6);
        rotation.y = (rotation.y - (mouse.y - start_angle.y) * 0.6);

        direction.x = cosf(glm::radians(rotation.x))*cosf(glm::radians(rotation.y));
        direction.y = sinf(glm::radians(rotation.y));
        direction.z = sinf(glm::radians(rotation.x))*cosf(glm::radians(rotation.y));

        direction = glm::normalize(direction);

        start_angle = glm::vec2((float)mouse.x,(float)mouse.y);
    }

    LOGGING->print(VERBOSE) << direction.x << ' ' << direction.y << ' ' << direction.z << '\n';

}

CameraUBO Camera::getUBO(){
    std::lock_guard<std::mutex> lock(uboMutex_);
    CameraUBO cameraData;
    cameraData.position = position;
    //int width,height;
    //glfwGetWindowSize(window_, &width, &height);
    float aspectRatio = 600.0f/800.0f;
    cameraData.direction = direction;
    cameraData.cameraPlanVector = direction;
    cameraData.cameraPlanSurfaceRightVector = glm::normalize(glm::cross(glm::vec3(0,1,0), direction)) * tanf(glm::radians(90.0f/2));
    cameraData.cameraPlanSurfaceUpVector = glm::normalize(glm::cross(cameraData.cameraPlanSurfaceRightVector, direction)) * aspectRatio * tanf(glm::radians(90.0f/2));
    //LOGGING->print(VERBOSE) << cameraData.cameraPlanSurfaceUpVector.x << ' ' << cameraData.cameraPlanSurfaceUpVector.y << ' ' << cameraData.cameraPlanSurfaceUpVector.z << '\n';
    return cameraData;
}