#include "IRC_Channel.hpp"

//CONSTRUCTOR

IRC_Channel::IRC_Channel(const std::string& name, const std::string& password) : _name(name), _password(password), _invite(false), _t(false), _limit(-1)
{
}

//DESTRUCTOR

IRC_Channel::~IRC_Channel()
{
}


std::string IRC_Channel::get_name() const
{
    return (_name);
}

void        IRC_Channel::set_name(const std::string &name)
{
    _name = name;
}

std::string IRC_Channel::get_password() const
{
    return (_password);
}

void        IRC_Channel::set_password(const std::string &password)
{
    _password = password;
}

std::string IRC_Channel::get_topic() const
{
    return _topic;
}

bool        IRC_Channel::set_topic(const std::string &topic, const std::string& username)
{
    if (_clients.find(username) != _clients.end())
    {
        if (_t == false || _clients[username] <= OPERATOR)
        {
            _topic = topic;
            return true;
        }
    }
    return false;
}

bool    IRC_Channel::get_invite() const
{
    return _invite;
}

void    IRC_Channel::set_invite(const std::string& client)
{
    _invited.push_back(client);
}

bool    IRC_Channel::get_t() const
{
    return _t;
}

int    IRC_Channel::get_limit() const
{
    return _limit;
}

int IRC_Channel::get_nb_user() const
{
    return static_cast<int>(_clients.size());
}

void    IRC_Channel::set_client_status(const std::string& name, int status)
{
    _clients[name] = status;
}


int IRC_Channel::get_client_status(const std::string& username) const
{
    if (_clients.find(username) == _clients.end())
        return NONE;
    // std::cout << "client status" <<_clients.at(username) <<std::endl;
    return _clients.at(username);
}

std::vector<std::string>    IRC_Channel::get_channel_clients() const{
    std::vector<std::string>    clients;
    for (std::map<std::string, int>::const_iterator it = _clients.begin(); it != _clients.end(); it++){
        clients.push_back(it->first);
    }
    return clients;
}

std::vector<std::string>    IRC_Channel::get_formated_channel_clients() const{
    std::vector<std::string>    clients;
    for (std::map<std::string, int>::const_iterator it = _clients.begin(); it != _clients.end(); it++){
        std::string role = it->second <= OPERATOR ? "@" : "";
        clients.push_back(role + it->first);
    }
    return clients;
}

//FUNCTIONS

bool    IRC_Channel::is_invited(const std::string& client) const
{
    for(size_t i = 0; i < _invited.size(); i++)
    {
        if (_invited[i] == client)
            return true;
    }
    return false;
}

bool IRC_Channel::in_channel(const std::string& name) const{
	if (_clients.find(name) != _clients.end()) {
        return true;
    }
    return false;
}

void    IRC_Channel::remove_client(const std::string& name)
{
    std::cout << "nb_client avant : " << _clients.size() << std::endl;
    if (_clients.find(name) != _clients.end())
        _clients.erase(name);
    std::cout << "nb_client apres : " << _clients.size() << std::endl;
}

bool    IRC_Channel::mode_i(const std::string& client, const std::string& arg)
{
    std::map<std::string, int>::iterator it = _clients.find(client);

    if (it != _clients.end() && it->second <= OPERATOR)
    {
        if (arg == "+i")
            _invite = true;
        else
            _invite = false;
    }
    else
        return false;
    return true;
}

bool    IRC_Channel::mode_t(const std::string& client, const std::string& arg)
{
    std::map<std::string, int>::iterator it = _clients.find(client);

    if (it != _clients.end() && it->second <= OPERATOR)
    {
        if (arg == "+t")
            _t = true;
        else
            _t = false;
    }
    else
        return false;
    return true;
}

bool    IRC_Channel::mode_k(const std::string& client, const std::string& arg, const std::string& password)
{
    std::map<std::string, int>::iterator it = _clients.find(client);

    if (it != _clients.end() && it->second <= OPERATOR)
    {
        if (arg == "+k" && _password.size() == 0)
            _password = password;
        else
        {
            if (_password != password)
                return false;
            else
                _password.erase();
        }
    }
    else
        return false;
    return true;
}

bool    IRC_Channel::mode_o(const std::string& client, const std::string& arg, const std::string& target)
{
    std::map<std::string, int>::iterator it = _clients.find(client);
    std::map<std::string, int>::iterator it2 = _clients.find(target);

    if (it != _clients.end() && it->second <= OPERATOR && it->second <= it2->second)
    {
        if (arg == "+o")
            it2->second = OPERATOR;
        else
            it2->second = MEMBER;
    }
    else
        return false;
    return true;
}

bool    IRC_Channel::mode_l(const std::string& client, const std::string& arg, const int& limit)
{
    std::map<std::string, int>::iterator it = _clients.find(client);

    if (it != _clients.end() && it->second <= OPERATOR)
    {
        if (arg == "+l")
            _limit = limit;
        else
            _limit = -1;
    }
    else
        return false;
    return true;
}

void    IRC_Channel::update_nick(const std::string& old_nick, const std::string& new_nick)
{
    if (_clients.find(old_nick) == _clients.end())
        return ;
    _clients[new_nick] = _clients.at(old_nick);
    _clients.erase(old_nick);

}
