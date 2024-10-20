#ifndef IRC_CHANNEL
#define IRC_CHANNEL
#include <vector>
#include "IRC_Server.hpp"
#include "IRC_Client.hpp"

class IRC_Channel{
    public:

    private:
        std::vector<std::string> _operators;
        std::vector<IRC_Client> _clients;
        std::string             _password;//si vide pas de mdp
        std::string             _topic;
};

#endif