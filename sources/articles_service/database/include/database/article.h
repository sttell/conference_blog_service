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
        [[nodiscard]] long               GetConsumerID() const noexcept;
        [[nodiscard]] const std::string& GetTitle() const noexcept;
        [[nodiscard]] const std::string& GetDescription() const noexcept;
        [[nodiscard]] const std::string& GetContent() const noexcept;
        [[nodiscard]] const std::string& GetExternalLink() const noexcept;
        [[nodiscard]] const std::string& GetCreateDate() const noexcept;

        long&        ID() noexcept;
        long&        ConsumerID() noexcept;
        std::string& Title() noexcept;
        std::string& Description() noexcept;
        std::string& Content() noexcept;
        std::string& ExternalLink() noexcept;

        static void Init();

        static std::optional<Article> SearchByID(long id);
        static std::vector<long> ReadAll();
        static bool DeleteByID(long id);

        void InsertToDatabase();

        [[nodiscard]] Poco::JSON::Object::Ptr ToJSON() const;

    private:
        long id_{ -1 };
        long consumer_id_{ -1 };
        std::string title_;
        std::string description_;
        std::string content_;
        std::string external_link_;
        std::string create_date_;
    };

} // namespace database

#endif //SERVER_DATABASE_ARTICLE_H
