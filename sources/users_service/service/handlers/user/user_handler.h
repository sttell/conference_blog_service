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

        void HandleUserRoleUpdateRequest(HTTPServerRequest& request, HTTPServerResponse& response);

        std::optional<std::string> AuthRequest(HTTPServerRequest& request, HTTPServerResponse& response);

    };

} // namespace handler

#endif //SERVER_USER_HANDLER_H
