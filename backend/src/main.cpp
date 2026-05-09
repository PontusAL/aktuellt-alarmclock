#include "crow.h"
#include "api_response.hpp"

crow::json::wvalue makeHealthData()
{
    crow::json::wvalue data;
    data["status"] = "ok";
    return data;
}

int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/api/health")
    ([] {
        return makeSuccessResponse(makeHealthData(), "Backend is healthy");
    });

    app.bindaddr("0.0.0.0")
        .port(8080)
        .multithreaded()
        .run();

    return 0;
}