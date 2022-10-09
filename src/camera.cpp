#include "camera.hpp"


Camera::Camera(GLFWwindow *window_, Stats *stats_, Properties *properties_, glm::vec3 position_, glm::vec3 direction_){
    window_p = window_;
    stats_p = stats_;
    properties = properties_;
    position = position_;
    direction = direction_;
}

void Camera::FirstPersonHandler(const keyLayout &layout, float speed, glm::vec3 up, double Sensitivity){
    up = glm::normalize(up);
    if(glfwGetKey(window_p, layout.up) == GLFW_PRESS)//up movement
        position += (float)((*stats_p).MS*speed) * up;
    
    if(glfwGetKey(window_p, layout.down) == GLFW_PRESS)//down movement
        position -= (float)((*stats_p).MS*speed) * up;

    if(glfwGetKey(window_p, layout.forward) == GLFW_PRESS)//forward movement
        position += (float)((*stats_p).MS*speed) * direction;

    if(glfwGetKey(window_p, layout.backward) == GLFW_PRESS)//backward movement
        position -= (float)((*stats_p).MS*speed) * direction;

    if(glfwGetKey(window_p, layout.left) == GLFW_PRESS)//left movement
        position += (float)((*stats_p).MS*speed) * glm::normalize(glm::cross(up, direction));

    if(glfwGetKey(window_p, layout.right) == GLFW_PRESS)//right movement
        position += (float)((*stats_p).MS*speed) * glm::normalize(glm::cross(direction, up));

    static glm::dvec2 newMouse;
    if(layout.x_axis_direction == MOUSE_X && layout.y_axis_direction == MOUSE_Y)
        glfwGetCursorPos(window_p, &newMouse.x, &newMouse.y);
    
    else if(layout.x_axis_direction == MOUSE_Y && layout.y_axis_direction == MOUSE_X)
        glfwGetCursorPos(window_p, &newMouse.y, &newMouse.x);

    glm::dvec2 delta = {newMouse.x - mousePosition_p.x, newMouse.y - mousePosition_p.y};
    mousePosition_p = {newMouse.x,newMouse.y};

    if((rotation_p + delta * Sensitivity * (*stats_p).MS).y > 80 || (rotation_p + delta * Sensitivity * (*stats_p).MS).y < -80) return;
    
    rotation_p += delta * Sensitivity * (*stats_p).MS;
    direction = glm::rotate({1,0,0}, (float)-glm::radians(rotation_p.x), up);
    direction = glm::rotate(direction, (float)glm::radians(rotation_p.y), glm::normalize(glm::cross({0,1,0}, direction)));
}