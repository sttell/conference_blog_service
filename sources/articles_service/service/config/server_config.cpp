#include "server_config.h"

#include "Poco/JSON/Parser.h"
#include <vector>
#include <cinttypes>
#include <fstream>

#include "path_validate.h"

namespace {

    constexpr const char* const  kDefaultIP   = "0.0.0.0";
    constexpr const unsigned int kDefaultPort = 8080;
    constexpr const char* const  kDefaultDB_Host = "127.0.0.1";
    constexpr const unsigned int kDefaultDB_Port = 3360;
    constexpr const char* const  kDefaultDB_Login = "admin";
    constexpr const char* const  kDefaultDB_Password = "admin";
    constexpr const char* const  kDefaultDB_Database = "archdb";

} // namespace [ Constants ]

namespace {

    template <typename ExpectedType>
    void JsonGetValue(Poco::JSON::Object& node, const char* key, ExpectedType& value) {
        if ( node.has(key) ) {
            auto str_representation = node.get(key).toString();
            if constexpr ( std::is_constructible_v<ExpectedType, std::string> ) {
                value = str_representation;
            }
            if constexpr ( std::is_integral_v<ExpectedType> ) {
                std::istringstream iss(str_representation);
                if constexpr ( std::is_unsigned_v<ExpectedType> ) {
                    uint64_t integral_value;
                    iss >> integral_value;
                    value = static_cast<ExpectedType>(integral_value);
                }
                if constexpr ( !std::is_unsigned_v<ExpectedType> ) {
                    int64_t integral_value;
                    iss >> integral_value;
                    value = static_cast<ExpectedType>(integral_value);
                }
            }
        }
    }

} // namespace [ Functions ]

namespace search_service {

    NetworkConfig::NetworkConfig() noexcept: ip_address_(kDefaultIP), port_(kDefaultPort) {}

    NetworkConfig::NetworkConfig(Poco::JSON::Object &json_root) noexcept: NetworkConfig() {
        JsonGetValue(json_root, "ip_address", ip_address_);
        JsonGetValue(json_root, "port", port_);
    }

    std::string NetworkConfig::GetIP() const noexcept { return ip_address_; }

    unsigned int NetworkConfig::GetPort() const noexcept { return port_; }

    void NetworkConfig::SetIP(const std::string &ip) noexcept {
        ip_address_ = ip;
    }

    void NetworkConfig::SetPort(unsigned int port) noexcept {
        port_ = port;
    }
} // namespace search_service

namespace search_service {

    DatabaseConfig::DatabaseConfig() noexcept:
            host_(kDefaultDB_Host),
            port_(kDefaultDB_Port),
            login_(kDefaultDB_Login),
            password_(kDefaultDB_Password),
            database_(kDefaultDB_Database) {}

    DatabaseConfig::DatabaseConfig(Poco::JSON::Object &json_root) noexcept: DatabaseConfig() {
        host_ = json_root.getValue<decltype(host_)>("host");
        port_ = json_root.getValue<decltype(port_)>("port");
        JsonGetValue(json_root, "login", login_);
        JsonGetValue(json_root, "password", password_);
        JsonGetValue(json_root, "database", database_);
    }

    void DatabaseConfig::SetHost(const std::string& host) noexcept { host_ = host; }

    void DatabaseConfig::SetPort(unsigned int port) noexcept { port_ = port; }

    void DatabaseConfig::SetLogin(const std::string& login) noexcept { login_ = login; }

    void DatabaseConfig::SetPassword(const std::string& password) noexcept { password_ = password; }

    void DatabaseConfig::SetDatabase(const std::string& database) noexcept { database_ = database; }

    std::string DatabaseConfig::GetHost() const noexcept { return host_; }

    unsigned int DatabaseConfig::GetPort() const noexcept { return port_; }

    std::string DatabaseConfig::GetLogin() const noexcept { return login_; }

    std::string DatabaseConfig::GetPassword() const noexcept { return password_; }

    std::string DatabaseConfig::GetDatabase() const noexcept { return database_; }

} // namespace search_service

namespace search_service {

    Config::Config(const std::string &path) :
        network_config_(nullptr), database_config_(nullptr) {

        config::utils::ValidateJsonPath(path);

        std::ifstream fin(path);
        if ( !fin.is_open() ) {
            throw std::runtime_error("Error opening server config file.");
        }

        Poco::JSON::Parser parser;
        auto parsing_result = parser.parse(fin);
        fin.close();

        auto root = parsing_result.extract<Poco::JSON::Object::Ptr>();

        if ( root->has("network") ) {
            network_config_ = std::make_shared<NetworkConfig>(*root->getObject("network"));
        } else {
            network_config_ = std::make_shared<NetworkConfig>();
        }
        if ( root->has("database") ) {
            database_config_ = std::make_shared<DatabaseConfig>(*root->getObject("database"));
        } else {
            database_config_ = std::make_shared<DatabaseConfig>();
        }
    }

    std::shared_ptr<NetworkConfig> Config::GetNetworkConfig() const noexcept { return network_config_; }

    std::shared_ptr<DatabaseConfig> Config::GetDatabaseConfig() const noexcept { return database_config_; }

} // namespace search_service