#ifndef SERVER_USER_H
#define SERVER_USER_H

#include "Poco/JSON/Object.h"

#include "user_role.h"

#include <string>
#include <vector>
#include <optional>

namespace database {

    class User {
        friend class Database;
    public:
        User() = default;
        User(User&& user) = default;
        User(const User& user) = default;
        User& operator=(User&& user) = default;
        User& operator=(const User& user) = default;

        static User FromJSON(const std::string & str);

        [[nodiscard]] long               GetID() const noexcept;
        [[nodiscard]] const std::string& GetFirstName() const noexcept;
        [[nodiscard]] const std::string& GetLastName() const noexcept;
        [[nodiscard]] const std::string& GetMiddleName() const noexcept;
        [[nodiscard]] const std::string& GetEMail() const noexcept;
        [[nodiscard]] const std::string& GetGender() const noexcept;
        [[nodiscard]] const std::string& GetLogin() const noexcept;
        [[nodiscard]] const std::string& GetPassword() const noexcept;
        [[nodiscard]] const UserRole&    GetRole() const noexcept;


        long&        ID() noexcept;
        std::string& FirstName() noexcept;
        std::string& LastName() noexcept;
        std::string& MiddleName() noexcept;
        std::string& EMail() noexcept;
        std::string& Gender() noexcept;
        std::string& Login() noexcept;
        std::string& Password() noexcept;
        UserRole&    Role() noexcept;

        static void Init();

        static std::vector<User> ReadAll();
        static std::vector<User> Search(std::string first_name,std::string last_name);
        static std::optional<User> SearchByID(long id);
        static std::optional<User> SearchByLogin(std::string login);
        static std::optional<User> ChangeRole(std::string login, UserRole new_role);
        static std::optional<User> AuthUser(std::string login, std::string password);
        static std::optional<User> FromCacheByID(long id);

        void SaveToCache();

        void InsertToDatabase();

        [[nodiscard]] Poco::JSON::Object::Ptr ToJSON() const;

        std::string Serialize() const;
        void Deserialize(const std::string&);

    private:
        long id_{-1};
        std::string first_name_;
        std::string last_name_;
        std::string middle_name_;
        std::string email_;
        std::string gender_;

        std::string login_;
        std::string password_;
        UserRole role_;

    };

} // namespace database

#endif //SERVER_USER_H
