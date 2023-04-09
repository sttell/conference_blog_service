#ifndef SEARCH_SERVICE_DATABASE_H
#define SEARCH_SERVICE_DATABASE_H

#include <string>
#include <memory>
#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/SessionPool.h>

namespace search_service { class DatabaseConfig; }

namespace database {

    struct ShardingHint {
        std::string hint;
        long shard_id;
    };

    class Database{
    private:
        Database();

    public:
        static Database& Instance();

        void BindConfigure(std::shared_ptr<search_service::DatabaseConfig> config);
        bool TryConnect();
        bool IsConnected() const noexcept;

        Poco::Data::Session CreateSession();

        static size_t GetMaxShard();
        static ShardingHint UserShardingHint(const std::string& login);
        static std::vector<ShardingHint> GetAllHints();

    private:
        bool is_connected_;
        std::string connection_string_;
        std::unique_ptr<Poco::Data::SessionPool> pool_;
        std::shared_ptr<search_service::DatabaseConfig> config_;
    };

} // namespace database

#endif // SEARCH_SERVICE_DATABASE_H
