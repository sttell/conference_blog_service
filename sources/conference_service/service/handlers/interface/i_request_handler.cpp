#include "i_request_handler.h"

#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/URI.h>
#include <Poco/JSON/Parser.h>

#include <utility>

namespace {

    const std::string kAuthServer = "http://users_service:8080/auth";
    const std::string kArticlesServer = "http://articles_service:8081/article";

} // namespace constants

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

    std::optional<std::pair<std::string, long>>
    IRequestHandler::AuthRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {

        if (!request.hasCredentials()) {
            SetUnauthorizedResponse(response, "User is unauthorized.");
            return { };
        }

        std::string schema;
        std::string base64;
        request.getCredentials(schema, base64);

        std::string auth_token = schema + " " + base64;
        std::string url = kAuthServer;
        std::cout << auth_token << std::endl;

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

            return { };
        }

        return std::make_pair(
                json_response->get("user_role").convert<std::string>(),
                json_response->get("id").convert<long>()
        );
    }

    std::optional<bool>
    IRequestHandler::CheckArticleExistsRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response, long id) {

        if (!request.hasCredentials()) {
            SetUnauthorizedResponse(response, "User is unauthorized.");
            return { };
        }

        std::string schema;
        std::string base64;
        request.getCredentials(schema, base64);

        std::string auth_token = schema + " " + base64;
        std::string url = kArticlesServer + "?id=" + std::to_string(id);

        Poco::URI uri(url);
        Poco::Net::HTTPClientSession s(uri.getHost(), uri.getPort());
        Poco::Net::HTTPRequest search_request(Poco::Net::HTTPRequest::HTTP_GET, uri.toString());
        search_request.setVersion(Poco::Net::HTTPMessage::HTTP_1_1);
        search_request.setContentType("application/json");
        search_request.setCredentials(schema, base64);
        search_request.setProxyCredentials(schema, base64);
        search_request.set("Accept", "application/json");
        search_request.setKeepAlive(true);

        s.sendRequest(search_request);

        Poco::Net::HTTPResponse auth_response;
        std::istream &rs = s.receiveResponse(auth_response);

        Poco::JSON::Parser parser;
        auto json_response = parser.parse(rs).extract<Poco::JSON::Object::Ptr>();

        if ( auth_response.getStatus() == Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND ) {
            return false;
        }

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

            return { };
        }

        return true;
    }

} // namespace handler