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

struct input{
	int			method;
	std::string	transmitter;
	std::string	recipient;
	std::string	content;
	std::string password;
	// bool		allowed;
	// bool		format;
};

struct response{
	int	status_code;
	std::string	transmitter;
	std::string	recipient;
	std::string method;
	std::string body;
};

enum cmds {
   CAP,        // Négociation des capacités du client/serveur
   NICK,       // Définir/changer le nickname
   PASS,       // Authentification avec le mot de passe serveur
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
		void cap(const struct input &, IRC_Client &);      // Négociation des capacités
		void dcc(const struct input &, IRC_Client &);      // Transfert de fichiers (bonus)
		void join(const struct input &, IRC_Client &);     // Rejoindre un channel
		void nick(const struct input &, IRC_Client &);     // Définir/changer le nickname 
		void kick(const struct input &, IRC_Client &);     // Éjecter un utilisateur (op)
		void invite(const struct input &, IRC_Client &);   // Inviter un utilisateur (op)
		void topic(const struct input &, IRC_Client &);    // Définir/voir le sujet (op)
		void mode(const struct input &, IRC_Client &);     // Modifier les modes (op)
		void privmsg(const struct input &, IRC_Client &);  // Messages privés/channel
		void ping(const struct input &, IRC_Client &);     // Réception d'un ping client 
		void pass(const struct input &, IRC_Client &);     // Vérif mot de passe serveur
		void user(const struct input &, IRC_Client &);     // Définir username/realname
		void whois(const struct input &, IRC_Client &);    // Info sur un utilisateur
		void leave(const struct input &, IRC_Client &);    // Quitter un channel
		void part(const struct input &, IRC_Client &);     // Alternative à leave
		void quit(const struct input &, IRC_Client &);     // Déconnexion du serveur 
		void names(const struct input &, IRC_Client &);    // Liste des users du channel
		void list(const struct input &, IRC_Client &);     // Liste des channels

};

#endif
