#pragma once

#include "crow.h"
#include <string>

inline crow::json::wvalue makeSuccessResponse(
    const crow::json::wvalue& data,
    const std::string& message
) {
    crow::json::wvalue response;
    response["success"] = true;
    response["data"] = data;
    response["message"] = message;
    return response;
}

inline crow::json::wvalue makeErrorResponse(
    const std::string& message
) {
    crow::json::wvalue response;
    response["success"] = false;
    response["data"] = nullptr;
    response["message"] = message;
    return response;
}