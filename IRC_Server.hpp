#ifndef	IRC_SERVER_HPP
# define	IRC_SERVER_HPP

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
#include <arpa/inet.h>
#include "IRC_Client.hpp"
#include "IRC_Channel.hpp"
#include <ctime>
#include <string>

class IRC_Client;

typedef void (*MethodFunction)(const struct input &, IRC_Client &);

struct input{
	int			method;
	std::string	transmitter;
	std::string	recipient;
	std::string	content;
	std::string password;
	// bool		allowed;
	bool		format;
};

enum cmds{
	CAP,
	JOIN,
	NICK,
	KICK,
	INVITE,
	TOPIC,
	MODE,
	PRIVMSG,
	DCC,
	PONG,
	PASS,
	USER,
	WHOIS,
	LEAVE,
	END_METHOD
};

class	IRC_Server
{
	public :

		IRC_Server();										//CANONICAL
		IRC_Server(int port, std::string const& password);
		IRC_Server(IRC_Server const& src);					//CANONICAL
		~IRC_Server();										//CANONICAL
		static std::string	get_create_time();
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
		fd_set						_readfds;
		fd_set						_writefds;
		fd_set						_exceptfds;
		static	std::string			_create_time;
		int							_port;
		std::string					_password;
		int							_socket;
		struct sockaddr_in			_server_addr;
		std::vector<IRC_Client>		_clients;
		//std::vector<IRC_Channel>	_channels;
		void						write_socket_client(int index_client);
		void						read_socket_client(int index_client);
		void						check_socket_client();
		void						check_socket_server();
		void						check_all_sockets();
		void						manage_fdset();
		int							get_nfds();
		static std::string					getCurrentDateTime();
		struct input				parse_data(const std::string &, IRC_Client &);
		void						launch_method(const struct input &, IRC_Client &);
		static void					cap(const struct input &, IRC_Client &);
		static void					dcc(const struct input &, IRC_Client &);
		static void					join(const struct input &, IRC_Client &);
		static void					nick(const struct input &, IRC_Client &);
		static void					kick(const struct input &, IRC_Client &);
		static void					invite(const struct input &, IRC_Client &);
		static void					topic(const struct input &, IRC_Client &);
		static void					mode(const struct input &, IRC_Client &);
		static void					privmsg(const struct input &, IRC_Client &);
		static void					pong(const struct input &, IRC_Client &);
		static void					pass(const struct input &, IRC_Client &);
		static void					user(const struct input &, IRC_Client &);
		static void					whois(const struct input &, IRC_Client &);
		static void					leave(const struct input &, IRC_Client &);
};

#endif