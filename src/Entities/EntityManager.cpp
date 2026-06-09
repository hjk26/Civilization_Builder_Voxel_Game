#include "EntityManager.h"
#include "../Core/ResourceManager.h"
#include "../Core/TimeManager.h" 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "Pathfinding.h"
#include "../Industry/ChronicleDatabase.h"
#include "../CivSystems/ZoneManager.h"
#include "../CivSystems/FactionManager.h" // Linked to access banner colors

// Instantiate static collections
std::vector<Citizen> EntityManager::activeCitizens;
unsigned int EntityManager::nextCitizenId = 1;

unsigned int EntityManager::citizenVAO = 0;
unsigned int EntityManager::citizenVBO = 0;
int EntityManager::citizenVertexCount = 0;

const std::string FIRST_NAMES[] = { "Haruto", "Sora", "Ren", "Yuki", "Hiro", "Sakura", "Aoi", "Hina", "Mei", "Kokoa" };
const std::string LAST_NAMES[]  = { "Sato", "Suzuki", "Takahashi", "Tanaka", "Watanabe", "Ito", "Nakamura", "Kobayashi" };

void EntityManager::initializeRenderer() {
    float cubeVertices[] = {
        // Back face
        -0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,          
         0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,
        // Front face
        -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f,
        // Left face
        -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
        // Right face
        0.5f,  0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,          
         0.5f, -0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f,     
        // Bottom face
        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f,
        // Top face
        -0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,     
         0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f        
    };

    citizenVertexCount = 36;

    glGenVertexArrays(1, &citizenVAO);
    glGenBuffers(1, &citizenVBO);

    glBindVertexArray(citizenVAO);
    glBindBuffer(GL_ARRAY_BUFFER, citizenVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}

void EntityManager::spawnCitizen(glm::vec3 spawnPos, uint64_t manualSeed) {
    Citizen citizen;
    citizen.id = nextCitizenId++;
    
    if (manualSeed == 0) {
        uint64_t r1 = std::rand() & 0xFFFF;
        uint64_t r2 = std::rand() & 0xFFFF;
        uint64_t r3 = std::rand() & 0xFFFF;
        uint64_t r4 = std::rand() & 0xFFFF;
        manualSeed = (r1 << 48) | (r2 << 32) | (r3 << 16) | r4;
    }
    
    citizen.dna = CitizenDNA::GenerateFromSeed(manualSeed, std::rand() % 500, 1);
    
    std::string first = FIRST_NAMES[std::rand() % 10];
    std::string last  = LAST_NAMES[std::rand() % 8];
    citizen.name = first + " " + last;
    
    // Assign a Faction ID systematically (Alternates between 1 and 2 to distribute population)
    citizen.factionId = (citizen.id % 2 == 0) ? 2 : 1;
    
    citizen.position = spawnPos;
    citizen.hasTarget = false;
    citizen.currentState = AIState::WANDERING;
    citizen.harvestTimer = 0.0f;
    citizen.heldResourceAmount = 0;
    citizen.heldResourceType = ResourceType::STONE;
    
    std::cout << "🌟 Citizen Spawned: " << citizen.name << " joined the settlement!" << std::endl;
    citizen.dna.debugPrint();
    assignJobBasedOnDNA(citizen);
    activeCitizens.push_back(citizen);
    
    ChronicleDatabase::logEvent(EventType::POPULATION, citizen.name + " immigrated to the settlement with profile DNA seed: " + std::to_string(manualSeed));
}

void EntityManager::assignJobBasedOnDNA(Citizen& citizen) {
    if (citizen.dna.baseStrength >= 10) {
        citizen.job = JobType::MINER;
    } else if (citizen.dna.baseIntelligence >= 12) {
        citizen.job = JobType::SCHOLAR;
    } else {
        citizen.job = JobType::LUMBERJACK;
    }
}

glm::ivec3 EntityManager::findNearestResource(glm::vec3 pos, int blockID, World& world, int radius) {
    glm::ivec3 center(std::floor(pos.x), std::floor(pos.y), std::floor(pos.z));
    glm::ivec3 closestBlock(-1);
    float minDistance = 99999.0f;

    for (int x = -radius; x <= radius; ++x) {
        for (int z = -radius; z <= radius; ++z) {
            for (int y = -5; y <= 5; ++y) {
                glm::ivec3 checkPos = center + glm::ivec3(x, y, z);
                if (world.getBlock(checkPos.x, checkPos.y, checkPos.z) == blockID) {
                    float dist = glm::distance(pos, glm::vec3(checkPos));
                    if (dist < minDistance) {
                        minDistance = dist;
                        closestBlock = checkPos;
                    }
                }
            }
        }
    }
    return closestBlock;
}

void EntityManager::updateEntities(float deltaTime, World& world) {
    bool tickSimulation = TimeManager::isTickThisFrame;

    for (auto& citizen : activeCitizens) {
        
        // 1. METABOLIC TICK ENGINE
        if (tickSimulation) {
            float metabolismMultiplier = (citizen.dna.primaryTrait == PersonalityTrait::GREEDY) ? 1.5f : 1.0f;
            citizen.hunger -= 1.0f * metabolismMultiplier;
            
            if (citizen.hunger < 30.0f) {
                if (ResourceManager::spendResource(ResourceType::FOOD, 1)) {
                    citizen.hunger = 100.0f;
                    std::cout << "🥪 " << citizen.name << " took a break to eat from the town storage." << std::endl;
                }
            }
        }

        // 2. FINITE STATE MACHINE (FSM)
        switch (citizen.currentState) {
            
            case AIState::WANDERING: {
                bool switchingState = false;
                
                // Job Target Parsing
                int targetBlockId = (citizen.job == JobType::MINER) ? 3 : 1; // 3 = Stone, 1 = Wood
                ZoneType targetZoneType = (citizen.job == JobType::MINER) ? ZoneType::MINING_DISTRICT : ZoneType::STOCKPILE; 

                glm::ivec3 workZoneCenter = ZoneManager::getNearestZoneCenter(citizen.position, targetZoneType, citizen.factionId);
                
                if (workZoneCenter.x != -1) {
                    glm::ivec3 resourcePos = findNearestResource(glm::vec3(workZoneCenter), targetBlockId, world, 20);
                    
                    if (resourcePos.x != -1) {
                        citizen.targetBlockPos = resourcePos;
                        
                        // Injecting a minor position jitter offset to stop pathfinding clumping
                        float jitterX = ((std::rand() % 3) - 1) * 0.25f;
                        float jitterZ = ((std::rand() % 3) - 1) * 0.25f;
                        
                        glm::vec3 walkTarget(resourcePos.x + 0.5f + jitterX, resourcePos.y, resourcePos.z + 0.5f + jitterZ);
                        citizen.currentPath = Pathfinding::findPath(citizen.position, walkTarget, world);
                        
                        if (!citizen.currentPath.empty()) {
                            citizen.currentPathIndex = 0;
                            citizen.targetDestination = citizen.currentPath[0];
                            citizen.hasTarget = true;
                            citizen.currentState = AIState::MOVING_TO_RESOURCE;
                            switchingState = true;
                        }
                    }
                }
                
                if (switchingState) break; 

                // Default wandering behavior fallback (With added localized variance)
                if (!citizen.hasTarget) {
                    int jitterX = (std::rand() % 15) - 7;
                    int jitterZ = (std::rand() % 15) - 7;
                    
                    float randX = citizen.position.x + jitterX;
                    float randZ = citizen.position.z + jitterZ;
                    int ix = static_cast<int>(std::floor(randX));
                    int iz = static_cast<int>(std::floor(randZ));
                    int groundY = 64;
                    for (int y = 127; y >= 0; --y) {
                        if (world.getBlock(ix, y, iz) != 0) { groundY = y + 1; break; }
                    }
                    glm::vec3 potentialTarget((float)ix + 0.5f, (float)groundY, (float)iz + 0.5f);
                    citizen.currentPath = Pathfinding::findPath(citizen.position, potentialTarget, world);
                    if (!citizen.currentPath.empty()) {
                        citizen.currentPathIndex = 0;
                        citizen.targetDestination = citizen.currentPath[0];
                        citizen.hasTarget = true;
                    }
                }
                break;
            }

            case AIState::MOVING_TO_RESOURCE: {
                if (!citizen.hasTarget) {
                    citizen.currentState = AIState::HARVESTING;
                    citizen.harvestTimer = 0.0f; // Match variable names here
                }
                break;
            }

            case AIState::HARVESTING: {
                // Apply trait speed scaling factors dynamically
                float speedMultiplier = 1.0f;
                if (citizen.dna.primaryTrait == PersonalityTrait::INDUSTRIOUS) {
                    speedMultiplier = 1.35f; 
                } else if (citizen.dna.primaryTrait == PersonalityTrait::PACIFIST) {
                    speedMultiplier = 0.80f; 
                }

                citizen.harvestTimer += deltaTime * speedMultiplier;
                float harvestDuration = std::max(1.0f, 4.0f - (citizen.dna.baseStrength * 0.1f));
                
                if (citizen.harvestTimer >= harvestDuration) {
                    int targetedBlock = world.getBlock(citizen.targetBlockPos.x, citizen.targetBlockPos.y, citizen.targetBlockPos.z);
                    
                    if (targetedBlock != 0) {
                        // Destroy block in simulation matrix
                        world.setBlock(citizen.targetBlockPos.x, citizen.targetBlockPos.y, citizen.targetBlockPos.z, 0);
                        
                        // Calculate Yield based on stats and traits
                        int finalYield = 1 + (citizen.dna.baseStrength / 6);
                        if (citizen.dna.primaryTrait == PersonalityTrait::GREEDY) {
                            finalYield *= 2;
                        }

                        // Save data temporarily to the inventory slots of the individual agent
                        citizen.heldResourceAmount = finalYield;
                        citizen.heldResourceType = (targetedBlock == 1) ? ResourceType::WOOD : ResourceType::STONE;

                        std::cout << "⚒️ " << citizen.name << " harvested " << finalYield 
                                  << "x " << ((targetedBlock == 1) ? "Wood" : "Stone") << "." << std::endl;

                        // Log substantial drops to chronological database registers
                        if (finalYield >= 3) {
                            ChronicleDatabase::logEvent(EventType::INDUSTRIAL, citizen.name + " collected an abundant harvest windfall.");
                        }

                        // Path back home to deposit
                        glm::ivec3 dropoff = ZoneManager::getNearestZoneCenter(citizen.position, ZoneType::STOCKPILE, citizen.factionId);
                        if (dropoff.x != -1) {
                            float jX = ((std::rand() % 3) - 1) * 0.5f;
                            float jZ = ((std::rand() % 3) - 1) * 0.5f;
                            glm::vec3 depositTarget(dropoff.x + 0.5f + jX, dropoff.y, dropoff.z + 0.5f + jZ);

                            citizen.currentPath = Pathfinding::findPath(citizen.position, depositTarget, world);
                            if (!citizen.currentPath.empty()) {
                                citizen.currentPathIndex = 0;
                                citizen.targetDestination = citizen.currentPath[0];
                                citizen.hasTarget = true;
                                citizen.currentState = AIState::RETURNING_TO_STOCKPILE; 
                                break; 
                            }
                        }
                    }
                    // Fallback if zone or path checks fail
                    citizen.currentState = AIState::WANDERING;
                }
                break;
            }

            case AIState::RETURNING_TO_STOCKPILE: {
                if (!citizen.hasTarget) {
                    // Hand off items to the global vault structure safely
                    ResourceManager::addResource(citizen.heldResourceType, citizen.heldResourceAmount);
                    
                    std::string resName = (citizen.heldResourceType == ResourceType::WOOD) ? "Wood" : "Stone";
                    std::cout << "📦 " << citizen.name << " safely delivered " << citizen.heldResourceAmount 
                              << " " << resName << " to the faction stockpile." << std::endl;
                    
                    // Clear inventory state completely
                    citizen.heldResourceAmount = 0;
                    citizen.currentState = AIState::WANDERING;
                }
                break;
            }
        } 

        // 3. STANDARD PATHWAY STEPPER MOTOR
        if (citizen.hasTarget) {
            glm::vec3 direction = citizen.targetDestination - citizen.position;
            float distanceToNode = glm::length(direction);
            float walkSpeed = 3.5f;

            if (distanceToNode > 0.1f) {
                citizen.position += glm::normalize(direction) * walkSpeed * deltaTime;
            } else {
                citizen.currentPathIndex++;
                if (citizen.currentPathIndex < citizen.currentPath.size()) {
                    citizen.targetDestination = citizen.currentPath[citizen.currentPathIndex];
                } else {
                    citizen.hasTarget = false;
                    citizen.currentPath.clear();
                }
            }
        } 
        
    } 
}

void EntityManager::renderEntities(unsigned int shaderProgram, unsigned int modelLoc, unsigned int overrideColorLoc) {
    if (activeCitizens.empty()) return;

    glUseProgram(shaderProgram);
    glDisableVertexAttribArray(1);
    glBindVertexArray(citizenVAO);

    for (const auto& citizen : activeCitizens) {
        // Query faction color scheme instead of pure random hair RGB values
        Faction* myFaction = FactionManager::getFaction(citizen.factionId);
        if (myFaction != nullptr) {
            glUniform3f(overrideColorLoc, myFaction->bannerColor.r, myFaction->bannerColor.g, myFaction->bannerColor.b);
        } else {
            glUniform3f(overrideColorLoc, citizen.dna.hairColorR, citizen.dna.hairColorG, citizen.dna.hairColorB);
        }

        float h = citizen.dna.heightModifier; 
        glm::vec3 modelScale(0.7f * h, 1.8f * h, 0.7f * h); 

        glm::vec3 renderPos(citizen.position.x, citizen.position.y + modelScale.y / 2.0f, citizen.position.z);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), renderPos);
        model = glm::scale(model, modelScale);

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, citizenVertexCount);
    }

    glUniform3f(overrideColorLoc, -1.0f, -1.0f, -1.0f);
    glBindVertexArray(0);
    glEnableVertexAttribArray(1);
}

void EntityManager::debugPrintSettlement() {
    std::cout << "\n===============================\n";
    std::cout << "    SETTLEMENT POPULATION ROSTER\n";
    std::cout << "===============================\n";
    for (const auto& citizen : activeCitizens) {
        int hungerPct = static_cast<int>(citizen.hunger);
        Faction* fac = FactionManager::getFaction(citizen.factionId);
        std::string facName = fac ? fac->name : "Unknown";

        std::cout << "ID: " << citizen.id << " | Name: " << citizen.name 
                  << " | Faction: " << facName
                  << " | Hunger: " << hungerPct << "% | Position: (" 
                  << (int)citizen.position.x << ", " << (int)citizen.position.y << ", " << (int)citizen.position.z << ")\n";
    }
    std::cout << "===============================\n\n";
}