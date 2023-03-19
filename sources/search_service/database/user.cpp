#include "user.h"

#include "database.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

#include <sstream>
#include <exception>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database
{

    void User::init()
    {
        try
        {
            Poco::Data::Session session = database::Database::Instance().CreateSession();
            Statement create_stmt(session);
            create_stmt << "CREATE TABLE IF NOT EXISTS `UserData` "
                        << "(`id` INT NOT NULL AUTO_INCREMENT,"
                        << "`login` VARCHAR(256) NOT NULL,"
                        << "`first_name` VARCHAR(256) NOT NULL,"
                        << "`last_name` VARCHAR(256) NOT NULL,"
                        << "`second_name` VARCHAR(256) NOT NULL,"
                        << "`email` VARCHAR(256) NOT NULL,"
                        << "`gender` INT NULL,"
                        << "PRIMARY KEY (`id`), "
                        << "KEY `logn` (`login`), "
                        << "KEY `fn` (`first_name`), "
                        << "KEY `ln` (`last_name`));",
                    now;
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

    Poco::JSON::Object::Ptr User::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("first_name", _first_name);
        root->set("last_name", _last_name);
        root->set("second_name", _second_name);
        root->set("email", _email);
        root->set("login", _login);
        root->set("gender", _gender);

        return root;
    }

    User User::fromJSON(const std::string &str)
    {
        User user;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        user.id() = object->getValue<long>("id");
        user.first_name() = object->getValue<std::string>("first_name");
        user.last_name() = object->getValue<std::string>("last_name");
        user.second_name() = object->getValue<std::string>("second_name");
        user.email() = object->getValue<std::string>("email");
        user.login() = object->getValue<std::string>("login");
        user.gender() = object->getValue<int>("gender");

        return user;
    }

    std::optional<User> User::read_by_id(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::Instance().CreateSession();
            Poco::Data::Statement select(session);
            User a;
            select << "SELECT id, login, first_name, last_name, second_name, email, gender FROM UserData where id=?",
                    into(a._id),
                    into(a._login),
                    into(a._first_name),
                    into(a._last_name),
                    into(a._second_name),
                    into(a._email),
                    into(a._gender),
                    use(id),
                    range(0, 1); //  iterate over result set one row at a time

            select.execute();
            Poco::Data::RecordSet rs(select);
            if (rs.moveFirst()) return a;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;

        }
        return {};
    }

    std::vector<User> User::read_all()
    {
        try
        {
            Poco::Data::Session session = database::Database::Instance().CreateSession();
            Statement select(session);
            std::vector<User> result;
            User a;
            select << "SELECT id, login, first_name, last_name, second_name, email, gender FROM UserData",
                    into(a._id),
                    into(a._login),
                    into(a._first_name),
                    into(a._last_name),
                    into(a._second_name),
                    into(a._email),
                    into(a._gender),
                    range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if (select.execute())
                    result.push_back(a);
            }
            return result;
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

    std::vector<User> User::search(std::string first_name, std::string last_name)
    {
        try
        {
            Poco::Data::Session session = database::Database::Instance().CreateSession();
            Statement select(session);
            std::vector<User> result;
            User a;
            first_name += "%";
            last_name += "%";
            select << "SELECT id, login, first_name, last_name, second_name, email, gender FROM UserData where first_name LIKE ? and last_name LIKE ?",
                    into(a._id),
                    into(a._login),
                    into(a._first_name),
                    into(a._last_name),
                    into(a._second_name),
                    into(a._email),
                    into(a._gender),
                    use(first_name),
                    use(last_name),
                    range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if (select.execute())
                    result.push_back(a);
            }
            return result;
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

    std::optional<User> User::search(const std::string& user_login) {
        try
        {
            std::string login = user_login;
            Poco::Data::Session session = database::Database::Instance().CreateSession();
            Statement select(session);
            std::optional<User> result;
            User a;
            select << "SELECT id, login, first_name, last_name, second_name, email, gender FROM UserData WHERE login = ?",
                    into(a._id),
                    into(a._login),
                    into(a._first_name),
                    into(a._last_name),
                    into(a._second_name),
                    into(a._email),
                    into(a._gender),
                    use(login),
                    range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if (select.execute())
                    result.emplace(a);
            }
            return result;
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

    void User::save_to_mysql()
    {

        try
        {
            Poco::Data::Session session = database::Database::Instance().CreateSession();
            Poco::Data::Statement insert(session);

            insert << "INSERT INTO UserData (login, first_name, last_name, email, gender) VALUES(?, ?, ?, ?, ?, ?)",
                    use(_login),
                    use(_first_name),
                    use(_last_name),
                    use(_email),
                    use(_gender);

            insert.execute();

            Poco::Data::Statement select(session);
            select << "SELECT LAST_INSERT_ID()",
                    into(_id),
                    range(0, 1); //  iterate over result set one row at a time

            if (!select.done())
            {
                select.execute();
            }

            if ( !_second_name.empty() ) {
                Poco::Data::Statement update(session);
                update << "UPDATE UserData SET second_name = ? WHERE id = ?",
                        use(_second_name),
                        use(_id);
                update.execute();

                if (!update.done()) {
                    update.execute();
                }
            }

            std::cout << "inserted:" << _id << std::endl;
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

    const std::string &User::get_login() const
    {
        return _login;
    }

    std::string &User::login()
    {
        return _login;
    }

    long User::get_id() const
    {
        return _id;
    }

    const std::string &User::get_first_name() const
    {
        return _first_name;
    }

    const std::string &User::get_last_name() const
    {
        return _last_name;
    }

    const std::string &User::get_email() const
    {
        return _email;
    }

    int &User::gender() {
        return _gender;
    }

    long &User::id()
    {
        return _id;
    }

    std::string &User::first_name()
    {
        return _first_name;
    }

    std::string &User::last_name()
    {
        return _last_name;
    }

    std::string &User::email()
    {
        return _email;
    }

    int User::get_gender() const {
        return _gender;
    }

    const std::string &User::get_second_name() const {
        return _second_name;
    }

    std::string &User::second_name() {
        return _second_name;
    }
}