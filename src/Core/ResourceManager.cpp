#include "ResourceManager.h"
#include <iostream>

// Initialize static members at zero/starting quantities
int ResourceManager::woodCount = 50;   // Give the player a small starting buffer
int ResourceManager::stoneCount = 20;
int ResourceManager::foodCount = 100;
int ResourceManager::waterCount = 100;
int ResourceManager::goldCount = 0;
int ResourceManager::metallurgyTier = 1;

void ResourceManager::addResource(ResourceType type, int amount) {
    if (amount <= 0) return;

    switch (type) {
        case ResourceType::WOOD:  woodCount += amount;  break;
        case ResourceType::STONE: stoneCount += amount; break;
        case ResourceType::FOOD:  foodCount += amount;  break;
        case ResourceType::WATER: waterCount += amount; break;
        case ResourceType::GOLD:  goldCount += amount;  break;
        case ResourceType::METALLURGY_TIER: metallurgyTier += amount; break;
    }
}

bool ResourceManager::spendResource(ResourceType type, int amount) {
    if (amount <= 0) return false;

    switch (type) {
        case ResourceType::WOOD:
            if (woodCount >= amount) { woodCount -= amount; return true; }
            break;
        case ResourceType::STONE:
            if (stoneCount >= amount) { stoneCount -= amount; return true; }
            break;
        case ResourceType::FOOD:
            if (foodCount >= amount) { foodCount -= amount; return true; }
            break;
        case ResourceType::WATER:
            if (waterCount >= amount) { waterCount -= amount; return true; }
            break;
        case ResourceType::GOLD:
            if (goldCount >= amount) { goldCount -= amount; return true; }
            break;
        default:
            return false;
    }
    return false; // Insufficient resources
}

int ResourceManager::getResourceCount(ResourceType type) {
    switch (type) {
        case ResourceType::WOOD:  return woodCount;
        case ResourceType::STONE: return stoneCount;
        case ResourceType::FOOD:  return foodCount;
        case ResourceType::WATER: return waterCount;
        case ResourceType::GOLD:  return goldCount;
        case ResourceType::METALLURGY_TIER: return metallurgyTier;
    }
    return 0;
}

void ResourceManager::printStockpile() {
    std::cout << "--- Kingdom Stockpile ---" << std::endl;
    std::cout << "Wood:  " << woodCount  << " | Stone: " << stoneCount << std::endl;
    std::cout << "Food:  " << foodCount  << " | Water: " << waterCount << std::endl;
    std::cout << "Gold:  " << goldCount  << " | Metal Tier: " << metallurgyTier << std::endl;
    std::cout << "-------------------------" << std::endl;
}