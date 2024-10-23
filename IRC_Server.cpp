#include "IRC_Server.hpp"
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
//CONSTRUCTOR

IRC_Server::IRC_Server() : _port(0)
{
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket == -1)
		throw ThrowException("ERROR SOCKET");
	memset(&_server_addr, 0, sizeof(_server_addr));
	_server_addr.sin_family = AF_INET;
	_server_addr.sin_addr.s_addr = INADDR_ANY;
	_server_addr.sin_port = htons(_port);
}

IRC_Server::IRC_Server(int port, std::string const& password) : _port(port), _password(password)
{
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket == -1)
		throw ThrowException("ERROR SOCKET");
	// std::cout<<"socket fd = "<<_socket<<std::endl;
	memset(&_server_addr, 0, sizeof(_server_addr));
	_server_addr.sin_family = AF_INET;
	_server_addr.sin_addr.s_addr = INADDR_ANY;
	_server_addr.sin_port = htons(_port);
}

IRC_Server::IRC_Server(IRC_Server const& src) : _password(src._password), _server_addr(src._server_addr)
{
	this->_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_socket == -1)
		throw ThrowException("ERROR SOCKET");
}

//DESTRUCTOR

IRC_Server::~IRC_Server()
{
	if (_socket != -1)
		close(_socket);
}

//OPERATOR

IRC_Server&	IRC_Server::operator=(IRC_Server const& rhs)
{
	if (this != &rhs)
	{
		this->_port = rhs._port;
		this->_password = rhs._password;
		if (this->_socket != -1)
			close(this->_socket);
		this->_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (this->_socket == -1)
			throw ThrowException("SOCKET ERROR");
		memset(&this->_server_addr, 0, sizeof(this->_server_addr));
		this->_server_addr.sin_family = AF_INET;
		this->_server_addr.sin_addr.s_addr = INADDR_ANY;
		this->_server_addr.sin_port = htons(this->_port);
	}
	return *this;
}

//MEMBER FUNCTIONS

// void	IRC_Server::manage()
// {
// 	if (bind(_socket, (struct sockaddr *)&_server_addr, sizeof(_server_addr)) == -1)
// 		throw ThrowException("BIND ERROR");
// 	if (listen(_socket, 3) == -1)
// 		throw ThrowException("LISTEN ERROR");

// 	fd_set	readfds;
// 	int		activity;
// 	char	buffer[1024];
// //vecteur de client quand accepte ok push nouveau client dans vecgeur de client
// 	while (true)
// 	{
// 		memset(&buffer, 0, sizeof(buffer));
// 		FD_ZERO(&readfds);
// 		FD_SET(_socket, &readfds);
// 		activity = select(_socket + 1, &readfds, NULL, NULL, NULL);
// 		if (activity == -1)
// 			throw ThrowException("SELECT ERROR");
// 		if (FD_ISSET(_socket, &readfds))
// 		{
// 			struct	sockaddr_in	client_addr;
// 			memset(&client_addr, 0, sizeof(client_addr));
// 			socklen_t addr_len = sizeof(client_addr);
// 			int socket_client = accept(_socket, (struct sockaddr *)&client_addr, &addr_len);
// 			if (socket_client == -1)
// 				throw ThrowException("ACCEPT ERROR");
// 			ssize_t	bytes_received = recv(socket_client, buffer, sizeof(buffer), 0);
// 			if (bytes_received > 0)
// 			{
// 				std::cout << buffer << std::endl;
// 				send(socket_client, "coucou la socket\n", strlen("coucou la socket\n"), 0);
// 			}
// 			else if (bytes_received == 0)
// 				std::cout << "Connexion closed" << std::endl;
// 			else
// 				throw ThrowException("RECV ERROR");
// 			// close(socket_client);
// 		}
// 	}
// }

void	IRC_Server::check_socket_server()
{
	if (FD_ISSET(_socket, &_readfds))
	{
		int client_fd;
		client_fd = accept(this->_socket, NULL, NULL);
		std::cout<<"accept client fd: "<<client_fd<<std::endl;
		if (client_fd == -1)
			throw ThrowException("ACCEPT ERROR");//enqueter pas quitter??
		this->_clients.push_back(client_fd);
	}
}


void	IRC_Server::write_socket_client(int i)
{
	if (_clients[i].get_client_confirmation() == false)
	{
		send(this->_clients[i].get_socket_client(), ":server CAP * LS :\n:server 001 madaguen :Welcome to the IRC Network madaguen!madaguen@localhost\n:server 002 madaguen :Your host is server, running version 1.0\n:server 003 madaguen :This server was created 2024-10-23\n:server 004 madaguen server 1.0 i nt\n", strlen(":server CAP * LS :\n:server 001 madaguen :Welcome to the IRC Network madaguen!madaguen@localhost\n:server 002 madaguen :Your host is server, running version 1.0\n:server 003 madaguen :This server was created 2024-10-23\n:server 004 madaguen server 1.0 i nt\n"), 0);
		_clients[i].set_client_confirmation(true);
	}
	else
		send(this->_clients[i].get_socket_client(), "MODE madaguen +i\n", strlen("MODE madaguen +i\n"), 0);
	// send(this->_clients[i].get_socket_client(), "coucou la socket\n", strlen("coucou la socket\n"), 0);
}

void	IRC_Server::read_socket_client(int i)
{
	char	buffer[2048];

	memset(&buffer, 0, sizeof(buffer));
	ssize_t	bytes_received = recv(this->_clients[i].get_socket_client(), buffer, sizeof(buffer), 0);
	if (bytes_received > 0)
	{
		std::cout << buffer << std::endl;
	}
	else if (bytes_received == 0)
	{
		FD_CLR(this->_clients[i].get_socket_client(), &_readfds);
		this->_clients.erase(this->_clients.begin() + i);
		std::cout << "Connexion closed" << std::endl;
	}
	else
		throw ThrowException("RECV ERROR");//ne pas quitter?
}

void	IRC_Server::check_socket_client()
{
	for (int i = 0; i < static_cast<int>(this->_clients.size()); i++)
	{
		if (FD_ISSET(this->_clients[i].get_socket_client(), &_exceptfds))
		{
			throw std::runtime_error("error in socket client");//ne pas quitter jsute degager le client
		}
		if (FD_ISSET(this->_clients[i].get_socket_client(), &_writefds))
		{
			this->write_socket_client(i);
		}
		if (FD_ISSET(this->_clients[i].get_socket_client(), &_readfds))
		{
			this->read_socket_client(i);
		}
	}

}

void	IRC_Server::check_all_sockets()
{
	check_socket_server();
	check_socket_client();
}

void	IRC_Server::manage_fdset()
{
    FD_ZERO(&_readfds);
    FD_ZERO(&_writefds);
    FD_ZERO(&_exceptfds);
    FD_SET(_socket, &_readfds);
    FD_SET(_socket, &_writefds);
    FD_SET(_socket, &_exceptfds);
	for (int i = 0; i < static_cast<int>(this->_clients.size()); i++)
    {
    	FD_SET(this->_clients[i].get_socket_client(), &_readfds);
    	FD_SET(this->_clients[i].get_socket_client(), &_writefds);
    	FD_SET(this->_clients[i].get_socket_client(), &_exceptfds);
    }
}

int	IRC_Server::get_nfds()
{
	int	maxi_fds = _socket;
	for (int i = 0; i < static_cast<int>(this->_clients.size()); i++)
	{
		if (this->_clients[i].get_socket_client() > maxi_fds)
			maxi_fds = this->_clients[i].get_socket_client();
	}
	return (maxi_fds);
}

void	IRC_Server::manage()
{
	if (bind(_socket, (struct sockaddr *)&_server_addr, sizeof(_server_addr)) == -1)
		throw ThrowException("BIND ERROR");
	if (listen(_socket, 1010) == -1)
		throw ThrowException("LISTEN ERROR");
	while (true)
	{
		this->manage_fdset();
		if (select(this->get_nfds() + 1, &_readfds, NULL /*remettre writefds quand on gerera le buffer client */, &_exceptfds, NULL) == -1)
		{
			dprintf(2, "eroror select == %s\n", strerror(errno));
			throw ThrowException("SELECT ERROR");//enqueter quitter ou pas? continue?
		}
		this->check_all_sockets();
	}
}
