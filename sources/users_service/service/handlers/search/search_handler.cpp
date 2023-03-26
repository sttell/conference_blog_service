#include "search_handler.h"

#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Object.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Base64Decoder.h>

#include "database/database.h"
#include "database/user.h"

#include <iostream>
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

    SearchHandler::SearchHandler(const std::string &format) : IRequestHandler(format, HandlerType::Search) { /* Empty */ }

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

        database::User request_sender;

        /* Проверка авторизации. TODO: Вынести в отдельный обработчик запросов. */
        if ( request.hasCredentials() ) {

            std::string scheme;
            std::string base64;
            request.getCredentials(scheme, base64);

            std::stringstream decode_stream;
            decode_stream << base64;
            Poco::Base64Decoder base64_decoder{decode_stream};

            std::string decoded;
            base64_decoder >> decoded;

            auto credentials = SplitString(decoded, ":");
            auto user = database::User::AuthUser(credentials[0], credentials[1]);
            if ( user.has_value() ) {
                request_sender = user.value();
            } else {
                SetBadRequestResponse(response, "Invalid login or password.");
                return;
            }
        } else {
            SetUnauthorizedResponse(response, "User is unauthorized.");
            return;
        }

        /* Проверка доступа к запросу. */
        if ( request_sender.GetRole() < database::UserRole::User ) {
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


    /**
     * @brief Заполнение BadRequest(400) формы ответа
     * @param response HTML ответ для записи.
     * @param description - описание ошибки.
     */
    void SearchHandler::SetBadRequestResponse(HTTPServerResponse &response, const std::string &description) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/bad_request");
        root->set("title", "Bad request error");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_BAD_REQUEST);
        root->set("detail", description);
        root->set("instance", "/search");
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

    /**
     * @brief Заполнение Unauthorized(401) формы ответа
     * @param response HTML ответ для записи.
     * @param description - описание ошибки.
     */
    void SearchHandler::SetUnauthorizedResponse(Poco::Net::HTTPServerResponse &response, const std::string &description) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/unauthorized_error");
        root->set("title", "User unauthorized.");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_UNAUTHORIZED);
        root->set("detail", description);
        root->set("instance", "/search");

        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

    /**
     * @brief Заполнение Forbidden(403) формы ответа
     * @param response HTML ответ для записи.
     * @param description - описание ошибки.
     */
    void
    SearchHandler::SetPermissionDeniedResponse(Poco::Net::HTTPServerResponse &response, const std::string &description) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_FORBIDDEN);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/forbidden_error");
        root->set("title", "Permission denied.");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_FORBIDDEN);
        root->set("detail", description);
        root->set("instance", "/search");

        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

    /**
     * @brief Заполнение NotFound(404) формы ответа
     * @param response HTML ответ для записи.
     * @param description - описание ошибки.
     */
    void SearchHandler::SetNotFoundResponse(Poco::Net::HTTPServerResponse &response, const std::string &description) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/not_acceptable_error");
        root->set("title", "Not found error.");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_NOT_FOUND);
        root->set("detail", description);
        root->set("instance", "/search");

        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

    /**
     * @brief Заполнение InternalError(500) формы ответа
     * @param response HTML ответ для записи.
     * @param description - описание ошибки.
     */
    void SearchHandler::SetInternalErrorResponse(Poco::Net::HTTPServerResponse &response,
                                               const std::string &description) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_INTERNAL_SERVER_ERROR);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/internal_error");
        root->set("title", "Internal Server Error");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_INTERNAL_SERVER_ERROR);
        root->set("detail", description);
        root->set("instance", "/search");
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

} // namespace handler