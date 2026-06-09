#pragma once
#include <glm/glm.hpp>
#include "Chunk.h"

class Spawner {
public:
    // Calculates a perfect, reliable surface block coordinate to drop the camera onto
    static glm::vec3 findSafeSpawn(int targetX, int targetZ, World& world);
};