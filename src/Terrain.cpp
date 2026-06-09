#include "Terrain.h"

TerrainSystem::TerrainSystem() {
    // Large structural shapes (Where oceans, plains, and mountains go)
    m_continentalNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_continentalNoise.SetFrequency(0.0005f);

    // Mountain structures (Jagged peaks)
    m_peaksNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_peaksNoise.SetFrequency(0.003f);

    // Fine details (Soil roughness, small bumps)
    m_detailNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_detailNoise.SetFrequency(0.01f); 

    // 3D Cave generation mapping
    m_caveNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_caveNoise.SetFrequency(0.015f);

    // Biome drivers
    m_temperatureNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_temperatureNoise.SetFrequency(0.0004f);

    m_humidityNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_humidityNoise.SetFrequency(0.0004f);
}

void TerrainSystem::initialize(int seed) {
    m_continentalNoise.SetSeed(seed);
    m_peaksNoise.SetSeed(seed + 1);
    m_detailNoise.SetSeed(seed + 2);
    m_caveNoise.SetSeed(seed + 3);
    m_temperatureNoise.SetSeed(seed + 4);
    m_humidityNoise.SetSeed(seed + 5);
}

BiomeType TerrainSystem::calculateBiome(float temp, float humidity, float elevation) {
    // High elevations automatically become harsh mountain environments
    if (elevation > 0.75f) return BiomeType::EXTREME_PEAKS;
    if (elevation > 0.55f) return BiomeType::SNOW_MOUNTAIN;
    
    // Lowland biome matrix
    if (temp > 0.65f && humidity < 0.35f) return BiomeType::DESERT;
    if (humidity > 0.65f) return BiomeType::FOREST;
    
    return BiomeType::PLAINS;
}

VoxelTerrainData TerrainSystem::getTerrainData(int worldX, int worldZ) {
    VoxelTerrainData data;
    float fx = static_cast<float>(worldX);
    float fz = static_cast<float>(worldZ);

    // Get baseline noise values [-1, 1] normalized to [0, 1]
    float continental = (m_continentalNoise.GetNoise(fx, fz) + 1.0f) * 0.5f;
    float peaks = (m_peaksNoise.GetNoise(fx, fz) + 1.0f) * 0.5f;
    float detail = m_detailNoise.GetNoise(fx, fz); 

    float temp = (m_temperatureNoise.GetNoise(fx, fz) + 1.0f) * 0.5f;
    float humidity = (m_humidityNoise.GetNoise(fx, fz) + 1.0f) * 0.5f;

    // Determine basic biome type
    data.biome = calculateBiome(temp, humidity, continental);

    float baseHeight = 40.0f; 
    float heightVariance = 0.0f;

    switch (data.biome) {
        case BiomeType::DESERT: {
            baseHeight = 35.0f;
            // GEOLOGICAL FORMATION: Badlands/Canyon Terracing (Layered horizontal steps)
            // Splitting continuous noise into mathematical steps simulates millions of years of rock layer erosion.
            float duneNoise = peaks * 12.0f + detail * 1.5f;
            float terracedNoise = std::floor(duneNoise / 3.0f) * 3.0f; // Snaps heights to flat intervals
            heightVariance = glm::mix(duneNoise, terracedNoise, 0.7f); // Blend to keep some slope roughness
            break;
        }
        case BiomeType::PLAINS:
            baseHeight = 38.0f;
            // GEOLOGICAL FORMATION: Flat Sediment Basins
            // Squashing the fine detail noise gives a gentle rolling meadow vibe.
            heightVariance = detail * 2.5f; 
            break;

        case BiomeType::FOREST:
            baseHeight = 42.0f;
            // GEOLOGICAL FORMATION: Weathered Foothills
            // Exponential curves make valley bottoms flat and hill crests rounded.
            heightVariance = std::pow(peaks, 1.8f) * 14.0f + (detail * 2.0f);
            break;

        case BiomeType::SNOW_MOUNTAIN: {
            baseHeight = 52.0f;
            // GEOLOGICAL FORMATION: Glaciated Ridges
            // Inverting the absolute noise value (|noise|) flips valleys into razor-sharp alpine ridges.
            float ridge = 1.0f - std::abs(m_peaksNoise.GetNoise(fx * 1.5f, fz * 1.5f));
            heightVariance = std::pow(ridge, 2.0f) * 28.0f + (detail * 3.0f);
            break;
        }
        case BiomeType::EXTREME_PEAKS: {
            baseHeight = 60.0f;
            // GEOLOGICAL FORMATION: Tectonic Uplift Horns
            // Compounding multiple layered sharp ridges together creates steep, jagged Swiss Alps-style peaks.
            float ridge1 = 1.0f - std::abs(m_peaksNoise.GetNoise(fx * 2.0f, fz * 2.0f));
            float ridge2 = 1.0f - std::abs(m_detailNoise.GetNoise(fx * 4.0f, fz * 4.0f));
            float combinedMountain = (ridge1 * 0.7f) + (ridge2 * 0.3f);
            heightVariance = std::pow(combinedMountain, 3.0f) * 55.0f;
            break;
        }
    }

    // Combine everything into final block calculations
    data.height = static_cast<int>(baseHeight + heightVariance);
    data.isRiver = false; 

    return data;
}

bool TerrainSystem::isCave(int worldX, int worldY, int worldZ, int surfaceHeight) {
    if (worldY > surfaceHeight - 5) return false; // Soil layer protection

    float cx = static_cast<float>(worldX);
    float cy = static_cast<float>(worldY);
    float cz = static_cast<float>(worldZ);

    // GEOLOGICAL FORMATION: Subterranean Karst Tube Networks
    // Checking if a 3D noise value hits a tiny threshold corridor generates twisting, hollow worm tunnels.
    float tunnelNoise = m_caveNoise.GetNoise(cx * 1.2f, cy * 2.0f, cz * 1.2f);
    
    // Narrow bounds mean the tunnel has walls, roofs, and bases instead of a massive open void
    return (tunnelNoise > 0.42f && tunnelNoise < 0.48f);
}