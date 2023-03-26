#include "i_request_handler.h"


namespace handler {

    IRequestHandler::IRequestHandler(const std::string &format, HandlerType type) :
        format_(format),
        type_(std::move(type)) { /* Empty */ }

    std::string &IRequestHandler::GetFormat() noexcept {
        return format_;
    }

    HandlerType IRequestHandler::GetType() const noexcept {
        return type_;
    }

} // namespace handler