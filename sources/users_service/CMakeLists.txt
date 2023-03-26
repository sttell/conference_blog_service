cmake_minimum_required(VERSION 3.2)

SET (PROJECT_NAME users_service)
SET (EXECUTABLE_NAME ${PROJECT_NAME})

project(${PROJECT_NAME} C CXX)

SET (CMAKE_CXX_STANDARD 17)
SET (CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL)
find_package(Threads)
find_package(ZLIB)
find_package(Poco REQUIRED COMPONENTS Foundation Util Net XML JSON Crypto NetSSL)

if(NOT ${Poco_FOUND})
    message(FATAL_ERROR "Poco C++ Libraries not found.")
endif()

include_directories(${Poco_INCLUDE_DIRS})
include_directories("/usr/include/mysql")
include_directories("/usr/local/include/mysql")
link_directories("/usr/local/lib")

add_executable(${EXECUTABLE_NAME}
        main.cpp

        database/src/database.cpp
        database/src/user.cpp
        database/src/user_role.cpp

        service/config/path_validate.cpp
        service/config/server_config.cpp
        service/handlers/interface/handler_factory.cpp
        service/handlers/interface/i_request_handler.cpp
        service/handlers/auth/auth_handler.cpp
        service/handlers/search/search_handler.cpp
        service/handlers/user/user_handler.cpp

        service/http_server.cpp
        ../shared/errors.cpp
        )

target_include_directories(${EXECUTABLE_NAME} PRIVATE "${CMAKE_BINARY_DIR}")
target_include_directories(${EXECUTABLE_NAME} PRIVATE "../shared")
target_include_directories(${EXECUTABLE_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/database/include")

target_compile_options(${EXECUTABLE_NAME} PRIVATE -Wall -Wextra -pedantic -Werror )
set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${EXECUTABLE_NAME} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE
        ${CMAKE_THREAD_LIBS_INIT}
        ${Poco_LIBRARIES}
        "PocoData"
        "PocoDataMySQL"
        "mysqlclient"
        ZLIB::ZLIB)

# Add directory with service data
add_custom_target(
        TARGET ${PROJECT_NAME}
        POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_CURRENT_BINARY_DIR}/users_service_data
)

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_LIST_DIR}/service/server_config.json
        ${CMAKE_CURRENT_BINARY_DIR}/users_service_data/server_config.json
)
