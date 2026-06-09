#ifndef PATHFINDING_H
#define PATHFINDING_H

#include <vector>
#include <glm/glm.hpp>
#include "../Chunk.h" // Requires access to your world block-checking methods

struct PathNode {
    glm::ivec3 position;
    float gCost; // Distance from starting node
    float hCost; // Heuristic distance to destination node
    glm::ivec3 parentPos;

    float fCost() const { return gCost + hCost; }
    
    bool operator==(const PathNode& other) const {
        return position == other.position;
    }
};

class Pathfinding {
public:
    // Finds a valid 3D walking path from start to target coordinates
    static std::vector<glm::vec3> findPath(glm::vec3 start, glm::vec3 target, World& world);

private:
    static float getDistance(glm::ivec3 a, glm::ivec3 b);
    static std::vector<glm::ivec3> getNeighbors(glm::ivec3 pos, World& world);
    static bool isValidWalkableSpace(glm::ivec3 pos, World& world);
};

#endif // PATHFINDING_H