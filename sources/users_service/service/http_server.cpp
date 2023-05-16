#include "http_server.h"

#include "database/database.h"
#include "database/user.h"
#include "database/cache.h"

#include <iostream>

namespace search_service {

    HTTPWebServer::HTTPWebServer() : is_help_requested_(false) {}

    void HTTPWebServer::initialize(Poco::Util::Application &self) {
        loadConfiguration();
        ServerApplication::initialize(self);
    }

    void HTTPWebServer::uninitialize() {
        ServerApplication::uninitialize();
    }

    int HTTPWebServer::main(const std::vector<std::string> &args) {
        if (!is_help_requested_) {
            for (auto &arg: args) {
                std::cout << arg << std::endl;
            }

            if ( args.empty() ) {
                std::cerr << "Error. Application required the service config path as first command line argument." << std::endl;
                return 1;
            }

            std::cout << "Arguments: " << std::endl;
            for ( const auto& arg : args ) {
                std::cout << "\t" << arg << std::endl;
            }
            config_ = std::make_shared<search_service::Config>(args[0]);
            auto network_config = config_->GetNetworkConfig();
            auto caching_config = config_->GetCachingConfig();

            if ( !database::Database::Instance().IsConnected() ) {
                database::Database::Instance().BindConfigure(config_->GetDatabaseConfig());
                bool result = database::Database::Instance().TryConnect();
                if ( !result ) {
                    std::cerr << "Failed connect to database." << std::endl;
                    return EXIT_DATAERR;
                }
            }

            database::User::Init();
            database::Cache::Get()->Init(
                    caching_config->GetHost(),
                    caching_config->GetPort(),
                    caching_config->GetExpiration()
            );

            ServerSocket svs(Poco::Net::SocketAddress(network_config->GetIP(), network_config->GetPort()));
            HTTPServer srv(new HTTPRequestFactory(DateTimeFormat::SORTABLE_FORMAT), svs, new HTTPServerParams);
            srv.start();
            waitForTerminationRequest();
            srv.stop();
        }
        return Application::EXIT_OK;
    }

} // namespace search_service