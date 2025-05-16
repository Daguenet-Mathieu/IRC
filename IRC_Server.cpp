#include "IRC_Server.hpp"
#include <unistd.h>
#include <stdio.h>
#include <errno.h>


//CONSTRUCTOR

IRC_Server::IRC_Server(int port, const std::string& password) :_password(password),  _port(port){
	_create_time = getCurrentDateTime();
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket == -1)
        throw ThrowException("ERROR SOCKET");
    memset(&_server_addr, 0, sizeof(_server_addr));
    _server_addr.sin_family = AF_INET;
    _server_addr.sin_addr.s_addr = INADDR_ANY;
    _server_addr.sin_port = htons(_port);
    std::cout << "Creation time: " << _create_time << std::endl;
}

//DESTRUCTOR

IRC_Server::~IRC_Server()
{
	if (_socket != -1)
		close(_socket);
}

//FUNCTIONS

std::string IRC_Server::getCurrentDateTime() {
    time_t now = time(0);
    struct tm tstruct;
    char buffer[80];

    tstruct = *localtime(&now);

    strftime(buffer, sizeof(buffer), "%d %B %Y", &tstruct);
    return std::string(buffer);
}

std::string	IRC_Server::get_create_time()
{
	return (_create_time);
}

void	IRC_Server::check_socket_server()
{
	if (FD_ISSET(_socket, &_readfds))
	{
		int client_fd;
		char *client_ip;

		struct sockaddr_in client_addr;
		socklen_t addr_len = sizeof(client_addr);
		client_fd = accept(this->_socket, (struct sockaddr *)&client_addr, &addr_len);
		if (client_fd == -1)
			return ;
		client_ip = inet_ntoa(client_addr.sin_addr);
		ip = ntohs(client_addr.sin_port);
		IRC_Client client(client_fd);
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
		close(this->_clients[i].get_socket_client());
		this->_clients.erase(this->_clients.begin() + i);
	}
	else
		throw ThrowException("RECV ERROR");
}

void	IRC_Server::check_socket_client()
{
	for (int i = 0; i < static_cast<int>(this->_clients.size()); i++)
	{
		if (FD_ISSET(this->_clients[i].get_socket_client(), &_exceptfds))
			throw std::runtime_error("error in socket client");
		if (FD_ISSET(this->_clients[i].get_socket_client(), &_writefds))
		{
			std::string line;
			if (this->_clients[i].get_input_client(line))
				this->launch_method(this->parse_data(line, this->_clients[i]), line, this->_clients[i], i);
			bool res = this->_clients[i].send_output_client();
			if (this->_clients[i].get_state() == ERROR && res == false)
			{
				close(this->_clients[i].get_socket_client());
				this->_clients.erase(this->_clients.begin() + i);
			}
		}
		if (FD_ISSET(this->_clients[i].get_socket_client(), &_readfds))
			this->read_socket_client(i);
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
		if (select(this->get_nfds() + 1, &_readfds, &_writefds, &_exceptfds, NULL) == -1)
		{
			dprintf(2, "error select == %s\n", strerror(errno));
			throw ThrowException("SELECT ERROR");
		}
		this->check_all_sockets();
	}
}

std::string get_word(const std::string &line, int word_index)
{
	size_t start_word = 0;
	size_t end_word = 0;

	start_word = line.find_first_not_of(' ', 0);
	for (int i = 0; i < word_index; i++)
	{
		start_word = line.find_first_not_of(' ', end_word);
		if (start_word == std::string::npos)
			return (std::string());
		end_word = line.find_first_of(' ', start_word);
		if (end_word == std::string::npos)
			end_word = line.size();
	}
	int size = end_word - start_word;
	if (size <= 0)
		return std::string();
	return (line.substr(start_word, size));
}

struct input	IRC_Server::parse_data(const std::string &line, IRC_Client &)
{
	struct input request_info;
	std::string word;

	const char *method[] = {
		"CAP",
		"NICK",
		"PASS",
		"USER",
		"JOIN",
		"KICK",
		"INVITE",
		"TOPIC",
		"MODE",
		"PRIVMSG",
		"DCC",
		"PING",
		"WHOIS",
		"LEAVE",
		"PART",
		"QUIT",
		"NAMES",
		"LIST",
		NULL
	};
	request_info.method = 0;
	word = get_word(line, 1);
	int i = 0;
	for (; method[i] != NULL && word != method[i]; i++);
	std::cout<<"found method : "<<i<<" word == |"<<word<<"|"<<std::endl;
	request_info.method = i;
	return (request_info);
}

//COMMANDS

bool	IRC_Server::cap(const struct input &, IRC_Client &client, const std::string &line)
{
	std::cout<<"iun cap"<<std::endl;//check que nego est bien fini toutes les infos set et user connecte
	if (get_word(line, 2) == "END")
		return true;
	std::string response = ":server CAP * LS :KICK INVITE TOPIC MODE\r\n";
	client.set_output_client(response);
	client.set_state(INSTANCE_CONNECT);//pour testester a degager
	//cap end check si status client connected sinon le degager
	// set negociating si ls
	// terminer negociation si end envoyer la liste des infos manquqntes
	// quitter si nego == over
	// std::string response = ":server CAP * LS :\n\n: Welcome to the IRC Network \n\n";
	// response += client.get_username();
	// response += '!';
	// response += client.get_nickname();
	// response += "@madaguen_auferran" + client.get_url() + "\n KICK\n INVITE\n TOPIC\n MODE [-i -t -k -o -l]\n";
	// std::string response = ":server CAP * LS :KICK INVITE TOPIC MODE\r\n";
	// response += ":server 001 " + client.get_nickname() + " :Welcome to the IRC Network\r\n";
	// response += ":" + client.get_nickname() + "!" + client.get_username() + "@" + client.get_url() + " ";
	// if (client.get_state() == END_NEGO)
	// 	return ;
	// if (struct_input.content == "END")
	// {
	// 	client.set_state(END_NEGO);
	// 	return ;
	// }
	// if (struct_input.content == "LS")
	// {
	// 	client.set_state(NEGOCIATING);

	// 	std::string response;

	// 	// 1. CAP LS
	// 	response = ":server CAP * LS :KICK INVITE TOPIC MODE\r\n";

	// 	// 2. RPL_WELCOME (001)
	// 	response += std::string(":server 001 ") + client.get_nickname() + "madaguen" + " :Welcome to the MADAGUENAUFERRANIRC SERVER\r\n";

	// 	// 3. RPL_YOURHOST (002)
	// 	response += ":server 002 " + client.get_nickname() +  " :Your host is server MADAGUENAUFERRANIRC, running version 1.0\r\n";

	// 	// 4. RPL_CREATED (003)
	// 	response += ":server 003 " + client.get_nickname() + " :This server was created " + get_create_time() + "\r\n";

	// 	// 5. RPL_MYINFO (004) - Format corrigé : ne pas mettre le ":" avant le message
	// 	response += ":server 004 " + client.get_nickname() + " localhost 1.0 o itklo\r\n";

	// 	// 6. Mode utilisateur
	// 	response += ":server MODE " + client.get_nickname() + " +i\r\n";
	return true;
	// 	// Pour la commande WHOIS, vous devez répondre avec :
	// 	// RPL_WHOISUSER (311)
	// 	// response += ":server 311 " + nickname + " " + target + " " + username + " " + hostname + " * :" + realname + "\r\n";
	// 	// RPL_ENDOFWHOIS (318)
	// 	// response += ":server 318 " + target + " :End of /WHOIS list\r\n";
	// 	std::cout<<"je print ici la reponse:"<<response<<std::endl;
	// 	client.set_output_client(response);
	// }
	
	// send(client.get_socket_client(), response.c_str(), response.size(), 0);
	return true;
}

bool	IRC_Server::pass(const struct input &, IRC_Client &, const std::string &)
{
	// if (_password != input.password)
	// {
	// 	std::string response = ":464 madaguen :Password incorrect\r\n";//mettre username
	// 	std::cout<<"\t\t\t wrong password"<<std::endl;
	// 	client.set_output_client(response);
	// 	client.set_state(ERROR);
	// }
	return true;
}

bool	IRC_Server::nick(const struct input &, IRC_Client &c, const std::string &line)
{
	std::cout<<"in nick"<<std::endl;
	std::cout<<"line : "<<line<<std::endl;
	c.set_nickname("");
	// 433 ERR_NICKNAMEINUSE
	//chekc si le nick est deja utilise  sinon le changer
	//:ancien_nick!user@host NICK :nouveau_nick
	// :irc.example.com 433 * nouveau_nick :Nickname is already in use
	// :irc.example.com 432 * mauvais_nick :Erroneous nickname
//si choix invalide ? char interdit?
	return true;
}

bool	IRC_Server::user(const struct input &, IRC_Client &, const std::string &)
{
	std::cout<<"in user"<<std::endl;
	return true;
}

bool	IRC_Server::join(const struct input &, IRC_Client &, const std::string &line)
{
	std::cout<<"in join"<<std::endl;
	std::cout << "line : " << line << std::endl;
	return true;
}

bool	IRC_Server::kick(const struct input &, IRC_Client &, const std::string &)
{
	std::cout<<"in kick"<<std::endl;
	return true;
}

bool	IRC_Server::invite(const struct input &, IRC_Client &, const std::string &)
{
	std::cout<<"in invit"<<std::endl;
	return true;
}

bool	IRC_Server::topic(const struct input &, IRC_Client &, const std::string &)
{
	std::cout<<"in topic"<<std::endl;
	return true;
}

bool	IRC_Server::mode(const struct input &, IRC_Client &, const std::string &)
{
	std::cout<<"in mode"<<std::endl;
	return true;
}

bool	IRC_Server::privmsg(const struct input &input, IRC_Client &, const std::string &)
{
	std::cout<<"in privmsg"<<std::endl;
	(void) input;
	return true;
}

bool	IRC_Server::dcc(const struct input &, IRC_Client &, const std::string &)
{
	std::cout<<"in dcc"<<std::endl;
	//file transfert?
	return true;
}

bool	IRC_Server::ping(const struct input &, IRC_Client &client, const std::string &line)
{
	std::string response = "PONG " + get_word(line, 2) + "\r\n";
	client.set_output_client(response);
	return true;
}

bool	IRC_Server::whois(const struct input &, IRC_Client &, const std::string &)
{
	std::cout<<"in whois"<<std::endl;
	return true;
}

bool	IRC_Server::leave(const struct input &, IRC_Client &, const std::string &)
{
	std::cout<<"in leave"<<std::endl;
	return true;
}

bool	IRC_Server::part(const struct input &, IRC_Client &, const std::string &)
{
	std::cout<<"in part"<<std::endl;
	return true;
}

bool	IRC_Server::quit(const struct input &, IRC_Client &, const std::string &)
{
	std::cout<<"in quit"<<std::endl;
	return true;
} 

bool	IRC_Server::names(const struct input &, IRC_Client &, const std::string &)
{
	std::cout<<"in names"<<std::endl;
	return true;
}

bool	IRC_Server::list(const struct input &, IRC_Client &, const std::string &)
{
	std::cout<<"in list"<<std::endl;
	return true;

} 

void	IRC_Server::launch_method(const struct input &user_input, const std::string &line, IRC_Client &client, int i)
{

	// if (client.get_state() == NOT_CONNECTED && (struct_input.method != PASS && struct_input.method != CAP))
	// {
	// 	//ERR_PASSWDMISMATCH (464)
	// 	//reclamer le password
	// 	;
	// }
	if (user_input.method > PASS && client.get_state() == NOT_CONNECTED)
	{
		client.set_output_client(":MADAGUENAUFERRAN 451 * PRIVMSG :You have not registered");
		close(client.get_socket_client());
		this->_clients.erase(this->_clients.begin() + i);
		//envoyer le client se faire foutre 3.1.1 rfc
		return ;
	}
	if (client.get_state() == INSTANCE_CONNECT){
		std::string response = std::string(":server 001 ") + client.get_nickname() + "madaguen" + " :Welcome to the MADAGUENAUFERRANIRC SERVER\r\n";
		// 3. RPL_YOURHOST (002)
		response += ":server 002 " + client.get_nickname() +  " :Your host is server MADAGUENAUFERRANIRC, running version 1.0\r\n";
		// 4. RPL_CREATED (003)
		response += ":server 003 " + client.get_nickname() + " :This server was created " + get_create_time() + "\r\n";
		// 5. RPL_MYINFO (004) - Format corrigé : ne pas mettre le ":" avant le message
		response += ":server 004 " + client.get_nickname() + " localhost 1.0 o itklo\r\n";
		// 6. Mode utilisateur
		response += ":server MODE " + client.get_nickname() + " +i\r\n";
		client.set_output_client(response);
		client.set_state(CONNECTED);
	}
	// std::cout<<"!= CAP:"<<(struct_input.method != CAP)<<"!= UESER:"<<(struct_input.method != USER)<<"!= NICK:"<<(struct_input.method != NICK)<<std::endl;
	// std::cout<<"method:"<<struct_input.method<<std::endl;
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
bool (IRC_Server::*fun[])(const struct input&, IRC_Client&, const std::string &) = {
   &IRC_Server::cap,       // Négociation des capacités
   &IRC_Server::pass,      // Vérification du mot de passe
   &IRC_Server::nick,      // Gestion du nickname
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
	if (user_input.method < END_METHOD)
		(this->*fun[user_input.method])(user_input, client, line);
}
