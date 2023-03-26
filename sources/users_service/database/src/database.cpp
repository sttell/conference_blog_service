#include "database/database.h"

#include "../../service/config/server_config.h"

#include "Poco/Data/Transaction.h"
#include "Poco/Data/Binding.h"

#include <sstream>

using Poco::Data::Keywords::use;
using Poco::Data::Keywords::into;
using Poco::Data::Keywords::range;

namespace database{

    Database::Database() : is_connected_(false) {}

    Database& Database::Instance(){
        static Database _instance;
        return _instance;
    }

    void Database::BindConfigure(std::shared_ptr<search_service::DatabaseConfig> config) {
        config_ = std::move(config);
    }

    bool Database::IsConnected() const noexcept { return is_connected_; }

    bool Database::TryConnect() {
        try {
            connection_string_ += "host=" + config_->GetHost() + ";";
            connection_string_ += "user=" + config_->GetLogin() + ";";
            connection_string_ += "db=" + config_->GetDatabase() + ";";
            connection_string_ += "port=" + std::to_string(config_->GetPort()) + ";";
            connection_string_ += "password=" + config_->GetPassword() + ";";

            std::cout << "Try connect to database. Connection request:\n\t" << connection_string_ << std::endl;
            Poco::Data::MySQL::Connector::registerConnector();
            pool_ = std::make_unique<Poco::Data::SessionPool>(Poco::Data::MySQL::Connector::KEY, connection_string_);
            is_connected_ = true;
            return true;
        } catch (...) {
            return false;
        }
    }

    Poco::Data::Session Database::CreateSession(){
        return Poco::Data::Session(pool_->get());
    }

}
