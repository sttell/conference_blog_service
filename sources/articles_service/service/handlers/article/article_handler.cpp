#include "article_handler.h"

#include <Poco/JSON/Object.h>
#include <Poco/Net/HTMLForm.h>

#include "database/database.h"
#include "database/user_role.h"
#include "database/article.h"

#include <string>
#include <vector>

using Poco::Net::HTMLForm;



namespace handler {

    ArticleHandler::ArticleHandler(const std::string &format) :
        IRequestHandler(format, HandlerType::Article, "/article") { /* Empty */ }

    void ArticleHandler::handleRequest([[maybe_unused]]Poco::Net::HTTPServerRequest &request,
                                      Poco::Net::HTTPServerResponse &response) {
        try {
            if (request.getMethod() == HTTPServerRequest::HTTP_GET) {
                HandleGetRequest(request, response);
            } else if ( request.getMethod() == HTTPServerRequest::HTTP_POST ) {
                HandlePostRequest(request, response);
            } else if ( request.getMethod() == HTTPServerRequest::HTTP_DELETE ) {
                HandleDeleteRequest(request, response);
            } else {
                SetBadRequestResponse(response, "Service unsupported this method for /article URI.");
            }

        } catch (const std::exception &e) {

            std::string error_desc{"Server end of work with exception: "};
            error_desc += e.what();
            SetInternalErrorResponse(response, error_desc);

        }
    }

    void
    ArticleHandler::HandleGetRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {

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

        if ( !form.has("id") ) {
            SetBadRequestResponse(response, "Need ID in request data.");
            return;
        }

        long id = atol(form.get("id").c_str());
        auto article = database::Article::SearchByID(id);
        if ( !article.has_value() ) {
            SetNotFoundResponse(response, "Article not found.");
            return;
        }

        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/success");
        root->set("title", "OK");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_OK);
        root->set("instance", "/article");
        root->set("article", article->ToJSON());

        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);

    }

    void
    ArticleHandler::HandlePostRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
        database::UserRole user_role;
        long user_id;
        if ( request.hasCredentials() ) {

            std::optional<std::pair<std::string, long>> user_data = AuthRequest(request, response);
            if ( !user_data.has_value() )
                return;
            user_role = database::UserRole(user_data.value().first);
            user_id = user_data.value().second;
        } else {
            SetUnauthorizedResponse(response, "User is unauthorized.");
            return;
        }

        if ( user_role < database::UserRole::User ) {
            SetPermissionDeniedResponse(response, "You don't have permission for this action.");
            return;
        }

        HTMLForm form(request);

        bool is_form_has_required = form.has("title") &&
                                    form.has("description") &&
                                    form.has("content");

        if ( !is_form_has_required ) {
            SetBadRequestResponse(response, "Wrong request data.");
            return;
        }

        std::string title = form.get("title");
        std::string description = form.get("description");
        std::string content = form.get("content");
        std::string external_link;
        if ( form.has("external_link") ) {
            external_link = form.get("external_link");
        }

        database::Article article;
        article.ConsumerID() = user_id;
        article.Title() = title;
        article.Description() = description;
        article.Content() = content;
        article.ExternalLink() = external_link;

        article.InsertToDatabase();

        if ( article.GetID() == -1 ) {
            SetInternalErrorResponse(response, "Article insert error.");
            return;
        }

        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/success");
        root->set("title", "OK");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_OK);
        root->set("instance", "/article");
        root->set("id", article.GetID());

        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);

    }

    void ArticleHandler::HandleDeleteRequest(Poco::Net::HTTPServerRequest &request,
                                             Poco::Net::HTTPServerResponse &response) {

        database::UserRole user_role;
        long user_id;
        if ( request.hasCredentials() ) {

            std::optional<std::pair<std::string, long>> user_data = AuthRequest(request, response);
            if ( !user_data.has_value() )
                return;
            user_role = database::UserRole(user_data.value().first);
            user_id = user_data.value().second;
        } else {
            SetUnauthorizedResponse(response, "User is unauthorized.");
            return;
        }

        if ( user_role < database::UserRole::User ) {
            SetPermissionDeniedResponse(response, "You don't have permission for this action.");
            return;
        }

        HTMLForm form(request);

        if ( !form.has("id") ) {
            SetBadRequestResponse(response, "Wrong request data.");
            return;
        }

        long article_id = atol(form.get("id").c_str());

        auto article = database::Article::SearchByID(article_id);
        if ( !article.has_value() ) {
            SetNotFoundResponse(response, "Article not found.");
            return;
        }

        if ( article.value().ConsumerID() == user_id || user_role >= database::UserRole::Moderator) {

            bool is_success = database::Article::DeleteByID(article_id);
            if ( !is_success ) {
                SetInternalErrorResponse(response, "Failed to delete article");
                return;
            }
        } else {
            SetPermissionDeniedResponse(response, "You don't have permission for this action.");
            return;
        }

        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/success");
        root->set("title", "OK");
        root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_OK);
        root->set("instance", "/article");
        root->set("id", article_id);

        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

} // namespace handler