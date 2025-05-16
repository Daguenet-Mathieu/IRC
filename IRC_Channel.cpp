#include "IRC_Channel.hpp"

//CONSTRUCTOR

IRC_Channel::IRC_Channel(const std::string& name, const std::string& password) : _name(name), _password(password)
{
}

//DESTRUCTOR

IRC_Channel::~IRC_Channel()
{
}


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
