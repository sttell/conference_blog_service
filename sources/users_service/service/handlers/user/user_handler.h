#ifndef SERVER_USER_HANDLER_H
#define SERVER_USER_HANDLER_H

#include "../interface/i_request_handler.h"

namespace handler {

    class UserHandler : public IRequestHandler {
    public:
        explicit UserHandler(const std::string& format);
        ~UserHandler() override = default;

    public:

        void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override;

    private:

        void HandleUserPostRequest(HTTPServerRequest& request, HTTPServerResponse& response);

        void HandleUserGetRequest(HTTPServerRequest& request, HTTPServerResponse& response);

    private:
        void SetBadRequestResponse(HTTPServerResponse& response, const std::string& description);

        void SetUnauthorizedResponse(HTTPServerResponse& response, const std::string& description);

        void SetPermissionDeniedResponse(HTTPServerResponse& response, const std::string& description);

        void SetNotFoundResponse(HTTPServerResponse& response, const std::string& description);

        void SetNotAcceptableResponse(HTTPServerResponse& response, const std::string& description);

        void SetInternalErrorResponse(HTTPServerResponse& response, const std::string& description);


    };

} // namespace handler

#endif //SERVER_USER_HANDLER_H
