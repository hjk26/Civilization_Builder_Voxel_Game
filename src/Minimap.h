#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "Terrain.h"

class Minimap {
public:
    static unsigned int quadVAO, quadVBO;
    static unsigned int mapTexture;
    static unsigned int shaderProgram;
    
    // Toggleable states
    static bool isMaximized;
    static bool mKeyWasPressed;

    static void initialize();
    static void checkInput(GLFWwindow* window); // Listen for the 'M' key
    static void updateAndRender(glm::vec3 playerPos, int screenWidth, int screenHeight);
    static void cleanup();
};