#include "IRC_Client.hpp"

IRC_Client::IRC_Client(int socket_client, int ip): _socket_client(socket_client), _ip(ip), _client_info(false)
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

IRC_Client::IRC_Client(const IRC_Client &client): _socket_client(client._socket_client), _ip(client._ip), _client_info(false)
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

int     IRC_Client::get_socket_client(void) const
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

std::string    IRC_Client::get_username() const
{
    return (_username);
}

void    IRC_Client::set_nickname(const std::string &nickname)
{
    _nickname = nickname;
}

std::string    IRC_Client::get_nickname() const
{
    return (_nickname);
}

void    IRC_Client::set_client_info(bool value)
{
    _client_info = value;
}

bool    IRC_Client::get_client_info() const
{
    return (_client_info);
}

void    IRC_Client::close_socket() const
{
    close(_socket_client);
}

int    IRC_Client::get_ip() const
{
    return (_ip);
}

void    IRC_Client::fill_input_client(char *buffer, ssize_t size)
{
    _input_client.insert(_input_client.end(), buffer, buffer + size);
}

bool    IRC_Client::get_input_client(std::string &line)
{
    (void)line;
    int i = 0;
    for (; i < (int)_input_client.size(); i++)
    {
        if (_input_client[i] == '\n')
            break;
    }
    if (i >= (int)_input_client.size())
        return (false);
    line = std::string(_input_client.begin(), _input_client.begin() + i - 1);
    _input_client.erase(_input_client.begin(), _input_client.begin() + i + 1);
    return (true);
}

void     IRC_Client::set_output_client(const std::string &)
{

}

void    IRC_Client::set_output_client(const char *, int size)
{
    (void)size;
}

void    IRC_Client::send_output_client()
{

}