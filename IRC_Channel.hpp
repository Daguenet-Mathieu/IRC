#ifndef IRC_CHANNEL
#define IRC_CHANNEL

#include "IRC_Server.hpp"
#include "IRC_Client.hpp"

#include <vector>
#include <map>


class IRC_Client;

class IRC_Channel
{
    public:

    IRC_Channel(const std::string& name, const std::string& password);
    ~IRC_Channel();

    private:

    std::map<IRC_Client &, int> _clients;
    std::string                 _name;
    std::string                 _password;
    std::string                 _topic;
};

#endif