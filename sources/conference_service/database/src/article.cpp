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

#define TABLE_NAME "AcceptedArticles"
#define CREATE_TABLE_REQUEST                                        \
    "CREATE TABLE IF NOT EXISTS `" TABLE_NAME "` "                  \
    "("                                                             \
        "`id` " "INT " "NOT NULL " "AUTO_INCREMENT, "               \
        "`article_id` " "INT " "NOT NULL,"                                  \
        "`acceptor_id` " "INT " "NOT NULL, "                        \
        "`accept_date` " "DATETIME " "DEFAULT CURRENT_TIMESTAMP, "  \
        "PRIMARY KEY (`id`)"                                        \
    ");"

#define SELECT_ALL_ID_REQUEST \
    "SELECT id, article_id, acceptor_id, accept_date FROM " TABLE_NAME

#define SELECT_BY_ID_REQUEST SELECT_ALL_ID_REQUEST " WHERE id=?"

#define INSERT_ARTICLE_REQUEST \
    "INSERT INTO " TABLE_NAME " " \
    "(article_id, acceptor_id) " \
    "VALUES(?, ?)"

#define DELETE_BY_ID_REQUEST \
    "DELETE FROM " TABLE_NAME " WHERE id=?"

namespace database {

    Article Article::FromJSON(const std::string &str) {
        Article article;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        article.AcceptorID() = object->getValue<long>("acceptor_id");
        article.ArticleID() = object->getValue<long>("article_id");

        return article;
    }

    long Article::GetID() const noexcept { return id_; }
    long Article::GetAcceptorID() const noexcept { return acceptor_id_; }
    long Article::GetArticleID() const noexcept { return article_id_; }
    const std::string &Article::GetAcceptDate() const noexcept { return accept_date_; }

    long &Article::AcceptorID() noexcept { return acceptor_id_; }
    long &Article::ArticleID() noexcept { return article_id_; }

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

    std::vector<Article> Article::ReadAll() {
        try
        {
            Poco::Data::Session session = database::Database::Instance().CreateSession();
            Statement select(session);

            Article article;
            std::vector<Article> result;

            Poco::DateTime accept_date;
            select << SELECT_ALL_ID_REQUEST,
                    into(article.id_),
                    into(article.article_id_),
                    into(article.acceptor_id_),
                    into(accept_date),
                    range(0, 1); //  iterate over result set one row at a time

            while (!select.done()) {
                if (select.execute()) {
                    Poco::DateTimeFormatter formatter;
                    article.accept_date_ = formatter.format(accept_date, "%f %b %Y, %H:%M:%S");
                    result.push_back(article);
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

            Poco::DateTime accept_date;
            select << SELECT_BY_ID_REQUEST,
                    into(article.id_),
                    into(article.article_id_),
                    into(article.acceptor_id_),
                    into(accept_date),
                    use(id);

            size_t selected_rows = select.execute();

            Poco::DateTimeFormatter formatter;
            article.accept_date_ = formatter.format(accept_date, "%f %b %Y, %H:%M:%S");

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

            insert << INSERT_ARTICLE_REQUEST, use(article_id_), use(acceptor_id_);

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
        root->set("article_id", article_id_);
        root->set("acceptor_id", acceptor_id_);
        root->set("accept_date", accept_date_);

        return root;
    }

} // namespace database