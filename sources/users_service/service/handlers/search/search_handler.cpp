#include "search_handler.h"

#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Base64Decoder.h>
#include <Poco/URI.h>

#include "database/database.h"
#include "database/user.h"

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
    SearchHandler::HandleGetRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {

        database::UserRole request_sender_role;

        {
            if (!request.hasCredentials()) {
                SetUnauthorizedResponse(response, "User is unauthorized.");
                return;
            }

            std::string schema;
            std::string base64;
            request.getCredentials(schema, base64);

            std::string auth_token = schema + " " + base64;
            std::string url = "http://127.0.0.1:8080/auth";

            Poco::URI uri(url);
            Poco::Net::HTTPClientSession s(uri.getHost(), uri.getPort());
            Poco::Net::HTTPRequest auth_request(Poco::Net::HTTPRequest::HTTP_GET, uri.toString());
            auth_request.setVersion(Poco::Net::HTTPMessage::HTTP_1_1);
            auth_request.setContentType("application/json");
            auth_request.setCredentials(schema, base64);
            auth_request.setProxyCredentials(schema, base64);
            auth_request.set("Accept", "application/json");
            auth_request.setKeepAlive(true);

            s.sendRequest(auth_request);

            Poco::Net::HTTPResponse auth_response;
            std::istream &rs = s.receiveResponse(auth_response);

            Poco::JSON::Parser parser;
            auto json_response = parser.parse(rs).extract<Poco::JSON::Object::Ptr>();

            if ( auth_response.getStatus() != Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK ) {
                response.setStatus(auth_response.getStatus());
                response.setContentType(auth_response.getContentType());
                response.setReason(auth_response.getReason());
                response.setChunkedTransferEncoding(auth_response.getChunkedTransferEncoding());

                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("type", json_response->get("type"));
                root->set("status", json_response->get("status"));
                root->set("detail", json_response->get("detail"));
                root->set("instance", json_response->get("instance"));
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);

            }

            request_sender_role = database::UserRole{ json_response->get("user_role").convert<std::string>() };

        }

        /* Проверка доступа к запросу. */
        if ( request_sender_role < database::UserRole::User ) {
            SetPermissionDeniedResponse(response, "User does not have privileges for this action");
            return;
        }

        HTMLForm form(request);
        if ( !(form.has("first_name") && form.has("last_name")) ) {
            SetBadRequestResponse(response, "Wrong request data.");
        }
        std::string first_name = form.get("first_name");
        std::string last_name  = form.get("last_name");

        std::vector<database::User> users = database::User::Search(first_name, last_name);
        if ( users.empty() ) {
            SetNotFoundResponse(response, "Users provided by mask not found.");
            return;
        }

        Poco::JSON::Array arr;
        for (const auto& s : users)
            arr.add(s.ToJSON());

        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(arr, ostr);
    }

} // namespace handler