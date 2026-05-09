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

    CROW_ROUTE(app, "/api/schedule").methods(crow::HTTPMethod::GET, crow::HTTPMethod::PUT)
    ([] {
        if (crow::request::method() == crow::HTTPMethod::GET) {
            return makeErrorResponse("Schedule retrieval is not implemented yet");
        } else if (crow::request::method() == crow::HTTPMethod::PUT) {
            return makeErrorResponse("Schedule update is not implemented yet");
        }
    });




    app.bindaddr("0.0.0.0")
        .port(8080)
        .multithreaded()
        .run();

    return 0;
}