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
class IRC_Channel;

struct input {
	int			method;
};

enum cmds {
   CAP,        // Négociation des capacités du client/serveur
   PASS,       // Authentification avec le mot de passe serveur
   NICK,       // Définir/changer le nickname
   USER,       // Définir le username et realname 
   JOIN,       // Rejoindre un channel
   KICK,       // Éjecter un utilisateur d'un channel (opérateur)
   INVITE,     // Inviter un utilisateur dans un channel (opérateur)
   TOPIC,      // Définir/voir le sujet d'un channel (opérateur)
   MODE,       // Modifier les modes du channel (i,t,k,o,l) (opérateur)
   PRIVMSG,    // Envoyer un message privé à un utilisateur ou channel
   DCC,        // Transfert direct de fichiers entre clients (bonus)
   PING,       // Serveur ping le client qui doit répondre PONG
   WHOIS,      // Obtenir des informations sur un utilisateur
   LEAVE,      // Quitter un channel
   PART,       // Quitter un channel (alternative standard à LEAVE)
   QUIT,       // Déconnexion du serveur
   NAMES,      // Lister les utilisateurs d'un channel
   LIST,       // Lister les channels disponibles
   END_METHOD  // Marqueur de fin pour validation des commandes
};

class	IRC_Server
{
	public :

		IRC_Server(int port, std::string const& password);
		~IRC_Server();

		std::string	get_create_time();
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
		std::string					_create_time;
		std::string					_password;
		int							_port;
		int							_socket;
		int							ip;
		struct sockaddr_in			_server_addr;
		std::vector<IRC_Client>		_clients;
		std::vector<IRC_Channel>	_channels;

		void						write_socket_client(int index_client);
		void						read_socket_client(int index_client);
		void						check_socket_client();
		void						check_socket_server();
		void						check_all_sockets();
		void						manage_fdset();
		int							get_nfds();
		std::string					getCurrentDateTime();
		struct input				parse_data(const std::string &, IRC_Client &);
		void						launch_method(const struct input &, const std::string &, IRC_Client &, int);
		bool 						cap(const struct input &, IRC_Client &, const std::string &);
		bool 						dcc(const struct input &, IRC_Client &, const std::string &);
		bool						join(const struct input &, IRC_Client &, const std::string &);
		bool						nick(const struct input &, IRC_Client &, const std::string &);
		bool						kick(const struct input &, IRC_Client &, const std::string &);
		bool						invite(const struct input &, IRC_Client &, const std::string &);
		bool						topic(const struct input &, IRC_Client &, const std::string &);
		bool						mode(const struct input &, IRC_Client &, const std::string &);
		bool						privmsg(const struct input &, IRC_Client &, const std::string &);
		bool						ping(const struct input &, IRC_Client &, const std::string &);
		bool						pass(const struct input &, IRC_Client &, const std::string &);
		bool						user(const struct input &, IRC_Client &, const std::string &);
		bool						whois(const struct input &, IRC_Client &, const std::string &);
		bool						leave(const struct input &, IRC_Client &, const std::string &);
		bool						part(const struct input &, IRC_Client &, const std::string &);
		bool						quit(const struct input &, IRC_Client &, const std::string &); 
		bool						names(const struct input &, IRC_Client &, const std::string &);
		bool						list(const struct input &, IRC_Client &, const std::string &);

};

#endif
