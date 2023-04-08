#include "database/user_role.h"

namespace constants {

    constexpr const char* kUserRoleString           = "user";
    constexpr const char* kModeratorRoleString      = "moderator";
    constexpr const char* kAdministratorRoleString  = "administrator";
    constexpr const char* kDefaultRoleString        = kUserRoleString;

} // namespace constants

namespace database {

    UserRole::UserRole() : type_(Type::User) { }

    UserRole::UserRole(const std::string &role) : UserRole() {
        if ( role == constants::kUserRoleString ) {
            type_ = Type::User;
        } else if ( role == constants::kModeratorRoleString ) {
            type_ = Type::Moderator;
        } else if ( role == constants::kAdministratorRoleString ) {
            type_ = Type::Administrator;
        }
    }

    std::string UserRole::ToString() const {
        switch (type_) {
            case Type::User:
                return constants::kUserRoleString;
            case Type::Moderator:
                return constants::kModeratorRoleString;
            case Type::Administrator:
                return constants::kAdministratorRoleString;
            default:
                return constants::kDefaultRoleString;
        }
    }

} // namespace database
