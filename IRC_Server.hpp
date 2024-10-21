#ifndef	IRC_SERVER_HPP
#define	IRC_SERVER_HPP

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <climits>
#include <cerrno>
#include <map>
#include <vector>
#include <exception>
#include <stdexcept>
#include <signal.h>
#include "IRC_Client.hpp"
#include "IRC_Channel.hpp"

class IRC_Client;

class	IRC_Server
{
	public :

	IRC_Server();										//CANONICAL
	IRC_Server(int port, std::string const& password);
	IRC_Server(IRC_Server const& src);					//CANONICAL
	~IRC_Server();										//CANONICAL

	IRC_Server&	operator=(IRC_Server const& rhs);		//CANONICAL
	void	manage();

	class	ThrowException : public std::exception
	{
		public :

		explicit ThrowException(const std::string& message) : _error_message(message){}
		virtual ~ThrowException() throw() {}

		const char*	what() const throw()
		{
			return _error_message.c_str();
		}

		private :

		std::string	_error_message;
	};

	private :

	int							_port;
	std::string					_password;
	int							_socket;
	struct sockaddr_in			_server_addr;
	std::vector<IRC_Client>		_clients;
	// std::vector<IRC_Channel>	_channels;
};

#endif
