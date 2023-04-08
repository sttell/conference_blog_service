#include "search_handler.h"

#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Base64Decoder.h>
#include <Poco/URI.h>

#include "database/database.h"
#include "database/user_role.h"
#include "database/article.h"

#include <iostream>
#include <string>
#include <vector>

using Poco::Net::HTMLForm;

namespace handler {

    SearchHandler::SearchHandler(const std::string &format) :
        IRequestHandler(format, HandlerType::Search, "/search") { /* Empty */ }

    void SearchHandler::handleRequest([[maybe_unused]]Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
        try {
            if ( request.getMethod() == HTTPServerRequest::HTTP_GET ) {
                HandleGetRequest(request, response);
            } else {
                SetBadRequestResponse(response, "Service unsupported this method for /search URI.");
            }

        } catch (const std::exception& e) {

            std::string error_desc{ "Server end of work with exception: " };
            error_desc += e.what();
            SetInternalErrorResponse(response, error_desc);

        }
    }

    void
    SearchHandler::HandleGetRequest([[maybe_unused]]Poco::Net::HTTPServerRequest &request, [[maybe_unused]]Poco::Net::HTTPServerResponse &response) {

        database::UserRole user_role;
        if ( request.hasCredentials() ) {

            auto user_data = AuthRequest(request, response);
            if ( !user_data.has_value() )
                return;
            user_role = database::UserRole(user_data.value().first);
        } else {
            SetUnauthorizedResponse(response, "User is unauthorized.");
            return;
        }

        if ( user_role < database::UserRole::User ) {
            SetPermissionDeniedResponse(response, "You don't have permission for this action.");
            return;
        }

        HTMLForm form(request);


        auto articles = database::Article::ReadAll();

        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/success");
        root->set("title", "OK");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_OK);
        root->set("instance", "/article");

        Poco::JSON::Array arr;
        for ( auto& article : articles ) {
            arr.add(article);
        }
        root->set("articles", arr);

        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);

    }

} // namespace handler