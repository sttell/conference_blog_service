#ifndef SERVER_I_REQUEST_HANDLER_H
#define SERVER_I_REQUEST_HANDLER_H

#include "handler_type.h"

#include <optional>

#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;

namespace handler {

    class IRequestHandler : public HTTPRequestHandler {
    public:
        IRequestHandler(std::string format, HandlerType type, std::string instance_name);

    public:

        virtual void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) = 0;

        [[nodiscard]] HandlerType GetType() const noexcept;

    protected:

        std::string& Instance() noexcept;

        std::string& GetFormat() noexcept;

        /* 400 */
        void SetBadRequestResponse(HTTPServerResponse& response, const std::string& description);

        /* 401 */
        void SetUnauthorizedResponse(HTTPServerResponse& response, const std::string& description);

        /* 403 */
        void SetPermissionDeniedResponse(HTTPServerResponse& response, const std::string& description);

        /* 404 */
        void SetNotFoundResponse(HTTPServerResponse& response, const std::string& description);

        /* 406 */
        void SetNotAcceptableResponse(HTTPServerResponse& response, const std::string& description);

        /* 500 */
        void SetInternalErrorResponse(HTTPServerResponse& response, const std::string& description);

        std::optional<std::pair<std::string, long>>
        AuthRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

        std::optional<bool>
        CheckArticleExistsRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response, long id);

    private:
        std::string format_;
        HandlerType type_;
        std::string instance_;

    };


} // namespace handler

class i_request_handler {

};


#endif //SERVER_I_REQUEST_HANDLER_H
