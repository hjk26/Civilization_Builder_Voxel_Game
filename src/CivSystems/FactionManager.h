#ifndef FACTION_MANAGER_H
#define FACTION_MANAGER_H

#include <vector>
#include <string>
#include <glm/glm.hpp>

enum class Alignment {
    ALLIED,
    NEUTRAL,
    HOSTILE
};

struct Faction {
    unsigned int id;
    std::string name;
    glm::vec3 bannerColor; // Color applied to citizens under this flag
    int totalReputation = 0;
};

class FactionManager {
public:
    static std::vector<Faction> activeFactions;
    static unsigned int nextFactionId;

    static void createFaction(std::string name, glm::vec3 color);
    static Faction* getFaction(unsigned int factionId);
    static Alignment getRelationship(unsigned int factionA, unsigned int factionB);
};

#endif // FACTION_MANAGER_H