#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "DNA.h"
#include "../Chunk.h"
#include "../Core/ResourceManager.h"

enum class AIState {
    WANDERING,
    MOVING_TO_RESOURCE,
    HARVESTING,
    RETURNING_TO_STOCKPILE
};

enum class JobType {
    UNASSIGNED,
    MINER,
    LUMBERJACK,
    SCHOLAR
};

struct Citizen {
    unsigned int id;
    std::string name;
    glm::vec3 position;
    glm::vec3 targetDestination;
    bool hasTarget = false;
    std::vector<glm::vec3> currentPath;
    int currentPathIndex = 0;
    unsigned int factionId = 1;
    
    float hunger = 100.0f;
    CitizenDNA dna;

    // --- NEW STATE & JOB SYSTEM ---
    AIState currentState = AIState::WANDERING;
    JobType job = JobType::UNASSIGNED;
    glm::ivec3 targetBlockPos = glm::ivec3(0);
    float harvestTimer = 0.0f; // This will act as our processing/harvest clock!

    // --- LOGISTICS & INVENTORY FIELDS ---
    int heldResourceAmount = 0;            // Internal inventory count
    ResourceType heldResourceType = ResourceType::STONE; // What item type they carry
};

class EntityManager {
public:
    static std::vector<Citizen> activeCitizens;
    static unsigned int nextCitizenId;

    static unsigned int citizenVAO;
    static unsigned int citizenVBO;
    static int citizenVertexCount;

    static void initializeRenderer();
    static void spawnCitizen(glm::vec3 spawnPos, uint64_t manualSeed = 0);
    static void updateEntities(float deltaTime, World& world);
    static void renderEntities(unsigned int shaderProgram, unsigned int modelLoc, unsigned int overrideColorLoc);
    
    // --- NEW BEHAVIOR UTILITIES ---
    static void assignJobBasedOnDNA(Citizen& citizen);
    static glm::ivec3 findNearestResource(glm::vec3 pos, int blockID, World& world, int radius = 20);

    static void debugPrintSettlement();
};

#endif // ENTITY_MANAGER_H