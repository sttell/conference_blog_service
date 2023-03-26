#ifndef SERVER_AUTH_HANDLER_H
#define SERVER_AUTH_HANDLER_H

#include "../interface/i_request_handler.h"

namespace handler {

    class AuthHandler : public IRequestHandler {
    public:
        explicit AuthHandler(const std::string& format);
        ~AuthHandler() override = default;
    public:

        void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override;

    private:

        void HandleGetRequest(HTTPServerRequest& request, HTTPServerResponse& response);

    private:

        void SetBadRequestResponse(HTTPServerResponse& response, const std::string& description);

        void SetNotFoundResponse(HTTPServerResponse& response, const std::string& description);

        void SetInternalErrorResponse(HTTPServerResponse& response, const std::string& description);

    };

} // namespace handler

#endif //SERVER_AUTH_HANDLER_H
