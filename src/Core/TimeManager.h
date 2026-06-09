#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <glm/glm.hpp>

class TimeManager {
public:
    static float totalGameTime;    // Monotonically increasing time tracker
    static int currentDay;         // Tracks calendar day count
    static float hourOfDay;        // Float ranging from 0.0f to 24.0f
    
    static float dayLengthInSeconds; // Real-world seconds per full day-night loop

    // Core execution hooks
    static void update(float deltaTime);
    
    // Shader Utilities
    static float getSkyFactor();        // Returns 0.0f (Night) to 1.0f (Midday)
    static glm::vec3 getDynamicSkyColor(); // Linearly interpolates a nice hex skybox tone
    
    // Simulation Pulse Checking
    static bool isTickThisFrame;   // True exactly once every logic second
private:
    static float tickTimer;
};

#endif // TIME_MANAGER_H