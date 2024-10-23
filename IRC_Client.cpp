#include "IRC_Client.hpp"

IRC_Client::IRC_Client(int socket_client): _socket_client(socket_client), _client_confirmation(false)
{
    // std::cout<<"cocket constructeur == "<< socket_client <<" socket dans le client : " << _socket_client<<std::endl;
    // _socket_client = socket(AF_INET, SOCK_STREAM, 0);
	// if (_socket_client== -1)
	// 	throw ThrowException("ERROR SOCKET");
	// memset(&_client_addr, 0, sizeof(_client_addr));
	// _client_addr.sin_family = AF_INET;
	// _client_addr.sin_addr.s_addr = INADDR_ANY;
	// _client_addr.sin_port = htons(port);
}

IRC_Client::IRC_Client(const IRC_Client &client): _socket_client(client._socket_client),  _client_confirmation(false)
{}

IRC_Client::~IRC_Client()
{
    // std::cout<<"client destructeur"<<std::endl;
    // if (_socket_client != -1)
    //     close(_socket_client);
}


// struct	sockaddr_in IRC_Client::get_client_addr(void)
// {
//     return (_client_addr);
// }

int     IRC_Client::get_socket_client(void)
{
    // std::cout<<"dans le getter "<<_socket_client<<std::endl;
    return (_socket_client);
}

void    IRC_Client::set_socket_client(int socket_client)
{
    // std::cout << "socket client : " << socket_client << std::endl;
    _socket_client = socket_client;
}


void    IRC_Client::set_username(const std::string &username)
{
    _username = username;
}

std::string    IRC_Client::get_username()
{
    return (_username);
}

void    IRC_Client::set_nickname(const std::string &nickname)
{
    _nickname = nickname;
}

std::string    IRC_Client::get_nickname()
{
    return (_nickname);
}

void    IRC_Client::set_client_confirmation(bool value)
{
    _client_confirmation = value;
}

bool    IRC_Client::get_client_confirmation()
{
    return (_client_confirmation);
}
