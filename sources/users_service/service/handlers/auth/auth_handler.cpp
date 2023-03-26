#include "auth_handler.h"

#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Object.h>

#include <iostream>

namespace handler {

    AuthHandler::AuthHandler(const std::string &format)  : IRequestHandler(format, HandlerType::Auth) { /* Empty */ }

    void AuthHandler::handleRequest([[maybe_unused]]Poco::Net::HTTPServerRequest &request, [[maybe_unused]]Poco::Net::HTTPServerResponse &response) {
        std::cout << "Handle auth request" << std::endl;


        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/not_found");
        root->set("title", "Internal exception");
        root->set("status", Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
        root->set("detail", "request ot found");
        root->set("instance", "/user");
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

} // namespace handler