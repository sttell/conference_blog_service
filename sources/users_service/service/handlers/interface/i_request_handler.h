#ifndef SERVER_I_REQUEST_HANDLER_H
#define SERVER_I_REQUEST_HANDLER_H

#include "handler_type.h"

#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;

namespace handler {

    class IRequestHandler : public HTTPRequestHandler {
    public:
        IRequestHandler(const std::string& format, HandlerType type);

    public:

        virtual void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) = 0;

        HandlerType GetType() const noexcept;

    protected:
        std::string& GetFormat() noexcept;

    private:
        std::string format_;
        HandlerType type_;
    };


} // namespace handler

class i_request_handler {

};


#endif //SERVER_I_REQUEST_HANDLER_H
