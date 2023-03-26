#include "i_request_handler.h"

#include <Poco/JSON/Object.h>

#include <utility>


namespace handler {

    IRequestHandler::IRequestHandler(std::string format, HandlerType type, std::string instance_name) :
        format_(std::move(format)),
        type_(type),
        instance_(std::move(instance_name)) { /* Empty */ }

    std::string& IRequestHandler::Instance() noexcept {
        return instance_;
    }

    std::string &IRequestHandler::GetFormat() noexcept {
        return format_;
    }

    HandlerType IRequestHandler::GetType() const noexcept {
        return type_;
    }

    /**
     * @brief Заполнение BadRequest(400) формы ответа
     * @param response HTML ответ для записи.
     * @param description - описание ошибки.
     */
    void IRequestHandler::SetBadRequestResponse(HTTPServerResponse &response, const std::string &description) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/bad_request");
        root->set("title", "Bad request error");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_BAD_REQUEST);
        root->set("detail", description);
        root->set("instance", this->Instance());
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

    /**
     * @brief Заполнение Unauthorized(401) формы ответа
     * @param response HTML ответ для записи.
     * @param description - описание ошибки.
     */
    void IRequestHandler::SetUnauthorizedResponse(Poco::Net::HTTPServerResponse &response, const std::string &description) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/unauthorized_error");
        root->set("title", "User unauthorized.");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_UNAUTHORIZED);
        root->set("detail", description);
        root->set("instance", this->Instance());

        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

    /**
     * @brief Заполнение Forbidden(403) формы ответа
     * @param response HTML ответ для записи.
     * @param description - описание ошибки.
     */
    void
    IRequestHandler::SetPermissionDeniedResponse(Poco::Net::HTTPServerResponse &response, const std::string &description) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_FORBIDDEN);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/forbidden_error");
        root->set("title", "Permission denied.");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_FORBIDDEN);
        root->set("detail", description);
        root->set("instance", this->Instance());

        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

    /**
     * @brief Заполнение NotFound(404) формы ответа
     * @param response HTML ответ для записи.
     * @param description - описание ошибки.
     */
    void IRequestHandler::SetNotFoundResponse(Poco::Net::HTTPServerResponse &response, const std::string &description) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/not_acceptable_error");
        root->set("title", "Not found error.");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_NOT_FOUND);
        root->set("detail", description);
        root->set("instance", this->Instance());

        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

    /**
     * @brief Заполнение NotAcceptable(406) формы ответа
     * @param response HTML ответ для записи.
     * @param description - описание ошибки.
     */
    void
    IRequestHandler::SetNotAcceptableResponse(Poco::Net::HTTPServerResponse &response, const std::string &description) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_ACCEPTABLE);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/not_acceptable_error");
        root->set("title", "Not acceptable error.");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_NOT_ACCEPTABLE);
        root->set("detail", description);
        root->set("instance", this->Instance());

        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }


    /**
     * @brief Заполнение InternalError(500) формы ответа
     * @param response HTML ответ для записи.
     * @param description - описание ошибки.
     */
    void IRequestHandler::SetInternalErrorResponse(Poco::Net::HTTPServerResponse &response,
                                               const std::string &description) {
        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_INTERNAL_SERVER_ERROR);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/internal_error");
        root->set("title", "Internal Server Error");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_INTERNAL_SERVER_ERROR);
        root->set("detail", description);
        root->set("instance", this->Instance());
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

} // namespace handler