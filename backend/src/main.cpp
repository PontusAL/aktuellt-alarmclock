#include "crow.h"
#include "api_response.hpp"

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

int main() {
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