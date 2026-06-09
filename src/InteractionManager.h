#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Chunk.h"

struct RaycastResult {
    bool hit;
    int x, y, z;      // Voxel position that was hit
    int nx, ny, nz;   // Neighbor position for block placement
};

class InteractionManager {
public:
    static bool leftMousePressed;
    static bool rightMousePressed;

    // Core Raycaster matching your block grid coordinate system
    static RaycastResult castRay(glm::vec3 start, glm::vec3 dir, World& world, float maxDistance = 5.0f);
    
    // Process mouse clicks for mining (left click) and placing (right click)
    static void handleMouseInteraction(GLFWwindow* window, World& world, glm::vec3 cameraPos, glm::vec3 cameraFront);
};