#ifndef SERVER_CACHE_H
#define SERVER_CACHE_H

#include <string>
#include <iostream>
#include <memory>
#include "user.h"

namespace database
{
    class Cache
    {
        Cache();

    public:
        static Cache* Get();
        void Init(const std::string& server_ip, unsigned int port, unsigned int expiration=60);

        void Put(long id, const User& val);
        bool Get(long id, User& val);

    private:
        std::shared_ptr<std::iostream> _stream;
        std::shared_ptr<std::string>   _expiration;
        bool _is_inited;

        void SaveToCache();
    };

} // namespace database


#endif //SERVER_CACHE_H
