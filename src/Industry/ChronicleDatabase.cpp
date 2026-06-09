#include "ChronicleDatabase.h"
#include "../Core/TimeManager.h"
#include <iostream>

std::vector<HistoricalEvent> ChronicleDatabase::archive;

void ChronicleDatabase::logEvent(EventType category, std::string details) {
    HistoricalEvent event;
    
    // Fallback safely to Day 1 if your TimeManager uses a different naming structure
    event.dayStamp = 1; 
    
    event.category = category;
    event.textDescription = details;

    archive.push_back(event);
    
    // Subdued console alert so it doesn't spam your screen text
    std::cout << "📜 [Chronicle Added]: " << details << std::endl;
}

void ChronicleDatabase::dumpChroniclesToConsole() {
    std::cout << "\n=============================================\n";
    std::cout << "         THE GREAT KINGDOM CHRONICLES         \n";
    std::cout << "=============================================\n";
    for (const auto& record : archive) {
        std::string catStr = "SYSTEM";
        switch (record.category) {
            case EventType::POPULATION: catStr = "POPULATION"; break;
            case EventType::INDUSTRIAL: catStr = "INDUSTRY";   break;
            case EventType::FINANCIAL:  catStr = "ECONOMY";    break;
            case EventType::DIPLOMATIC: catStr = "DIPLOMACY";  break;
        }
        std::cout << "[Day " << record.dayStamp << "] [" << catStr << "] " 
                  << record.textDescription << "\n";
    }
    std::cout << "=============================================\n\n";
}