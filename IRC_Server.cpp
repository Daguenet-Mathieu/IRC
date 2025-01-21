#include "IRC_Server.hpp"
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

 std::string	IRC_Server::_create_time = getCurrentDateTime();

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

IRC_Server::IRC_Server(int port, const std::string& password) : _port(port), _password(password){
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket == -1)
        throw ThrowException("ERROR SOCKET");
    memset(&_server_addr, 0, sizeof(_server_addr));
    _server_addr.sin_family = AF_INET;
    _server_addr.sin_addr.s_addr = INADDR_ANY;
    _server_addr.sin_port = htons(_port);
    std::cout << "Creation time: " << _create_time << std::endl;
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

std::string IRC_Server::getCurrentDateTime() {
    // Obtenez l'heure actuelle
    time_t now = time(0);
    struct tm tstruct;
    char buffer[80];

    // Convertissez le temps en une structure tm
    tstruct = *localtime(&now);

    // Formatez le temps en "jj mois yyyy"
    strftime(buffer, sizeof(buffer), "%d %B %Y", &tstruct);
    return std::string(buffer);
}

std::string	IRC_Server::get_create_time()
{
	return (_create_time);
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
		char *client_ip;
		int	ip;
		struct sockaddr_in client_addr;
		socklen_t addr_len = sizeof(client_addr);
		client_fd = accept(this->_socket, (struct sockaddr *)&client_addr, &addr_len);
		std::cout<<"--------------------------new accept client fd: "<<client_fd<<std::endl;
		if (client_fd == -1)
			throw ThrowException("ACCEPT ERROR");//enqueter pas quitter??
		client_ip = inet_ntoa(client_addr.sin_addr);
		ip = ntohs(client_addr.sin_port);
		IRC_Client client(client_fd, ip);
		std::cout<<"Connexion acceptée de "<<client_ip<<" : "<<ntohs(client_addr.sin_port)<<std::endl;
		this->_clients.push_back(client);
	}
}

void	IRC_Server::read_socket_client(int i)
{
	char		buffer[2048];

	memset(&buffer, 0, sizeof(buffer));
	ssize_t	bytes_received = recv(this->_clients[i].get_socket_client(), buffer, sizeof(buffer), 0);
	if (bytes_received > 0)
	{
		std::cout << buffer << std::endl;
		_clients[i].fill_input_client(buffer, bytes_received);
		std::string input(buffer);
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
			std::string line;
			if (this->_clients[i].get_input_client(line))
			{
				std::cout<<"\t\t\t\ta ete lu dansl 'input client: "<<line<<std::endl;
				this->launch_method(this->parse_data(line, this->_clients[i]), this->_clients[i]);
			}
			bool res = this->_clients[i].send_output_client();
			std::cout<<"RES == "<<res<<" current STATE == "<<this->_clients[i].get_state()<<std::endl;
			if (this->_clients[i].get_state() == ERROR && res == false){
			// { int k = 0;
			// 	while (this->_clients[i].send_output_client())
			// 		std::cout<<"k ==== "<<k<<std::endl;;
				close(this->_clients[i].get_socket_client());
				// shutdown(this->_clients[i].get_socket_client(), SHUT_WR);
				this->_clients.erase(this->_clients.begin() + i);
			}
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
		// std::cout<<"avat select"<<std::endl;
		if (select(this->get_nfds() + 1, &_readfds, &_writefds, &_exceptfds, NULL) == -1)
		{
			dprintf(2, "eroror select == %s\n", strerror(errno));
			throw ThrowException("SELECT ERROR");//enqueter quitter ou pas? continue?
		}
		// std::cout<<"apres select"<<std::endl;
		this->check_all_sockets();
	}
}

std::string get_word(const std::string &line, int word_index)
{
	size_t start_word = 0;
	size_t end_word = 0;

	for (int i = 0; i < word_index; i++)
	{
		// std::cout<<"i:"<<i<<std::endl;
		start_word = line.find_first_not_of(' ', end_word);
		if (start_word == std::string::npos)
			return (std::string());
		end_word = line.find_first_of(' ', start_word);
		if (end_word == std::string::npos)
			end_word = line.size();
		// std::cout<<"start:"<<start_word<<" end:"<<end_word<<" size:"<<end_word - start_word<<std::endl;
	}
	// std::cout<<"start:"<<start_word<<" end:"<<end_word<<" size:"<<end_word - start_word<<std::endl;
	int size = end_word - start_word;
	// std::cout<<"size:"<<size<<std::endl;
	if (size <= 0)
		return std::string();
	// std::cout<<"substr:"<<line.substr(start_word, end_word - start_word)<<std::endl;
	// std::cout<<"line:"<<line<<std::endl;
	return (line.substr(start_word, size));
}

struct input	IRC_Server::parse_data(const std::string &line, IRC_Client &client)
{
	struct input res;
	std::cout<<"\t\ton parse :"<<line<<std::endl;
	(void)client;
	(void)line;
	const char	*method[] = {"CAP","JOIN" ,"NICK", "KICK", "INVITE", "TOPIC", "MODE",  "PRIVMSG", "DCC","PING", "PASS", "USER", "WHOIS",NULL};
	// // for (int i =0; i < (int)line.size(); i++)
	// // {
	// // 	std::cout<<"line:"<<line[i]<<" et:"<<(int)line[i]<<std::endl;
	// // }
	// if (input[input.size() - 1] == '\n')
	// {
	// 	std::cout<<"coucou"<<std::endl;
	// 	std::cout<<"innput size: "<<input.size()<<std::endl;
	// 	input.erase(input.size() - 1);
	// }
	std::string word;
	res.method = 0;
	int i;
	for (i = 0; method[i]; i++)
	{
		word = get_word(line, 1);
		// std::cout<<"word:|"<<word<<"|"<<std::endl;
		// std::cout<<"mehtod:|"<<method[i]<<"|"<<std::endl;
		if (word == method[i])
			break ;
	}
	// std::cout<<"method found :"<<i<<std::endl;
	res.method = i;
	// if (!method[i])
	// std::cout<<"word ============= "<<word<<std::endl;
	// else if (line == "CAP END")
	// {
	// 	std::cout<<"nego finie"<<std::endl;
	// 	res.method = CAPEND;
	// 	return (res);		
	// }
	// else
	// 	std::cout<<"line recu == |"<<line<<"|"<<std::endl;
	if (word == "CAP")
		res.content = get_word(line, 2);
	// std::cout<<"content:"<<res.content<<std::endl;
	//poteger si vide?
	if (word == "PONG")
	{
		std::cout<<"cocuocu pong"<<std::endl;
		res.method = PONG;
		res.content = get_word(line, 2);
	}
	if (word == "PASS")
	{
		res.password = get_word(line, 2);
	}
	return (res);
}

void	IRC_Server::cap(const struct input &struct_input, IRC_Client &client)
{
	if (client.get_state() == ERROR)
		return ;
	// set negociating si ls
	// terminer negociation si end envoyer la liste des infos manquqntes
	//quitter si nego == over
	(void)struct_input;
	std::cout<<"intput:"<<struct_input.content<<std::endl;
	// std::string response = ":server CAP * LS :\n\n: Welcome to the IRC Network \n\n";
	// response += client.get_username();
	// response += '!';
	// response += client.get_nickname();
	// response += "@madaguen_auferran" + client.get_url() + "\n KICK\n INVITE\n TOPIC\n MODE [-i -t -k -o -l]\n";
	// std::string response = ":server CAP * LS :KICK INVITE TOPIC MODE\r\n";
	// response += ":server 001 " + client.get_nickname() + " :Welcome to the IRC Network\r\n";
	// response += ":" + client.get_nickname() + "!" + client.get_username() + "@" + client.get_url() + " ";
	if (client.get_state() == END_NEGO)
		return ;
	if (struct_input.content == "END")
	{
		client.set_state(END_NEGO);
		return ;
	}
	if (struct_input.content == "LS")
	{
		client.set_state(NEGOCIATING);

		std::string response;

		// 1. CAP LS
		response = ":server CAP * LS :KICK INVITE TOPIC MODE\r\n";

		// 2. RPL_WELCOME (001)
		response += std::string(":server 001 ") + client.get_nickname() + "madaguen" + " :Welcome to the MADAGUENAUFERRANIRC SERVER\r\n";

		// 3. RPL_YOURHOST (002)
		response += ":server 002 " + client.get_nickname() +  " :Your host is server MADAGUENAUFERRANIRC, running version 1.0\r\n";

		// 4. RPL_CREATED (003)
		response += ":server 003 " + client.get_nickname() + " :This server was created " + get_create_time() + "\r\n";

		// 5. RPL_MYINFO (004) - Format corrigé : ne pas mettre le ":" avant le message
		response += ":server 004 " + client.get_nickname() + " localhost 1.0 o itklo\r\n";

		// 6. Mode utilisateur
		response += ":server MODE " + client.get_nickname() + " +i\r\n";

		// Pour la commande WHOIS, vous devez répondre avec :
		// RPL_WHOISUSER (311)
		// response += ":server 311 " + nickname + " " + target + " " + username + " " + hostname + " * :" + realname + "\r\n";
		// RPL_ENDOFWHOIS (318)
		// response += ":server 318 " + target + " :End of /WHOIS list\r\n";
		std::cout<<"je print ici la reponse:"<<response<<std::endl;
		client.set_output_client(response);
	}
	
	// send(client.get_socket_client(), response.c_str(), response.size(), 0);
}


void	IRC_Server::dcc(const struct input &, IRC_Client &)
{
	std::cout<<"in dcc"<<std::endl;
	//file transfert?
}

void	IRC_Server::join(const struct input &, IRC_Client &)
{
	std::cout<<"in join"<<std::endl;
}

void	IRC_Server::nick(const struct input &, IRC_Client &)
{
	std::cout<<"in nick"<<std::endl;
	// 433 ERR_NICKNAMEINUSE
	//chekc si le nick est deja utilise  sinon le changer
	//:ancien_nick!user@host NICK :nouveau_nick
	// :irc.example.com 433 * nouveau_nick :Nickname is already in use
	// :irc.example.com 432 * mauvais_nick :Erroneous nickname
//si choix invalide ? char interdit?

}

void	IRC_Server::kick(const struct input &, IRC_Client &)
{
	std::cout<<"in kick"<<std::endl;
}

void	IRC_Server::invite(const struct input &, IRC_Client &)
{
	std::cout<<"in invit"<<std::endl;
}

void	IRC_Server::topic(const struct input &, IRC_Client &)
{
	std::cout<<"in topic"<<std::endl;
}

void	IRC_Server::mode(const struct input &, IRC_Client &)
{
	std::cout<<"in mode"<<std::endl;
}

void	IRC_Server::privmsg(const struct input &input, IRC_Client &)
{
	std::cout<<"in privmsg"<<std::endl;
	(void) input;
	// if (input.destinataire[0] == '#')
	//channel
	// 	;
	// else
	//message user to user
	// 	;
}

void	IRC_Server::pass(const struct input &input, IRC_Client &client)
{
	std::cout<<"in pass"<<std::endl;
	std::cout<<"le passwod du serveur est  : " << _password <<std::endl;
	std::cout<<"le passwod recu est : " << input.password <<std::endl;
	if (_password != input.password)
	{
		std::string response = ":464 madaguen :Password incorrect\r\n";//mettre username
		std::cout<<"\t\t\t wrong password"<<std::endl;
		client.set_output_client(response);
		client.set_state(ERROR);
	}
}

void	IRC_Server::user(const struct input &, IRC_Client &)
{
	std::cout<<"in user"<<std::endl;
}

void	IRC_Server::whois(const struct input &, IRC_Client &)
{
	std::cout<<"in whois"<<std::endl;
}

void	IRC_Server::leave(const struct input &, IRC_Client &)
{
	std::cout<<"in leave"<<std::endl;
}


void ping(const struct input &, IRC_Client &)
{
	std::cout<<"in ping"<<std::endl;
	std::string response;
	response += "PONG" + input.content;
	std::cout<<"pong response: "<<response<<std::endl;
	client.set_output_client(response);
}

void part(const struct input &, IRC_Client &)
{
	std::cout<<"in part"<<std::endl;
}

void quit(const struct input &, IRC_Client &)
{
	std::cout<<"in quit"<<std::endl;

} 

void names(const struct input &, IRC_Client &)
{
	std::cout<<"in names"<<std::endl;

}

void list(const struct input &, IRC_Client &)
{
	std::cout<<"in list"<<std::endl;

} 


//en cas d'erreur mdp invalid par ex kill le client

void	IRC_Server::launch_method(const struct input &struct_input,  IRC_Client &client)
{

	// if (client.get_state() == NOT_CONNECTED && (struct_input.method != PASS && struct_input.method != CAP))
	// {
	// 	//ERR_PASSWDMISMATCH (464)
	// 	//reclamer le password
	// 	;
	// }
	std::cout<<"!= CAP:"<<(struct_input.method != CAP)<<"!= UESER:"<<(struct_input.method != USER)<<"!= NICK:"<<(struct_input.method != NICK)<<std::endl;
	std::cout<<"method:"<<struct_input.method<<std::endl;
	// if (client.get_state() != CONNECTED && (struct_input.method != CAP && struct_input.method != USER && struct_input.method != NICK))
	// {
	// 	//send missing info message 
	// 	// :MonSrv 431 ERR_NICKREQ Vous devez fournir un nom de surnom (NICK) avant de vous connecter.
	// 	//nick?
	// 	// :MonSrv ERR_USERREQ Vous devez fournir des informations utilisateur (USER) avant de vous connecter.
	// 	// :MonSrv 461 ERR_NEEDMOREPARAMS  Vous devez fournir un username (USER) avant de vous connecter.
	// 	//user?
	// 	std::cout<<"on rentre la dedans meme si on veut pas"<<std::endl;
	// 	return ;
	// }
	//si cap et nego over ignorer

	//si different de cap nick ou user et pas tout set envoyer erreur doner les infos manquqntes dans le message
void (IRC_Server::*fun[])(const struct input&, IRC_Client&) = {
   &IRC_Server::cap,       // Négociation des capacités
   &IRC_Server::nick,      // Gestion du nickname
   &IRC_Server::pass,      // Vérification du mot de passe
   &IRC_Server::user,      // Configuration username/realname
   &IRC_Server::join,      // Rejoindre un channel
   &IRC_Server::kick,      // Éjecter un user (op)
   &IRC_Server::invite,    // Inviter un user (op)
   &IRC_Server::topic,     // Gérer le sujet (op)
   &IRC_Server::mode,      // Gérer les modes (op)
   &IRC_Server::privmsg,   // Messages privés/channel
   &IRC_Server::dcc,       // Transfert de fichiers
   &IRC_Server::ping,      // Gestion du ping
   &IRC_Server::whois,     // Info sur un user
   &IRC_Server::leave,     // Quitter un channel
   &IRC_Server::part,      // Alternative à leave
   &IRC_Server::quit,      // Déconnexion
   &IRC_Server::names,     // Liste des users
   &IRC_Server::list,      // Liste des channels
   NULL                    // Fin du tableau
};
	//  void (IRC_Server::*fun[])(const struct input&, IRC_Client&) = {&IRC_Server::cap,&IRC_Server::join ,&IRC_Server::nick, &IRC_Server::kick, &IRC_Server::invite, &IRC_Server::topic, &IRC_Server::mode, &IRC_Server::privmsg, \
	// &IRC_Server::dcc, &IRC_Server::pong,  &IRC_Server::pass, &IRC_Server::user, &IRC_Server::whois, &IRC_Server::leave, NULL};
	// MethodFunction fun = initializeFunctions();
	if (struct_input.method < END_METHOD)
		(this->*fun[struct_input.method])(struct_input, client);
}
