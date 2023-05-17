#include "../include/database/cache.h"

#include <exception>
#include <mutex>

#include <redis-cpp/stream.h>
#include <redis-cpp/execute.h>

namespace {

//    std::string SerializeUser(const database::User& user) {
//
//        std::string result;
//
//        result += user.GetFirstName() + ";";
//        result += user.GetLastName() + ";";
//        result += user.GetMiddleName() + ";";
//        result += user.GetEMail() + ";";
//        result += user.GetGender() + ";";
//        result += user.GetRole().ToString();
//
//        return result;
//    }
//
//    database::User DeserializeUser(std::string& string) {
//        std::stringstream test(string);
//        std::string segment;
//        std::vector<std::string> seglist;
//        seglist.reserve(6);
//
//        while(std::getline(test, segment, ';')) {
//            seglist.push_back(segment);
//        }
//
//        database::User user;
//
//        user.FirstName() = std::move(seglist[0]);
//        user.LastName()  = std::move(seglist[1]);
//        user.MiddleName() = std::move(seglist[2]);
//        user.EMail() = std::move(seglist[3]);
//        user.Role() = database::UserRole(seglist[4]);
//
//        return user;
//    }

} // namespace [ Functions ]

namespace database
{
    std::mutex _mtx;
    Cache::Cache() : _expiration(std::make_shared<std::string>("60")), _is_inited(false) {}

    void Cache::Init(const std::string& server_ip, unsigned int port, unsigned int expiration) {
        std::lock_guard<std::mutex> lck(_mtx);

        std::cout << "cache host:" << server_ip <<" port:" << port << std::endl;
        _stream = rediscpp::make_stream(server_ip, std::to_string(port));
        if (!_stream->good()) {
            std::cerr << "Error opening stream." << std::endl;
        }
        _expiration = std::make_shared<std::string>(std::to_string(expiration));
        _is_inited = true;
    }

    Cache* Cache::Get() {
        static Cache* instance;
        if ( !instance ) instance = new Cache();
        return instance;
    }

    void Cache::Put([[maybe_unused]] long id, [[maybe_unused]] const User& val) {
        std::lock_guard<std::mutex> lck(_mtx);

        assert(_stream != nullptr);

        std::string serialized = val.Serialize();
        rediscpp::value response = rediscpp::execute(*_stream, "set",
                                                     std::to_string(id),
                                                     serialized,
                                                     "ex", "60");
    }

    bool Cache::Get([[maybe_unused]] long id, [[maybe_unused]] User& val) {
        std::lock_guard<std::mutex> lck(_mtx);
        assert(_stream != nullptr);
        rediscpp::value response = rediscpp::execute(*_stream, "get", std::to_string(id));

        if (response.is_error_message())
            return false;
        if (response.empty())
            return false;

        try {
            auto serialized = response.as<std::string>();
            val.Deserialize(serialized);
        } catch ( const std::exception& e ) {
            return false;
        }

        return true;
    }
}