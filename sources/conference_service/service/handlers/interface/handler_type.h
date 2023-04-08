#ifndef SERVER_HANDLER_TYPE_H
#define SERVER_HANDLER_TYPE_H

#include <cstdint>

namespace handler {

    class HandlerType {
    public:
        enum Type : uint8_t {
            Article,
            Search
        };

        HandlerType() = default;

        constexpr HandlerType(Type type) : type_(type) { /* Empty */ };
        constexpr operator Type() const noexcept { return type_; }

        constexpr bool operator==(const HandlerType::Type& rhs) { return type_ == rhs; }
        constexpr bool operator==(const HandlerType& rhs) { return type_ == rhs.type_; }
        constexpr bool operator!=(const HandlerType& rhs) { return type_ != rhs.type_; }
        constexpr bool operator< (const HandlerType& rhs) { return type_ <  rhs.type_; }

    private:
        Type type_;
    };

} // namespace handler

#endif //SERVER_HANDLER_TYPE_H
