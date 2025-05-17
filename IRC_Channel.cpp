#include "IRC_Channel.hpp"

//CONSTRUCTOR

IRC_Channel::IRC_Channel(const std::string& name, const std::string& password) : _name(name), _password(password)
{
}

//DESTRUCTOR

IRC_Channel::~IRC_Channel()
{
}

//FUNCTIONS

std::string IRC_Channel::get_name() const{
    return (_name);
}

void        IRC_Channel::set_name(const std::string &name){
    _name = name;
}

std::string IRC_Channel::get_password() const{
    return (_password);
}

void        IRC_Channel::set_password(const std::string &password){
    _password = password;
}

std::string IRC_Channel::get_topic() const{
    return _topic;
}

void        IRC_Channel::set_topic(const std::string &topic){
    _topic = topic;
}

bool IRC_Channel::in_channel(const std::string& name){
	if (_clients.find(name) != _clients.end()) {
        return true;
    }
    return false;
}

std::vector<std::string>    IRC_Channel::get_channel_clients(){
    std::vector<std::string>    clients;
    for (std::map<std::string, int>::iterator it = _clients.begin(); it != _clients.end(); it++){
        clients.push_back(it->first);
    }
    return clients;
}

void    IRC_Channel::set_client_status(const std::string& name, int status)
{
    _clients[name] = status;
}

void    IRC_Channel::remove_client(const std::string& name)
{
    if (_clients.find(name) != _clients.end())
        _clients.erase(name);
}