#include "auth_handler.h"

#include <Poco/JSON/Object.h>
#include <Poco/Net/HTMLForm.h>

#include "database/database.h"
#include "database/user.h"

#include <string>
#include <vector>

using Poco::Net::HTMLForm;

namespace handler {

    AuthHandler::AuthHandler(const std::string &format) : IRequestHandler(format,
                                                                              HandlerType::Search) { /* Empty */ }

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

        HTMLForm form(request);
        if (!(form.has("login") && form.has("password"))) {
            SetBadRequestResponse(response, "Wrong request data.");
        }
        std::string login    = form.get("login");
        std::string password = form.get("password");

        auto user = database::User::AuthUser(login, password);
        if ( !user.has_value() ) {
            SetNotFoundResponse(response, "User not found.");
            return;
        }

        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");

        Poco::JSON::Object object;
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/success");
        root->set("title", "OK");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_OK);
        root->set("instance", "/user");
        root->set("id", std::to_string(user->GetID()));
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);

    }


    /**
     * @brief Заполнение BadRequest(400) формы ответа
     * @param response HTML ответ для записи.
     * @param description - описание ошибки.
     */
    void AuthHandler::SetBadRequestResponse(HTTPServerResponse &response, const std::string &description) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/bad_request");
        root->set("title", "Bad request error");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_BAD_REQUEST);
        root->set("detail", description);
        root->set("instance", "/auth");
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

    /**
     * @brief Заполнение NotFound(404) формы ответа
     * @param response HTML ответ для записи.
     * @param description - описание ошибки.
     */
    void AuthHandler::SetNotFoundResponse(Poco::Net::HTTPServerResponse &response, const std::string &description) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/not_acceptable_error");
        root->set("title", "Not found error.");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_NOT_FOUND);
        root->set("detail", description);
        root->set("instance", "/auth");

        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

    /**
     * @brief Заполнение InternalError(500) формы ответа
     * @param response HTML ответ для записи.
     * @param description - описание ошибки.
     */
    void AuthHandler::SetInternalErrorResponse(Poco::Net::HTTPServerResponse &response,
                                                 const std::string &description) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_INTERNAL_SERVER_ERROR);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/internal_error");
        root->set("title", "Internal Server Error");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_INTERNAL_SERVER_ERROR);
        root->set("detail", description);
        root->set("instance", "/auth");
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }


} // namespace handler