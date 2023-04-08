#include "database/article.h"

#include "database/database.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

#define TABLE_NAME "Articles"
#define CREATE_TABLE_REQUEST                                        \
    "CREATE TABLE IF NOT EXISTS `" TABLE_NAME "` "                  \
    "("                                                             \
        "`id` " "INT " "NOT NULL " "AUTO_INCREMENT, "               \
        "`consumer_id` " "INT " "NOT NULL, "                        \
        "`title` " "VARCHAR(256) " "NOT NULL, "                     \
        "`description` " "TEXT " "NOT NULL, "                       \
        "`content` " "TEXT " "NOT NULL, "                           \
        "`external_link` " "TEXT " "NULL, "                         \
        "`create_date` " "DATETIME " "DEFAULT CURRENT_TIMESTAMP, "  \
        "PRIMARY KEY (`id`)"                                        \
    ");"

#define SELECT_ALL_ID_REQUEST \
    "SELECT id FROM " TABLE_NAME

#define SELECT_BY_ID_REQUEST                                                                    \
    "SELECT id, consumer_id, title, description, content, external_link, create_date FROM "     \
    TABLE_NAME                                                                                  \
    " WHERE id=?"

#define INSERT_ARTICLE_REQUEST \
    "INSERT INTO " TABLE_NAME " " \
    "(consumer_id, title, description, content, external_link) " \
    "VALUES(?, ?, ?, ?, ?)"

#define DELETE_BY_ID_REQUEST \
    "DELETE FROM " TABLE_NAME " WHERE id=?"

namespace database {

    Article Article::FromJSON(const std::string &str) {
        Article article;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        article.ConsumerID() = object->getValue<long>("consumer_id");
        article.Title() = object->getValue<std::string>("title");
        article.Description() = object->getValue<std::string>("description");
        article.Content() = object->getValue<std::string>("content");
        if ( object->has("external_link") ) {
            article.ExternalLink() = object->getValue<std::string>("external_link");
        }

        return article;
    }

    long Article::GetID() const noexcept { return id_; }
    long Article::GetConsumerID() const noexcept { return consumer_id_; }
    const std::string &Article::GetTitle() const noexcept { return title_; }
    const std::string &Article::GetDescription() const noexcept { return description_; }
    const std::string &Article::GetContent() const noexcept { return content_; }
    const std::string &Article::GetCreateDate() const noexcept { return create_date_; }
    const std::string &Article::GetExternalLink() const noexcept { return external_link_; }

    long &Article::ID() noexcept { return id_; }
    long &Article::ConsumerID() noexcept { return consumer_id_; }
    std::string &Article::Title() noexcept { return title_; }
    std::string &Article::Description() noexcept { return description_; }
    std::string &Article::Content() noexcept { return content_; }
    std::string &Article::ExternalLink() noexcept { return external_link_; }

    void Article::Init() {
        try {
            Poco::Data::Session session = database::Database::Instance().CreateSession();
            Statement create_statement(session);

            create_statement << CREATE_TABLE_REQUEST, now;
        } catch (Poco::Data::MySQL::ConnectionException& e) {
            std::cerr << "Connection to database failed: " << e.what() << std::endl;
            throw;
        } catch (Poco::Data::MySQL::StatementException &e) {
            std::cerr << "Statement exception: " << e.what() << std::endl;
            throw;
        }
    }

    std::vector<long> Article::ReadAll() {
        try
        {
            Poco::Data::Session session = database::Database::Instance().CreateSession();
            Statement select(session);
            std::vector<long> result;

            long id;
            select << SELECT_ALL_ID_REQUEST,
                    into(id),
                    range(0, 1); //  iterate over result set one row at a time

            while (!select.done()) {
                if (select.execute()) {
                    result.push_back(id);
                }
            }

            return result;
        }
        catch (Poco::Data::MySQL::ConnectionException &e) {
            std::cerr << "Connection to DB error: " << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e) {
            std::cerr << "Statement error: " << e.what() << std::endl;
            throw;
        }
    }

    std::optional<Article> Article::SearchByID(long id) {
        try {
            Poco::Data::Session session = database::Database::Instance().CreateSession();
            Statement select(session);

            Article article;

            Poco::DateTime create_date;
            select << SELECT_BY_ID_REQUEST,
                    into(article.id_),
                    into(article.consumer_id_),
                    into(article.title_),
                    into(article.description_),
                    into(article.content_),
                    into(article.external_link_),
                    into(create_date),
                    use(id);

            size_t selected_rows = select.execute();

            Poco::DateTimeFormatter formatter;
            article.create_date_ = formatter.format(create_date, "%f %b %Y, %H:%M:%S");

            if ( selected_rows > 0 ) {
                return article;
            }

            return { };
        }

        catch (Poco::Data::MySQL::ConnectionException &e) {
            std::cerr << "Connection to DB error: " << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e) {
            std::cerr << "Statement error: " << e.what() << std::endl;
            throw;
        }
    }

    bool Article::DeleteByID(long id) {
        try {
            Poco::Data::Session session = database::Database::Instance().CreateSession();
            Statement delete_stm(session);

            delete_stm << DELETE_BY_ID_REQUEST, use(id);

            size_t deleted_rows = delete_stm.execute();

            if ( deleted_rows > 0 ) {
                return true;
            }

            return false;
        }

        catch (Poco::Data::MySQL::ConnectionException &e) {
            std::cerr << "Connection to DB error: " << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e) {
            std::cerr << "Statement error: " << e.what() << std::endl;
            throw;
        }
    }

    void Article::InsertToDatabase() {
        try
        {
            Poco::Data::Session session = database::Database::Instance().CreateSession();
            Poco::Data::Statement insert(session);

            insert << INSERT_ARTICLE_REQUEST,
                    use(consumer_id_),
                    use(title_),
                    use(description_),
                    use(content_),
                    use(external_link_);

            insert.execute();

            Poco::Data::Statement select(session);
            select << "SELECT LAST_INSERT_ID()",
                    into(id_),
                    range(0, 1); //  iterate over result set one row at a time

            if (!select.done()) {
                select.execute();
            }

            std::cout << "Inserted to DB: " << id_ << std::endl;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cerr << "Connection to DB error: " << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cerr << "Statement error: " << e.what() << std::endl;
            throw;
        }
    }

    Poco::JSON::Object::Ptr Article::ToJSON() const {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", id_);
        root->set("consumer_id", consumer_id_);
        root->set("title", title_);
        root->set("description", description_);
        root->set("content", content_);
        if ( !external_link_.empty() )
            root->set("external_link", external_link_);
        root->set("create_date", create_date_);

        return root;
    }

} // namespace database