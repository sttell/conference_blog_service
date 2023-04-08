#ifndef SERVER_DATABASE_ARTICLE_H
#define SERVER_DATABASE_ARTICLE_H

#include <Poco/JSON/Object.h>
#include <string>
#include <optional>

namespace database {

    class Article {
        friend class Database;
    public:
        static Article FromJSON(const std::string & str);

        [[nodiscard]] long               GetID() const noexcept;
        [[nodiscard]] long               GetArticleID() const noexcept;
        [[nodiscard]] long               GetAcceptorID() const noexcept;
        [[nodiscard]] const std::string& GetAcceptDate() const noexcept;

        long& AcceptorID() noexcept;
        long& ArticleID() noexcept;

        static void Init();

        static std::optional<Article> SearchByID(long id);
        static std::vector<Article> ReadAll();
        static bool DeleteByID(long id);

        void InsertToDatabase();

        [[nodiscard]] Poco::JSON::Object::Ptr ToJSON() const;

    private:
        long id_{ -1 };
        long article_id_{ -1 };
        long acceptor_id_{ -1 };
        std::string accept_date_;
    };

} // namespace database

#endif //SERVER_DATABASE_ARTICLE_H
