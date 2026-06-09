#include "FactionManager.h"
#include <iostream>

std::vector<Faction> FactionManager::activeFactions;
unsigned int FactionManager::nextFactionId = 1;

void FactionManager::createFaction(std::string name, glm::vec3 color) {
    Faction newFaction;
    newFaction.id = nextFactionId++;
    newFaction.name = name;
    newFaction.bannerColor = color;
    
    activeFactions.push_back(newFaction);
    std::cout << "🚩 New Faction Established: " << name << " has claimed a stake in the world." << std::endl;
}

Faction* FactionManager::getFaction(unsigned int factionId) {
    for (auto& faction : activeFactions) {
        if (faction.id == factionId) return &faction;
    }
    return nullptr;
}

Alignment FactionManager::getRelationship(unsigned int factionA, unsigned int factionB) {
    if (factionA == factionB) return Alignment::ALLIED;
    return Alignment::NEUTRAL; // Default state for now; easily upgraded to a lookup matrix later!
}