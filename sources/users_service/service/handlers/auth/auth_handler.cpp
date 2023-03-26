#include "auth_handler.h"

#include <Poco/JSON/Object.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Base64Decoder.h>

#include "database/database.h"
#include "database/user.h"

#include <string>
#include <vector>

using Poco::Net::HTMLForm;

namespace {

    /**
     * @brief Разделение строки на подстроки по разделяющему символу
     * @param str исходная строка
     * @param delimiter разделитель
     * @return вектор подстрок разделенных по delimiter
     */
    std::vector<std::string> SplitString(const std::string& str, const std::string& delimiter=" ") {
        std::string str_copy = str;
        size_t pos;
        std::vector<std::string> tokens;
        while ((pos = str_copy.find(delimiter)) != std::string::npos) {
            tokens.push_back(str_copy.substr(0, pos));
            str_copy.erase(0, pos + delimiter.length());
        }
        tokens.push_back(str_copy);
        return tokens;
    }

} // namespace [ functions ]

namespace handler {

    AuthHandler::AuthHandler(const std::string &format) :
        IRequestHandler(format, HandlerType::Search, "/auth") { /* Empty */ }

    void AuthHandler::handleRequest([[maybe_unused]]Poco::Net::HTTPServerRequest &request,
                                      Poco::Net::HTTPServerResponse &response) {
        try {
            if (request.getMethod() == HTTPServerRequest::HTTP_GET) {
                HandleGetRequest(request, response);
            } else {
                SetBadRequestResponse(response, "Service unsupported this method for /auth URI.");
            }

        } catch (const std::exception &e) {

            std::string error_desc{"Server end of work with exception: "};
            error_desc += e.what();
            SetInternalErrorResponse(response, error_desc);

        }
    }

    void
    AuthHandler::HandleGetRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {

        database::User request_sender;

        if ( request.hasCredentials() ) {

            std::string scheme;
            std::string base64;
            request.getCredentials(scheme, base64);

            if ( scheme != "Basic" ) {
                SetBadRequestResponse(response, "Unsupported authorization scheme.");
                return;
            }

            std::stringstream decode_stream;
            decode_stream << base64;
            Poco::Base64Decoder base64_decoder{decode_stream};

            std::string decoded;
            base64_decoder >> decoded;

            auto credentials = SplitString(decoded, ":");
            if ( credentials.size() != 2 ) {
                SetBadRequestResponse(response, "Invalid auth credentials.");
                return;
            }
            auto user = database::User::AuthUser(credentials[0], credentials[1]);
            if ( user.has_value() ) {
                request_sender = user.value();
            } else {
                SetNotFoundResponse(response, "Invalid login or password.");
                return;
            }
        } else {
            SetUnauthorizedResponse(response, "User is unauthorized.");
            return;
        }

        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/success");
        root->set("title", "OK");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_OK);
        root->set("instance", "/user");
        root->set("id", std::to_string(request_sender.GetID()));
        root->set("user_role", request_sender.GetRole().ToString());

        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);

    }


} // namespace handler