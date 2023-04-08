#include "service/http_server.h"

int main(int argc, char*argv[])
{
    search_service::HTTPWebServer app;
    return app.run(argc, argv);
}
