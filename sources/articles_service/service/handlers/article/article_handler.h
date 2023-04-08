#ifndef SERVER_ARTICLE_HANDLER_H
#define SERVER_ARTICLE_HANDLER_H

#include "../interface/i_request_handler.h"

namespace handler {

    class ArticleHandler : public IRequestHandler {
    public:
        explicit ArticleHandler(const std::string& format);
        ~ArticleHandler() override = default;
    public:

        void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override;

    private:

        void HandleGetRequest(HTTPServerRequest& request, HTTPServerResponse& response);

        void HandlePostRequest(HTTPServerRequest& request, HTTPServerResponse& response);

        void HandleDeleteRequest(HTTPServerRequest& request, HTTPServerResponse& response);


    };

} // namespace handler

#endif //SERVER_ARTICLE_HANDLER_H
