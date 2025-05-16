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
        std::string get_name() const;
        void        set_name(const std::string&);
        std::string get_password() const;
        void        set_password(const std::string&);
        std::string get_topic() const;
        void        set_topic(const std::string&);


    private:
        std::map<IRC_Client &, int> _clients;
        std::string                 _name;
        std::string                 _password;
        std::string                 _topic;
};

#endif