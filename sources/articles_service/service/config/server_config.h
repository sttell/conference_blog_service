#ifndef SEARCH_SERVICE_SERVER_CONFIG_H
#define SEARCH_SERVICE_SERVER_CONFIG_H

#include <string>
#include <memory>

namespace Poco::JSON {
    class Object;
}

namespace search_service {

    class NetworkConfig {
    public:
        NetworkConfig() noexcept;

        explicit NetworkConfig(Poco::JSON::Object& json_root) noexcept;

        void SetIP(const std::string&) noexcept;

        void SetPort(unsigned int port) noexcept;

        [[nodiscard]] std::string GetIP() const noexcept;

        [[nodiscard]] unsigned int GetPort() const noexcept;

    private:
        std::string ip_address_;
        unsigned int port_;
    };

    class DatabaseConfig {
    public:
        DatabaseConfig() noexcept;
        explicit DatabaseConfig(Poco::JSON::Object& json_root) noexcept;

        void SetHost(const std::string&) noexcept;
        void SetPort(unsigned int) noexcept;
        void SetLogin(const std::string&) noexcept;
        void SetPassword(const std::string&) noexcept;
        void SetDatabase(const std::string&) noexcept;

        std::string GetHost() const noexcept;
        unsigned int GetPort() const noexcept;
        std::string GetLogin() const noexcept;
        std::string GetPassword() const noexcept;
        std::string GetDatabase() const noexcept;

    private:
        std::string host_;
        unsigned int port_;
        std::string login_;
        std::string password_;
        std::string database_;
    };

    class Config {
    public:
        explicit Config(const std::string &path);

        Config(const Config &) = default;

        Config &operator=(const Config &) = default;

        Config(Config &&) = default;

        Config &operator=(Config &&) = default;

        [[nodiscard]] std::shared_ptr<NetworkConfig> GetNetworkConfig() const noexcept;

        [[nodiscard]] std::shared_ptr<DatabaseConfig> GetDatabaseConfig() const noexcept;

    private:
        std::shared_ptr<NetworkConfig> network_config_;
        std::shared_ptr<DatabaseConfig> database_config_;
    };

} // namespace search_service

#endif //SEARCH_SERVICE_SERVER_CONFIG_H
