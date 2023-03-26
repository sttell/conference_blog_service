#ifndef SERVER_SEARCH_HANDLER_H
#define SERVER_SEARCH_HANDLER_H

#include "../interface/i_request_handler.h"

namespace handler {

    class SearchHandler : public IRequestHandler {
    public:
        explicit SearchHandler(const std::string& format);
        ~SearchHandler() override = default;

    public:
        void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override;

    private:

        void HandleGetRequest(HTTPServerRequest& request, HTTPServerResponse& response);

    private:
        void SetBadRequestResponse(HTTPServerResponse& response, const std::string& description);

        void SetUnauthorizedResponse(HTTPServerResponse& response, const std::string& description);

        void SetPermissionDeniedResponse(HTTPServerResponse& response, const std::string& description);

        void SetNotFoundResponse(HTTPServerResponse& response, const std::string& description);

        void SetInternalErrorResponse(HTTPServerResponse& response, const std::string& description);

    };

} // namespace handler

#endif //SERVER_SEARCH_HANDLER_H
