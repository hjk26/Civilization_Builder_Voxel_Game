#ifndef DNA_H
#define DNA_H

#include <string>
#include <cstdint>
#include <iostream>  // <--- ADD THIS FOR std::cout and std::endl
#include <ios>       // <--- ADD THIS FOR std::hex and std::dec

// Visual Styles for the Anime Voxel Meshes
enum class HairStyle    : uint8_t { SHORT_SPIKY, TWIN_TAILS, LONG_FLOWING, BOB_CUT, PON_TAIL };
enum class EyeShape     : uint8_t { LARGE_ANIME, DROOPY, SHARP, CLOSED_SMILE };
enum class OutfitType   : uint8_t { LABOURER_TUNIC, GUILD_ROBES, NOBLE_DRESS, SOLDIER_ARMOR };

// Behavioral Traits that impact the simulation loops
enum class PersonalityTrait : uint8_t {
    AMBITIOUS,  // Tends to demand promotions or start factions
    INDUSTRIOUS,// Works 20% faster at machines
    GREEDY,     // Consumes more gold/tax resources
    PACIFIST,   // Refuses to join military squads, but high happiness stability
    SCHEMING    // Prone to criminal or underground syndicate operations
};

struct CitizenDNA {
    // Unique 64-bit genetic seed representing the character's entire code
    uint64_t geneticSeed;

    // --- Visual Phenotypes (Decoded from Seed) ---
    float heightModifier;         // Scale factor for rendering (e.g., 0.85f to 1.15f)
    float hairColorR, hairColorG, hairColorB;
    float eyeColorR,  eyeColorG,  eyeColorB;
    HairStyle hairStyle;
    EyeShape eyeShape;
    OutfitType outfit;

    // --- Personality & Attributes (Decoded from Seed) ---
    PersonalityTrait primaryTrait;
    int baseIntelligence;         // Determines research speed
    int baseStrength;             // Determines mining/carrying capacity

    // Family Lineage Markers
    uint32_t familyId;
    uint8_t generation;

    // Decodes a raw 64-bit number into structured citizen traits
    static CitizenDNA GenerateFromSeed(uint64_t seed, uint32_t family = 0, uint8_t gen = 1) {
        CitizenDNA dna;
        dna.geneticSeed = seed;
        dna.familyId = family;
        dna.generation = gen;

        // Extract height modifier using bits 0-7 (Map byte 0-255 to a clean float scale)
        uint8_t heightByte = (seed & 0xFF);
        dna.heightModifier = 0.85f + ((float)heightByte / 255.0f) * 0.30f;

        // Extract Hair Color using bits 8-31 (Standard 24-bit RGB mapping)
        dna.hairColorR = (float)((seed >> 8)  & 0xFF) / 255.0f;
        dna.hairColorG = (float)((seed >> 16) & 0xFF) / 255.0f;
        dna.hairColorB = (float)((seed >> 24) & 0xFF) / 255.0f;

        // Extract Eye Color using bits 32-39 (Use a step multiplier to keep anime expressions sharp)
        dna.eyeColorR = (float)((seed >> 32) & 0x0F) / 15.0f;
        dna.eyeColorG = (float)((seed >> 36) & 0x0F) / 15.0f;
        dna.eyeColorB = (float)((seed >> 40) & 0x0F) / 15.0f;

        // Extract Enum Index Styles using bits 44-47
        dna.hairStyle = static_cast<HairStyle>((seed >> 44) % 5);
        dna.eyeShape  = static_cast<EyeShape> ((seed >> 46) % 4);
        dna.outfit    = static_cast<OutfitType>((seed >> 48) % 4);

        // Extract Personality Attributes using bits 52-63
        dna.primaryTrait = static_cast<PersonalityTrait>((seed >> 52) % 5);
        dna.baseIntelligence = 5 + ((seed >> 56) & 0x0F); // Range 5-20
        dna.baseStrength     = 5 + ((seed >> 60) & 0x0F); // Range 5-20

        return dna;
    }

    // Helper to print out their identity breakdown to the console
    void debugPrint() const {
        std::string traits[] = { "Ambitious", "Industrious", "Greedy", "Pacifist", "Scheming" };
        std::string outfits[] = { "Labourer Tunic", "Guild Robes", "Noble Dress", "Soldier Armor" };
        
        std::cout << "--- Citizen DNA Decoded [Seed: 0x" << std::hex << geneticSeed << std::dec << "] ---" << std::endl;
        std::cout << "Height Scale: " << heightModifier << "x" << std::endl;
        std::cout << "Hair RGB:    (" << hairColorR << ", " << hairColorG << ", " << hairColorB << ")" << std::endl;
        std::cout << "Outfit Type: " << outfits[static_cast<int>(outfit)] << std::endl;
        std::cout << "Personality: " << traits[static_cast<int>(primaryTrait)] << std::endl;
        std::cout << "Stats:       INT: " << baseIntelligence << " | STR: " << baseStrength << std::endl;
        std::cout << "Lineage:     Family ID: " << familyId << " | Gen: " << (int)generation << std::endl;
        std::cout << "------------------------------------------" << std::endl;
    }
};

#endif // DNA_H