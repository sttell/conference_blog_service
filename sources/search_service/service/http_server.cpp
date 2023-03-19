#include "http_server.h"

#include "../database/database.h"

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

            config_ = std::make_shared<search_service::Config>("/home/ilya/work/MAI/conference_blog_service/sources/search_service/server_config.json");
            auto network_config = config_->GetNetworkConfig();
            if ( !database::Database::Instance().IsConnected() ) {
                database::Database::Instance().BindConfigure(config_->GetDatabaseConfig());
                bool result = database::Database::Instance().TryConnect();
                if ( !result ) {
                    std::cerr << "Failed connect to database." << std::endl;
                    return EXIT_DATAERR;
                }
            }
            database::User::init();
            ServerSocket svs(Poco::Net::SocketAddress(network_config->GetIP(), network_config->GetPort()));
            HTTPServer srv(new HTTPRequestFactory(DateTimeFormat::SORTABLE_FORMAT), svs, new HTTPServerParams);
            srv.start();
            waitForTerminationRequest();
            srv.stop();
        }
        return Application::EXIT_OK;
    }

} // namespace search_service