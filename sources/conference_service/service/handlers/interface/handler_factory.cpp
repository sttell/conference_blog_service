#include "handler_factory.h"

#include "../article/article_handler.h"
#include "../search/search_handler.h"

#include "../../../../shared/errors.h"

#include <optional>
#include <map>

namespace {

    std::optional<handler::HandlerType> GetTypeByURI( const std::string& URI ) {
        static const std::map<std::string, handler::HandlerType> handlers_uri_map = {
                { "/article", handler::HandlerType::Article },
                { "/search",  handler::HandlerType::Search }
        };

        for ( const auto& [ handler_uri, handler_type ] : handlers_uri_map ) {
            if ( URI.find(handler_uri) != std::string::npos ) {
                return handler_type;
            }
        }
        return std::optional<handler::HandlerType>{ /* Empty */ };
    }

} // functions

namespace handler {

    IRequestHandler* HandlerFactory::Create(const std::string &format, const std::string& request_URI) {

        std::optional<HandlerType> type = GetTypeByURI(request_URI);

        if ( !type.has_value() ) {
            throw exceptions::BadURI("Failed to create request handler by URI: " + request_URI);
        }

        switch (type.value()) {
            case HandlerType::Article:
                return new ArticleHandler(format);
            case HandlerType::Search:
                return new SearchHandler(format);
        }

        throw exceptions::UnexpectedHandlerType("Unknown handler type " + std::to_string(static_cast<int>(type.value())));
    }

} // namespace handler