#ifndef SERVER_USER_ROLE_H
#define SERVER_USER_ROLE_H

#include <cstdint>
#include <string>

namespace database {

    class UserRole {
    public:
        enum Type : uint8_t {
            User,
            Moderator,
            Administrator
        };

        UserRole();
        explicit UserRole(const std::string& role);
        constexpr UserRole(Type type) : type_(type) { /* Empty */ };
        constexpr operator Type() const noexcept { return type_; }

        constexpr bool operator==(const UserRole::Type& rhs) { return type_ == rhs; }
        constexpr bool operator< (const UserRole::Type& rhs) { return type_ <  rhs; }
        constexpr bool operator==(const UserRole& rhs) { return type_ == rhs.type_; }
        constexpr bool operator!=(const UserRole& rhs) { return type_ != rhs.type_; }
        constexpr bool operator< (const UserRole& rhs) { return type_ <  rhs.type_; }

        std::string ToString() const;

    private:
        Type type_;
    };

} // namespace database

#endif //SERVER_USER_ROLE_H
