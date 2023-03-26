#ifndef SERVER_HANDLER_FACTORY_H
#define SERVER_HANDLER_FACTORY_H

#include "handler_type.h"
#include "i_request_handler.h"
#include <memory>

namespace handler {

    class HandlerFactory {
    public:
        static IRequestHandler* Create(const std::string& format, const std::string& request_URI);
    };

} // namespace handler

#endif //SERVER_HANDLER_FACTORY_H
