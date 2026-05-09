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

void writeFileAtomically(const std::string& path, const std::string& contents) {
    std::string tempPath = path + ".tmp";
    {
        std::ofstream file(tempPath);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open temporary config file for writing");
        }
        file << contents;
        if (!file.good()) {
            throw std::runtime_error("Failed while writing temporary config file");
        }
    }
    if (std::rename(tempPath.c_str(), path.c_str()) != 0) {
        throw std::runtime_error("Could not replace config file");
    }
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

crow::json::wvalue updateScheduleConfig(
    const std::string& configPath,
    const std::string& requestBody
) {
    auto incoming = crow::json::load(requestBody);
    if (!incoming) {
        throw std::runtime_error("Request body is not valid JSON");
    }
    validateScheduleJson(incoming);
    std::string existingContents = readFile(configPath);
    auto existing = crow::json::load(existingContents);

    if (!existing) {
        throw std::runtime_error("Existing config file is not valid JSON");
    }
    crow::json::wvalue updated;

    updated["enabled"] = existing["enabled"].b();
    updated["alarm_time"] = std::string(existing["alarm_time"].s());
    updated["playback_time"] = std::string(existing["playback_time"].s());
    updated["download_minutes_before"] = existing["download_minutes_before"].i();
    updated["timezone"] = std::string(existing["timezone"].s());
    updated["retain_files"] = existing["retain_files"].i();

    if (incoming.has("enabled")) {
        updated["enabled"] = incoming["enabled"].b();
    }
    if (incoming.has("alarm_time")) {
        updated["alarm_time"] = std::string(incoming["alarm_time"].s());
    }
    if (incoming.has("playback_time")) {
        updated["playback_time"] = std::string(incoming["playback_time"].s());
    }
    if (incoming.has("download_minutes_before")) {
        updated["download_minutes_before"] = incoming["download_minutes_before"].i();
    }
    if (incoming.has("timezone")) {
        updated["timezone"] = std::string(incoming["timezone"].s());
    }
    if (incoming.has("retain_files")) {
        updated["retain_files"] = incoming["retain_files"].i();
    }
    writeFileAtomically(configPath, updated.dump());

    return updated;
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
            try {
                crow::json::wvalue schedule = updateScheduleConfig(configPath, req.body);
                return makeSuccessResponse(std::move(schedule), "Schedule updated successfully");
            } catch (const std::exception& error) {
                return makeErrorResponse(error.what());
            }
        }
        return makeErrorResponse("Unsupported method");
    });




    app.bindaddr("0.0.0.0")
        .port(8080)
        .multithreaded()
        .run();

    return 0;
}