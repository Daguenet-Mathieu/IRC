#ifndef IRC_CLIENT
#define IRC_CLIENT
#include <vector>
#include "IRC_Server.hpp"
// #include "IRC_Channel.hpp"??

class IRC_Client{
    public:
//geeter setter
    private:
        std::string         _username;
        std::string         _nickname;//??doublon
        int                 _socket_client;
        struct	sockaddr_in	_client_addr;
        // std::string         _current_channel;
        // std::vector<std::string>    _channels_name;
        
};

#endif