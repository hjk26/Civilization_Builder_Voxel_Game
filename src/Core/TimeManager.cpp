#include "TimeManager.h"
#include <iostream>

// Initialize static variables
float TimeManager::totalGameTime = 0.0f;
int TimeManager::currentDay = 1;
float TimeManager::hourOfDay = 6.0f; // Start at 6:00 AM (Sunrise)
float TimeManager::dayLengthInSeconds = 120.0f; // A full day lasts 2 minutes real-world
float TimeManager::tickTimer = 0.0f;
bool TimeManager::isTickThisFrame = false;

void TimeManager::update(float deltaTime) {
    totalGameTime += deltaTime;
    isTickThisFrame = false;

    // Advance the day clock
    hourOfDay += (deltaTime / dayLengthInSeconds) * 24.0f;
    if (hourOfDay >= 24.0f) {
        hourOfDay -= 24.0f;
        currentDay++;
        std::cout << "☀️ Day " << currentDay << " has arrived!" << std::endl;
    }

    // Handle the fixed interval simulation pulse (once every 1 second)
    tickTimer += deltaTime;
    if (tickTimer >= 1.0f) {
        tickTimer -= 1.0f;
        isTickThisFrame = true;
    }
}

float TimeManager::getSkyFactor() {
    // Maps time into a sine wave oscillating between 0.0 (Midnight) and 1.0 (Noon)
    // Shift by 6 hours so peak brightness hits at 12:00 PM precisely
    float angle = ((hourOfDay - 6.0f) / 24.0f) * 2.0f * 3.141592f;
    float sinValue = std::sin(angle); // Ranges from -1.0 to 1.0
    
    return (sinValue + 1.0f) * 0.5f;   // Rescale to clean 0.0f -> 1.0f range
}

glm::vec3 TimeManager::getDynamicSkyColor() {
    float factor = getSkyFactor();
    
    // Sky colors: Day = Light Blue, Night = Dark Space Navy Blue
    glm::vec3 daySky(0.5f, 0.8f, 0.9f);
    glm::vec3 nightSky(0.05f, 0.05f, 0.15f);
    
    // Smoothly mix the vectors together based on time factor
    return glm::mix(nightSky, daySky, factor);
}