#ifndef SEARCH_SERVICE_DATABASE_USER_H
#define SEARCH_SERVICE_DATABASE_USER_H


#include <string>
#include <vector>
#include "Poco/JSON/Object.h"
#include <optional>

namespace database
{
    class User{
    private:
        long _id;
        std::string _login;
        std::string _first_name;
        std::string _last_name;
        std::string _second_name;
        std::string _email;
        int _gender;

    public:

        static User fromJSON(const std::string & str);

        long               get_id() const;
        const std::string &get_first_name() const;
        const std::string &get_last_name() const;
        const std::string &get_second_name() const;
        const std::string &get_email() const;
        const std::string &get_login() const;
        int                get_gender() const;

        long&        id();
        std::string &first_name();
        std::string &last_name();
        std::string &second_name();
        std::string &email();
        std::string &login();
        int&         gender();

        static void init();

        static std::optional<User> search(const std::string& user_login);
        static std::optional<User> read_by_id(long id);
        static std::vector<User> read_all();
        static std::vector<User> search(std::string first_name,std::string last_name);
        void save_to_mysql();

        Poco::JSON::Object::Ptr toJSON() const;

    };
}

#endif //SEARCH_SERVICE_DATABASE_USER_H
