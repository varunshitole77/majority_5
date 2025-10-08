#include "utils.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <unistd.h>
#endif

void Utils::ensureDir(const std::string& directory) {
    // Create directory recursively
    size_t pos = 0;
    do {
        pos = directory.find_first_of("/\\", pos + 1);
        std::string subdir = directory.substr(0, pos);
        if (!subdir.empty()) {
            struct stat st;
            if (stat(subdir.c_str(), &st) != 0) {
                mkdir(subdir.c_str(), 0755);
            }
        }
    } while (pos != std::string::npos);
}

void Utils::saveToTxt(const std::string& data, const std::string& filepath) {
    // Extract directory path
    size_t last_slash = filepath.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        std::string dir = filepath.substr(0, last_slash);
        ensureDir(dir);
    }
    
    std::ofstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for writing: " + filepath);
    }
    file << data;
    file.close();
    std::cout << "Saved to " << filepath << std::endl;
}

std::string Utils::loadFromTxt(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("File not found: " + filepath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string Utils::formatBinary(uint64_t value, int width) {
    std::string result;
    for (int i = width - 1; i >= 0; --i) {
        result += ((value >> i) & 1) ? '1' : '0';
    }
    return result;
}

void Utils::logMessage(const std::string& message, const std::string& logfile) {
    // Extract directory path
    size_t last_slash = logfile.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        std::string dir = logfile.substr(0, last_slash);
        ensureDir(dir);
    }
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::ofstream log(logfile, std::ios::app);
    if (log.is_open()) {
        struct tm* timeinfo = std::localtime(&time_t);
        log << "[" << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S") << "] " 
            << message << std::endl;
    }
    
    std::cout << message << std::endl;
}

Utils::Timer::Timer(const std::string& timer_name) : name(timer_name) {
    start_time = std::chrono::high_resolution_clock::now();
}

Utils::Timer::~Timer() {
    double elapsed_time = elapsed();
    std::cout << name << " took " << elapsed_time << " seconds" << std::endl;
}

double Utils::Timer::elapsed() const {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    return duration.count() / 1000000.0;
}