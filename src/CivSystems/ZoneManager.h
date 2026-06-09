#ifndef ZONE_MANAGER_H
#define ZONE_MANAGER_H

#include <vector>
#include <string>
#include <glm/glm.hpp>

enum class ZoneType {
    STOCKPILE,
    MINING_DISTRICT,
    FORESTRY,
    RESIDENTIAL
};

struct Zone {
    unsigned int id;
    std::string name;
    ZoneType type;
    glm::ivec3 minBounds; // Bottom-left-front corner of the zone
    glm::ivec3 maxBounds; // Top-right-back corner of the zone
    unsigned int owningFactionId = 1;
    
    bool contains(glm::ivec3 pos) const {
        return (pos.x >= minBounds.x && pos.x <= maxBounds.x &&
                pos.y >= minBounds.y && pos.y <= maxBounds.y &&
                pos.z >= minBounds.z && pos.z <= maxBounds.z);
    }
};

class ZoneManager {
public:
    static std::vector<Zone> activeZones;
    static unsigned int nextZoneId;

    static void createZone(std::string name, ZoneType type, glm::ivec3 minB, glm::ivec3 maxB);
    static glm::ivec3 getRandomVoxelInZone(unsigned int zoneId);
    static int findZoneAt(glm::ivec3 pos); // Returns Zone ID or -1 if unzoned
    static glm::ivec3 getNearestZoneCenter(glm::vec3 currentPos, ZoneType type, unsigned int factionId);
};

#endif // ZONE_MANAGER_H