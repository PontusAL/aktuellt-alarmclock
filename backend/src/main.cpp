#include "crow.h"
#include "api_response.hpp"

#include <cstdio>
#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>

// Validation helpers for requests

bool isValidTimeString(const std::string& value) {
    std::regex timeRegex("^([01][0-9]|2[0-3]):[0-5][0-9]$");
    return std::regex_match(value, timeRegex);
}

void validateScheduleJson(const crow::json::rvalue& json) {
    if (json.has("enabled") && json["enabled"].t() != crow::json::type::False && json["enabled"].t() != crow::json::type::True) {
        throw std::runtime_error("enabled must be a boolean");
    }
    if (json.has("alarm_time")) {
        std::string alarmTime = std::string(json["alarm_time"].s());
        if (!isValidTimeString(alarmTime)) {
            throw std::runtime_error("alarm_time must use HH:MM format");
        }
    }
    if (json.has("playback_time")) {
        std::string playbackTime = std::string(json["playback_time"].s());
        if (!isValidTimeString(playbackTime)) {
            throw std::runtime_error("playback_time must use HH:MM format");
        }
    }
    if (json.has("download_minutes_before")) {
        int minutes = json["download_minutes_before"].i();
        if (minutes < 0 || minutes > 180) {
            throw std::runtime_error("download_minutes_before must be between 0 and 180");
        }
    }
    if (json.has("retain_files")) {
        int retainFiles = json["retain_files"].i();
        if (retainFiles < 1 || retainFiles > 30) {
            throw std::runtime_error("retain_files must be between 1 and 30");
        }
    }
    if (json.has("timezone")) {
        std::string timezone = std::string(json["timezone"].s());
        if (!isValidTimezone(timezone)) {
            throw std::runtime_error("timezone is not supported");
        }
    }
}



crow::json::wvalue makeHealthData() {
    crow::json::wvalue data;
    data["status"] = "ok";
    return data;
}

std::string readFile(const std::string& path){
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

crow::json::wvalue readScheduleConfig(const std::string& configPath) {
    std::string fileContents = readFile(configPath);
    auto parsed = crow::json::load(fileContents);

    if (!parsed) {

        throw std::runtime_error("Config file is not valid JSON");

    }
    crow::json::wvalue schedule;
    schedule["enabled"] = parsed["enabled"].b();
    schedule["alarm_time"] = std::string(parsed["alarm_time"].s());
    schedule["playback_time"] = std::string(parsed["playback_time"].s());
    schedule["download_minutes_before"] = parsed["download_minutes_before"].i();
    schedule["timezone"] = std::string(parsed["timezone"].s());
    schedule["retain_files"] = parsed["retain_files"].i();
    return schedule;
}

int main(int argc, char* argv[]){

    std::string configPath = "../../config.json";
    if (argc > 1) {
        configPath = argv[1];
    }
    crow::SimpleApp app;

    CROW_ROUTE(app, "/api/health")
    ([] {
        return makeSuccessResponse(makeHealthData(), "Backend is healthy");
    });

    CROW_ROUTE(app, "/api/schedule")
        .methods(crow::HTTPMethod::GET, crow::HTTPMethod::PUT)
    ([configPath](const crow::request& req) {
        if (req.method == crow::HTTPMethod::GET) {
            try {
                crow::json::wvalue schedule = readScheduleConfig(configPath);
                return makeSuccessResponse(std::move(schedule), "Schedule retrieved successfully");
            } catch (const std::exception& error) {
                return makeErrorResponse(error.what());
            }
        }
        if (req.method == crow::HTTPMethod::PUT) {
            return makeErrorResponse("Schedule update is not implemented yet");
        }
        return makeErrorResponse("Unsupported method");
    });




    app.bindaddr("0.0.0.0")
        .port(8080)
        .multithreaded()
        .run();

    return 0;
}