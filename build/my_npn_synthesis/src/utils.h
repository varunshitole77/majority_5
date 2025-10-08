#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <chrono>
#include <cstdint>

class Utils {
public:
    // File operations
    static void ensureDir(const std::string& directory);
    static void saveToTxt(const std::string& data, const std::string& filepath);
    static std::string loadFromTxt(const std::string& filepath);
    
    // Formatting
    static std::string formatBinary(uint64_t value, int width);
    
    // Logging
    static void logMessage(const std::string& message, const std::string& logfile = "logs/synthesis.log");
    
    // Timer utility
    class Timer {
    private:
        std::chrono::high_resolution_clock::time_point start_time;
        std::string name;
    public:
        Timer(const std::string& timer_name);
        ~Timer();
        double elapsed() const;
    };
};

#endif // UTILS_H
