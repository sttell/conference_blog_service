#include "database/user.h"

#include "database/database.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

#include <future>

#include "database/cache.h"

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

namespace {

    class DB_ID_Index {
        DB_ID_Index() = default;
    public:

        static DB_ID_Index FromExternID(long id) {
            DB_ID_Index index{};
            index.ext_id_ = id;
            auto max_shards = database::Database::GetMaxShard();

            index.shard_id_ = (id % max_shards == 0) ? 1 : 0;
            index.db_id_ = (index.ext_id_ + (max_shards - index.shard_id_ - 1)) / max_shards;

            return index;
        }

        static DB_ID_Index FromDBID(long id, size_t shard_id) {
            DB_ID_Index index{};
            index.db_id_ = id;
            index.shard_id_ = shard_id;

            auto max_shards = database::Database::GetMaxShard();
            index.ext_id_ = max_shards * id - (max_shards - shard_id - 1);

            return index;
        }

        size_t GetShard() const noexcept { return shard_id_; }
        long   GetDBID() const noexcept { return db_id_; }
        long   GetExternalID() const noexcept { return ext_id_;  }

    private:
        size_t shard_id_;
        long db_id_;
        long ext_id_;
    };

}

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

            for ( auto& hint : database::Database::GetAllHints() ) {
                Statement create_stmt(session);
                create_stmt << CREATE_TABLE_REQUEST << " " << hint.hint, now;

                std::cout << "DB create statement send: " <<  create_stmt.toString() << std::endl;
            }
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
            std::vector<User> result;

            User user;
            std::string sharding_hint_prefix = " -- sharding:";
            size_t num_hints = database::Database::GetMaxShard();

            for ( size_t hint = 0; hint < num_hints; hint++ ) {
                std::string sharding_hint = sharding_hint_prefix + std::to_string(hint);
                std::string select_str = SELECT_USER_REQUEST + sharding_hint;

                Statement select(session);

                std::string role_str;
                select << select_str, into(user.id_),
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

            std::vector<User> result;

            std::vector<ShardingHint> hints = database::Database::GetAllHints();

            std::vector<std::future<std::vector<User>>> futures;

            for ( const auto& hint : hints ) {

                auto handle = std::async(std::launch::async, [first_name, last_name, hint]() mutable -> std::vector<User>{
                    std::vector<User> result;

                    Poco::Data::Session session = database::Database::Instance().CreateSession();
                    Statement select(session);

                    std::string select_req = SELECT_BY_MASK_REQUEST;
                    select_req += hint.hint;

                    first_name += "%";
                    last_name += "%";

                    User user;
                    std::string role_str;
                    select << select_req,
                            into(user.id_),
                            into(user.first_name_),
                            into(user.last_name_),
                            into(user.middle_name_),
                            into(user.email_),
                            into(user.gender_),
                            into(role_str),
                            use(first_name),
                            use(last_name),
                            range(0, 1);

                    select.execute();

                    while (!select.done()) {
                        if (select.execute()) {
                            user.Role() = UserRole(role_str);
                            user.ID() = DB_ID_Index::FromDBID(user.id_, hint.shard_id).GetExternalID();
                            result.push_back(user);
                        }
                    }
                    return result;
                });

                futures.emplace_back(std::move(handle));
            }

            for ( std::future<std::vector<User>>& res : futures ) {
                std::vector<User> v = res.get();
                std::copy(std::begin(v), std::end(v), std::back_inserter(result));
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

            auto id_index = DB_ID_Index::FromExternID(id);
            auto internal_id = id_index.GetDBID();
            auto shard_id = id_index.GetShard();

            std::string query = SELECT_BY_ID_REQUEST + std::string(" -- sharding:") + std::to_string(shard_id);

            std::string role_str;
            select << query,
                    into(info.id_),
                    into(info.first_name_),
                    into(info.last_name_),
                    into(info.middle_name_),
                    into(info.email_),
                    into(info.gender_),
                    into(role_str),
                    use(internal_id); //  iterate over result set one row at a time


            size_t selected_rows = select.execute();
            if ( selected_rows > 0 ) {
                info.Role() = UserRole(role_str);
                info.ID() = id_index.GetExternalID();
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
            std::vector<ShardingHint> hints = database::Database::GetAllHints();

            std::vector<std::future<std::optional<User>>> futures;

            for ( const ShardingHint& hint : hints ) {

                auto handle = std::async(std::launch::async, [login, hint]() mutable -> std::optional<User> {

                    Poco::Data::Session session = database::Database::Instance().CreateSession();
                    Statement select(session);

                    std::string select_req = SELECT_BY_LOGIN_REQUEST;
                    select_req += hint.hint;

                    User user;
                    std::string role_str;
                    select << select_req,
                            into(user.id_),
                            into(user.first_name_),
                            into(user.last_name_),
                            into(user.middle_name_),
                            into(user.email_),
                            into(user.gender_),
                            into(role_str),
                            use(login),
                            range(0, 1);

                    size_t selected_rows = select.execute();
                    if ( selected_rows > 0 ) {
                        auto external_id = DB_ID_Index::FromDBID(user.id_, hint.shard_id).GetExternalID();

                        user.Role() = UserRole(role_str);
                        user.ID() = external_id;
                        std::cout << "User with login " << login << " found with ID " << user.id_ << std::endl;
                        return user;
                    }
                    return {};
                });

                futures.emplace_back(std::move(handle));
            }

            for ( std::future<std::optional<User>>& res : futures ) {
                std::optional<User> user_opt = res.get();
                if ( user_opt.has_value() ) {
                    return user_opt;
                }
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

    std::optional<User> User::FromCacheByID(long id) {
        std::optional<User> user;
        try {
            User user_obj;
            if (database::Cache::Get()->Get(id, user_obj)) {
                user = std::make_optional<User>(std::move(user_obj));
            }

        } catch (const std::exception& e) {
            std::cerr << "Read: Cache exception: " << e.what() << std::endl;
        }
        return user;
    }

    void User::SaveToCache() {

        try {
            database::Cache::Get()->Put(this->id_, *this);
        } catch (const std::exception& e) {
            std::cerr << "Save: Cache exception: " << e.what() << std::endl;
        }

    }

    std::optional<User> User::ChangeRole(std::string login, database::UserRole new_role) {
        try {
            Poco::Data::Session session = database::Database::Instance().CreateSession();
            Statement update(session);

            std::string new_role_str = new_role.ToString();
            ShardingHint sharding_hint = database::Database::UserShardingHint(login);

            std::string query = UPDATE_ROLE_REQUEST + std::string(" ") + sharding_hint.hint;

            update << query,
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

            std::vector<ShardingHint> hints = database::Database::GetAllHints();

            std::vector<std::future<std::optional<User>>> futures;

            for ( const ShardingHint& hint : hints ) {

                auto handle = std::async(std::launch::async, [login, password, hint]() mutable -> std::optional<User> {

                    Poco::Data::Session session = database::Database::Instance().CreateSession();
                    Statement select(session);

                    std::string select_req = SELECT_BY_CREDENTIALS_REQUEST;
                    select_req += hint.hint;

                    User user;
                    std::string role_str;
                    select << select_req,
                            into(user.id_),
                            into(user.first_name_),
                            into(user.last_name_),
                            into(user.middle_name_),
                            into(user.email_),
                            into(user.gender_),
                            into(role_str),
                            use(login),
                            use(password);

                    size_t selected_rows = select.execute();
                    if ( selected_rows > 0 ) {
                        auto external_id = DB_ID_Index::FromDBID(user.id_, hint.shard_id).GetExternalID();

                        user.Role() = UserRole(role_str);
                        user.ID() = external_id;

                        return user;
                    }
                    return {};
                });

                futures.emplace_back(std::move(handle));
            }

            for ( std::future<std::optional<User>>& res : futures ) {
                std::optional<User> user_opt = res.get();
                if ( user_opt.has_value() ) {
                    return user_opt;
                }
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
            ShardingHint sharding_hint = database::Database::UserShardingHint(login_);
            std::string insert_req = std::string(INSERT_USER_REQUEST) + " " + sharding_hint.hint;

            std::string role_str = role_.ToString();
            insert << insert_req,
                    use(first_name_),
                    use(last_name_),
                    use(middle_name_),
                    use(email_),
                    use(gender_),
                    use(login_),
                    use(password_),
                    use(role_str);

            size_t changes = insert.execute();
            if ( changes == 0 ) return;

            Poco::Data::Statement select(session);
            select << "SELECT LAST_INSERT_ID() " + sharding_hint.hint,
                    into(id_),
                    range(0, 1); //  iterate over result set one row at a time

            if (!select.done()) {
                select.execute();
            }

            auto extern_index = DB_ID_Index::FromDBID(id_, sharding_hint.shard_id);
            id_ = extern_index.GetExternalID();
            std::cout << "Inserted user with shard id " << sharding_hint.shard_id << " DB IDX: " << extern_index.GetDBID();
            std::cout << " External ID: " << id_ << std::endl;
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

    std::string User::Serialize() const {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", id_);
        root->set("first_name", first_name_);
        root->set("last_name", last_name_);
        root->set("middle_name", middle_name_);
        root->set("email", email_);
        root->set("gender", gender_);
        root->set("role", role_.ToString());

        std::stringstream ss;
        root->stringify(ss);
        return ss.str();
    }

    void User::Deserialize(const std::string& serialized) {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(serialized);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        this->ID()        = object->getValue<long>("id");
        this->FirstName() = object->getValue<std::string>("first_name");
        this->LastName()  = object->getValue<std::string>("last_name");
        this->EMail()     = object->getValue<std::string>("email");
        this->Gender()    = object->getValue<std::string>("gender");
        this->Role()      = UserRole(object->getValue<std::string>("role"));

        if ( object->has("middle_name") ) {
            this->MiddleName() = object->getValue<std::string>("middle_name");
        }
    }

} // namespace database