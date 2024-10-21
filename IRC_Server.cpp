#include "IRC_Server.hpp"
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

void	IRC_Server::manage()
{
	char 		buffer[2048];
	fd_set		readfds;

	if (bind(_socket, (struct sockaddr *)&_server_addr, sizeof(_server_addr)) == -1)
		throw ThrowException("BIND ERROR");
	if (listen(_socket, 3) == -1)
		throw ThrowException("LISTEN ERROR");
	FD_ZERO(&readfds);
	FD_SET(_socket, &readfds);
	while (true)
	{
		memset(&buffer, 0, sizeof(buffer));
        FD_ZERO(&readfds);
        FD_SET(_socket, &readfds);
        for (int i = 0; i < static_cast<int>(this->_clients.size()); i++)
        {
        	FD_SET(this->_clients[i].get_socket_client(), &readfds);
        }

		std::cout<<"onrepassera par la"<<std::endl;


		// struct timeval timeout;
    	// timeout.tv_sec = 5;   // 5 secondes
    	// timeout.tv_usec = 0;  // 0 microsecondes
		std::cout<<"_socket == "<<_socket<<std::endl;
		if (select(_socket + 1, &readfds, NULL, NULL, NULL) == -1)
		{
			throw ThrowException("SELECT ERROR");
		}
		if (FD_ISSET(_socket, &readfds))
		{
			std::cout<<"cocou dan fdiset"<<std::endl;
			IRC_Client	client(this->_port);

			//socklen_t addr_len = sizeof(client.get_client_addr());
			//struct sockaddr_in addr = client.get_client_addr();
			client.set_socket_client(accept(this->_socket, 0, 0));
			if (client.get_socket_client() == -1)
				throw ThrowException("ACCEPT ERROR");
			std::cout<<"JE MET CE FD DANS READFDS"<<client.get_socket_client()<<std::endl;
			FD_SET(client.get_socket_client(), &readfds);
			ssize_t	bytes_received = recv(client.get_socket_client(), buffer, sizeof(buffer), 0);
			this->_clients.push_back(client);
			client.set_socket_client(-1);
			if (bytes_received > 0)
 			{
 				std::cout << buffer << std::endl;
 				send(client.get_socket_client(), "coucou la socket de connection\n", strlen("coucou la socket\n"), 0);
 			}
 			else if (bytes_received == 0)
 				std::cout << "Connexion closed" << std::endl;
 			else
 				throw ThrowException("RECV ERROR");
		}
		else
		{
			std::cout<<"on estp asse par ici"<<std::endl;
			for (int i = 0; i < static_cast<int>(this->_clients.size()); i++)
			{
				if (FD_ISSET(this->_clients[i].get_socket_client(), &readfds))
				{
					ssize_t	bytes_received = recv(this->_clients[i].get_socket_client(), buffer, sizeof(buffer), 0);
					if (bytes_received > 0)
 					{
 						std::cout << buffer << std::endl;
 						send(this->_clients[i].get_socket_client(), "coucou la socket\n", strlen("coucou la socket\n"), 0);
						break ;
 					}
					else if (bytes_received == 0)
					{
						FD_CLR(this->_clients[i].get_socket_client(), &readfds);
 						std::cout << "Connexion closed" << std::endl;
					}
 					else
 						throw ThrowException("RECV ERROR");
				}

			}
		}
	}
}
