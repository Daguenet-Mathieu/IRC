#ifndef IRC_CHANNEL
#define IRC_CHANNEL

#include "IRC_Server.hpp"
#include "IRC_Client.hpp"

#include <vector>
#include <map>

enum channel_role {
    OPERATOR,
    MEMBER
};


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
        bool        in_channel(const std::string&);
        std::vector<std::string>    get_channel_clients();
        std::vector<std::string>    get_formated_channel_clients();
        void                        set_client_status(const std::string&, int);
        void                        remove_client(const std::string&);
//execute action mode + client name bool

    private:
        std::map<std::string, int>      _clients;
        std::string                 	_name;
        std::string                 	_password;
        std::string                 	_topic;
};

#endif
