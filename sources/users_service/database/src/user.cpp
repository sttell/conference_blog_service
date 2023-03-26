#include "database/user.h"

#include "database/database.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

#define TABLE_NAME "Users"
#define CREATE_TABLE_REQUEST \
    "CREATE TABLE IF NOT EXISTS `" TABLE_NAME "` "                  \
    "(`id` "         "INT "          "NOT NULL " "AUTO_INCREMENT,"  \
    "`first_name` "  "VARCHAR(256) " "NOT NULL,"                    \
    "`last_name` "   "VARCHAR(256) " "NOT NULL,"                    \
    "`middle_name` " "VARCHAR(256) " "NULL,"                        \
    "`email` "       "VARCHAR(256) " "NOT NULL,"                    \
    "`gender` "      "VARCHAR(32) "  "NOT NULL,"                    \
    "`login` "       "VARCHAR(256) " "NOT NULL,"                    \
    "`password` "    "VARCHAR(256) " "NOT NULL,"                    \
    "`role`     "    "VARCHAR(32)  " "NOT NULL,"                    \
    "PRIMARY KEY (`id`), "                                          \
    "KEY `fn` (`first_name`),"                                      \
    "KEY `ln` (`last_name`));"

#define SELECT_USER_REQUEST \
    "SELECT id, first_name, last_name, middle_name, email, gender, login, password, role FROM " \
    TABLE_NAME

#define SELECT_BY_MASK_REQUEST \
    "SELECT id, first_name, last_name, middle_name, email, gender, role FROM " \
    TABLE_NAME \
    " WHERE first_name LIKE ? and last_name LIKE ?"

#define SELECT_BY_ID_REQUEST \
    "SELECT id, first_name, last_name, middle_name, email, gender, role FROM " \
    TABLE_NAME \
    " WHERE id=?"

#define SELECT_BY_LOGIN_REQUEST \
    "SELECT id, first_name, last_name, middle_name, email, gender, role FROM " \
    TABLE_NAME \
    " WHERE login=?"

#define UPDATE_ROLE_REQUEST     \
    "UPDATE " TABLE_NAME " "    \
    "SET role=? "               \
    "WHERE login=?"

#define SELECT_BY_CREDENTIALS_REQUEST \
    "SELECT id, first_name, last_name, middle_name, email, gender, role FROM " \
    TABLE_NAME \
    " WHERE login=? and password=?"

#define INSERT_USER_REQUEST \
    "INSERT INTO " TABLE_NAME " " \
    "(first_name, last_name, middle_name, email, gender, login, password, role) " \
    "VALUES(?, ?, ?, ?, ?, ?, ?, ?)"


namespace database {

    User User::FromJSON(const std::string & str) {
        User info;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        info.ID()        = object->getValue<long>("id");
        info.FirstName() = object->getValue<std::string>("first_name");
        info.LastName()  = object->getValue<std::string>("last_name");
        info.EMail()     = object->getValue<std::string>("email");
        info.Gender()    = object->getValue<std::string>("gender");
        info.Login()     = object->getValue<std::string>("login");
        info.Password()  = object->getValue<std::string>("password");
        info.Role()      = UserRole(object->getValue<std::string>("role"));

        if ( object->has("middle_name") ) {
            info.MiddleName() = object->getValue<std::string>("middle_name");
        }

        return info;
    }

    long User::GetID() const noexcept { return id_; }

    const std::string &User::GetFirstName() const noexcept { return first_name_; }

    const std::string &User::GetLastName() const noexcept { return last_name_; }

    const std::string &User::GetMiddleName() const noexcept { return middle_name_; }

    const std::string &User::GetEMail() const noexcept { return email_; }

    const std::string &User::GetGender() const noexcept { return gender_; }

    const std::string &User::GetLogin() const noexcept { return login_; }

    const std::string &User::GetPassword() const noexcept { return password_; }

    const UserRole &User::GetRole() const noexcept { return role_; }

    long &User::ID() noexcept { return id_; }

    std::string &User::FirstName() noexcept { return first_name_; }

    std::string &User::LastName() noexcept { return last_name_; }

    std::string &User::MiddleName() noexcept { return middle_name_; }

    std::string &User::EMail() noexcept { return email_; }

    std::string &User::Gender() noexcept { return gender_; }

    std::string &User::Login() noexcept { return login_; }

    std::string &User::Password() noexcept { return password_; }

    UserRole &User::Role() noexcept { return role_; }

    void User::Init() {
        try {
            Poco::Data::Session session = database::Database::Instance().CreateSession();
            Statement create_stmt(session);
            create_stmt << CREATE_TABLE_REQUEST, now;
        }
        catch (Poco::Data::MySQL::ConnectionException &e) {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e) {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::vector<User> User::ReadAll() {
        try
        {
            Poco::Data::Session session = database::Database::Instance().CreateSession();
            Statement select(session);
            std::vector<User> result;

            User user;
            std::string role_str;
            select << SELECT_USER_REQUEST,
                    into(user.id_),
                    into(user.first_name_),
                    into(user.last_name_),
                    into(user.middle_name_),
                    into(user.email_),
                    into(user.gender_),
                    into(user.login_),
                    into(user.password_),
                    into(role_str),
                    range(0, 1); //  iterate over result set one row at a time

            while (!select.done()) {
                if (select.execute()) {
                    user.Role() = UserRole(role_str);
                    result.push_back(user);
                }
            }

            return result;
        }
        catch (Poco::Data::MySQL::ConnectionException &e) {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e) {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::vector<User> User::Search(std::string first_name, std::string last_name) {
        try {
            Poco::Data::Session session = database::Database::Instance().CreateSession();
            Statement select(session);
            std::vector<User> result;
            User info;

            std::string role_str;
            first_name += "%";
            last_name += "%";
            select << SELECT_BY_MASK_REQUEST,
                    into(info.id_),
                    into(info.first_name_),
                    into(info.last_name_),
                    into(info.middle_name_),
                    into(info.email_),
                    into(info.gender_),
                    into(role_str),
                    use(first_name),
                    use(last_name),
                    range(0, 1); //  iterate over result set one row at a time

            while (!select.done()) {
                if (select.execute()) {
                    info.Role() = UserRole(role_str);
                    result.push_back(info);
                }
            }
            return result;
        }

        catch (Poco::Data::MySQL::ConnectionException &e) {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e) {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::optional<User> User::SearchByID(long id) {
        try {
            Poco::Data::Session session = database::Database::Instance().CreateSession();
            Statement select(session);

            User info;

            std::string role_str;
            select << SELECT_BY_ID_REQUEST,
                    into(info.id_),
                    into(info.first_name_),
                    into(info.last_name_),
                    into(info.middle_name_),
                    into(info.email_),
                    into(info.gender_),
                    into(role_str),
                    use(id); //  iterate over result set one row at a time


            size_t selected_rows = select.execute();

            if ( selected_rows > 0 ) {
                info.Role() = UserRole(role_str);
                return info;
            }

            return { };
        }

        catch (Poco::Data::MySQL::ConnectionException &e) {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e) {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::optional<User> User::SearchByLogin(std::string login) {
        try {
            Poco::Data::Session session = database::Database::Instance().CreateSession();
            Statement select(session);

            User info;

            std::string role_str;
            select << SELECT_BY_LOGIN_REQUEST,
                    into(info.id_),
                    into(info.first_name_),
                    into(info.last_name_),
                    into(info.middle_name_),
                    into(info.email_),
                    into(info.gender_),
                    into(role_str),
                    use(login),
                    range(0, 1); //  iterate over result set one row at a time


            size_t selected_rows = select.execute();

            if ( selected_rows  ) {
                info.Role() = UserRole(role_str);
                std::cout << "User with login " << login << " found with ID " << info.id_ << std::endl;
                return info;
            }

            std::cout << "User with login " << login << " not found " << std::endl;

            return { };
        }
        catch (Poco::Data::MySQL::ConnectionException &e) {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e) {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::optional<User> User::ChangeRole(std::string login, database::UserRole new_role) {
        try {
            Poco::Data::Session session = database::Database::Instance().CreateSession();
            Statement update(session);

            std::string new_role_str = new_role.ToString();
            update << UPDATE_ROLE_REQUEST,
                      use(new_role_str),
                      use(login);

            size_t updated_rows = update.execute();

            if ( updated_rows == 0 ) {
                return { };
            }

            return SearchByLogin(login);
        }

        catch (Poco::Data::MySQL::ConnectionException &e) {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e) {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::optional<User> User::AuthUser(std::string login, std::string password) {
        try {
            Poco::Data::Session session = database::Database::Instance().CreateSession();
            Statement select(session);

            User info;

            std::string role_str;
            select << SELECT_BY_CREDENTIALS_REQUEST,
                    into(info.id_),
                    into(info.first_name_),
                    into(info.last_name_),
                    into(info.middle_name_),
                    into(info.email_),
                    into(info.gender_),
                    into(role_str),
                    use(login),
                    use(password); //  iterate over result set one row at a time

            size_t selected_rows = select.execute();
            if ( selected_rows > 0 ) {
                info.Role() = UserRole(role_str);
                return info;
            }
            return { };
        }

        catch (Poco::Data::MySQL::ConnectionException &e) {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e) {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    Poco::JSON::Object::Ptr User::ToJSON() const {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", id_);
        root->set("first_name", first_name_);
        root->set("last_name", last_name_);
        root->set("middle_name", middle_name_);
        root->set("email", email_);
        root->set("gender", gender_);
        root->set("role", role_.ToString());

        return root;
    }

    void User::InsertToDatabase() {
        try
        {
            Poco::Data::Session session = database::Database::Instance().CreateSession();
            Poco::Data::Statement insert(session);

            std::string role_str = role_.ToString();
            insert << INSERT_USER_REQUEST,
                    use(first_name_),
                    use(last_name_),
                    use(middle_name_),
                    use(email_),
                    use(gender_),
                    use(login_),
                    use(password_),
                    use(role_str);

            insert.execute();

            Poco::Data::Statement select(session);
            select << "SELECT LAST_INSERT_ID()",
                    into(id_),
                    range(0, 1); //  iterate over result set one row at a time

            if (!select.done()) {
                select.execute();
            }

            std::cout << "inserted:" << id_ << std::endl;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

} // namespace database