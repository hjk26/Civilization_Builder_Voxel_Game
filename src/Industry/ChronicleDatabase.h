#ifndef CHRONICLE_DATABASE_H
#define CHRONICLE_DATABASE_H

#include <string>
#include <vector>

enum class EventType {
    POPULATION,
    INDUSTRIAL,
    FINANCIAL,
    DIPLOMATIC
};

struct HistoricalEvent {
    unsigned int dayStamp;
    EventType category;
    std::string textDescription;
};

class ChronicleDatabase {
public:
    static std::vector<HistoricalEvent> archive;

    static void logEvent(EventType category, std::string details);
    static void dumpChroniclesToConsole();
};

#endif // CHRONICLE_DATABASE_H