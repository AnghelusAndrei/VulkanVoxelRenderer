#include "camera.hpp"

Camera::Camera(GLFWwindow *window, glm::vec3 position_, glm::vec3 direction_, Properties &properties_, VoxelEngine::Stats &stats) : 
                  window_(window), position(position_),direction(direction_), properties(properties_),                 stats_(stats) {}

void Camera::FirstPersonHandler(keyLayout &layout, float speed, glm::vec3 up, double Sensitivity){
    up = glm::normalize(up);
    if(glfwGetKey(window_, layout.up) == GLFW_PRESS)//up movement
        position += (float)(stats_.MS*speed) * up;
    
    if(glfwGetKey(window_, layout.down) == GLFW_PRESS)//down movement
        position -= (float)(stats_.MS*speed) * up;

    if(glfwGetKey(window_, layout.forward) == GLFW_PRESS)//forward movement
        position += (float)(stats_.MS*speed) * direction;

    if(glfwGetKey(window_, layout.forward) == GLFW_PRESS)//backward movement
        position -= (float)(stats_.MS*speed) * direction;

    if(glfwGetKey(window_, layout.forward) == GLFW_PRESS)//left movement
        position += (float)(stats_.MS*speed) * glm::normalize(glm::cross(up, direction));

    if(glfwGetKey(window_, layout.forward) == GLFW_PRESS)//right movement
        position += (float)(stats_.MS*speed) * glm::normalize(glm::cross(direction, up));

    static glm::dvec2 newMouse;
    if(layout.x_axis_direction == MOUSE_X && layout.y_axis_direction == MOUSE_Y)
        glfwGetCursorPos(window_, &newMouse.x, &newMouse.y);
    
    else if(layout.x_axis_direction == MOUSE_Y && layout.y_axis_direction == MOUSE_X)
        glfwGetCursorPos(window_, &newMouse.y, &newMouse.x);

    glm::dvec2 delta = {newMouse.x - mousePosition.x, newMouse.y - mousePosition.y};
    mousePosition = {newMouse.x,newMouse.y};

    if((rotation + delta * Sensitivity * stats_.MS).y > 80 || (rotation + delta * Sensitivity * stats_.MS).y < -80) return;
    
    rotation += delta * Sensitivity * stats_.MS;
    direction = glm::rotate({1,0,0}, (float)-glm::radians(rotation.x), up);
    direction = glm::rotate(direction, (float)glm::radians(rotation.y), glm::normalize(glm::cross({0,1,0}, direction)));
}