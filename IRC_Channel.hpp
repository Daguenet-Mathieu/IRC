#ifndef IRC_CHANNEL_HPP
#define IRC_CHANNEL_HPP

#include "IRC_Server.hpp"
#include "IRC_Client.hpp"

#include <vector>
#include <map>

enum channel_role {
    SUPER_OPERATEUR,
    OPERATOR,
    MEMBER
};


class IRC_Client;

class IRC_Channel
{
    public:
        IRC_Channel(const std::string& name, const std::string& password);
        ~IRC_Channel();
        std::string                 get_name() const;
        void                        set_name(const std::string&);
        std::string                 get_password() const;
        void                        set_password(const std::string&);
        std::string                 get_topic() const;
        void                        set_topic(const std::string&);
        std::vector<std::string>    get_channel_clients() const;
        std::vector<std::string>    get_formated_channel_clients() const;
        bool                        get_invite() const;
        bool                        get_t() const;
        int                         get_limit() const;
        int                         get_nb_user() const;
        bool                        is_invited(const std::string& client) const;
        void                        set_client_status(const std::string&, int);
        bool                        in_channel(const std::string&) const;
        void                        remove_client(const std::string&);
        bool                        mode_i(const std::string&, const std::string&);
        bool                        mode_t(const std::string&, const std::string&);
        bool                        mode_k(const std::string&, const std::string&, const std::string&);
        bool                        mode_o(const std::string&, const std::string&, const std::string&);
        bool                        mode_l(const std::string&, const std::string&, const int&);
//execute ation mode + client name bool

    private:
        std::map<std::string, int>      _clients;
        std::vector<std::string>        _invited;
        std::string                 	_name;
        std::string                 	_password;
        std::string                 	_topic;
        bool                            _invite;
        bool                            _t; 
        int                             _limit;
    };

#endif
