#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Chunk.h"

class InputHandler {
public:
    static glm::vec3 cameraPos;
    static glm::vec3 cameraFront;
    static glm::vec3 cameraUp;

    static float yaw;
    static float pitch;
    static bool firstMouse;
    static float lastX;
    static float lastY;

    static float verticalVelocity;
    static bool isGrounded;

    static void initialize(float startX, float startY, float startZ);
    static void processKeyboard(GLFWwindow* window, World& world, float deltaTime);
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static bool isSolidBlock(float x, float y, float z, World& world);
};