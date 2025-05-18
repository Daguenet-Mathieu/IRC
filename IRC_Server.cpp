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
	for (std::map<std::string, IRC_Channel*>::iterator it = _channels.begin(); it != _channels.end(); it++)
		delete it->second;
}

//FUNCTIONS

std::string IRC_Server::get_word(const std::string &line, int word_index)
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

std::vector<std::string>	IRC_Server::split_channels(const std::string& line)
{
	std::vector<std::string>	channels;
	std::string channel_list = get_word(line, 2);
	size_t comma_index = 0;

	std::cout << "split_channels channel_list = " << channel_list << std::endl;
	while ((comma_index = channel_list.find_first_of(',')) != std::string::npos)
	{
		std::cout << "split_channels channel_list = " << channel_list << std::endl;
		channels.push_back(channel_list.substr(0, comma_index));
		channel_list.erase(0, comma_index + 1);
	}
	std::cout << "split_channels channel_list = " << channel_list << std::endl;
	channels.push_back(channel_list);
	return channels;
}

std::vector<std::string>	IRC_Server::get_client_channels(const std::string& client){
	std::vector<std::string>	channels;

	for (std::map<std::string, IRC_Channel*>::iterator it = _channels.begin(); it != _channels.end(); it++){
		if(it->second->in_channel(client))
			channels.push_back(it->first);
	}
	return channels;
}

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
		int		client_fd;
		char	*client_ip;

		struct sockaddr_in client_addr;
		socklen_t addr_len = sizeof(client_addr);
		client_fd = accept(this->_socket, (struct sockaddr *)&client_addr, &addr_len);
		if (client_fd == -1)
			return ;
		client_ip = inet_ntoa(client_addr.sin_addr);
		std::cout<<"ip client : "<<client_ip<<std::endl;
		ip = ntohs(client_addr.sin_port);
		IRC_Client *client = new IRC_Client(client_fd, client_ip);
		std::cout<<"Connexion acceptée de "<<client_ip<<" : "<<ntohs(client_addr.sin_port)<<std::endl;
		this->_clients.push_back(client);
	}
}

void	IRC_Server::read_socket_client(int i)
{
	char		buffer[2048];

	memset(&buffer, 0, sizeof(buffer));
	ssize_t	bytes_received = recv(this->_clients[i]->get_socket_client(), buffer, sizeof(buffer), 0);
	if (bytes_received > 0)
	{
		std::cout << buffer << std::endl;
		_clients[i]->fill_input_client(buffer, bytes_received);
		std::string input(buffer);
	}
	else if (bytes_received == 0)
	{
		FD_CLR(this->_clients[i]->get_socket_client(), &_readfds);
		delete(this->_clients[i]);
		this->_clients.erase(this->_clients.begin() + i);
	}
	else
		throw ThrowException("RECV ERROR");
}

void	IRC_Server::check_socket_client()
{
	for (int i = 0; i < static_cast<int>(this->_clients.size()); i++)
	{
		if (FD_ISSET(this->_clients[i]->get_socket_client(), &_exceptfds))
			throw std::runtime_error("error in socket client");
		if (FD_ISSET(this->_clients[i]->get_socket_client(), &_writefds))
		{
			std::string line;
			if (this->_clients[i]->get_input_client(line)){
				if (this->launch_method(this->parse_data(line, *this->_clients[i]), line, *this->_clients[i], i) == false)
					continue ;
			}
			bool res = this->_clients[i]->send_output_client();
			if (this->_clients[i]->get_state() == ERROR && res == false)
			{
				this->_clients.erase(this->_clients.begin() + i);
				delete(this->_clients[i]);
			}
		}
		if (FD_ISSET(this->_clients[i]->get_socket_client(), &_readfds))
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
    	FD_SET(this->_clients[i]->get_socket_client(), &_readfds);
    	FD_SET(this->_clients[i]->get_socket_client(), &_writefds);
    	FD_SET(this->_clients[i]->get_socket_client(), &_exceptfds);
    }
}

int	IRC_Server::get_nfds()
{
	int	maxi_fds = _socket;
	for (int i = 0; i < static_cast<int>(this->_clients.size()); i++)
	{
		if (this->_clients[i]->get_socket_client() > maxi_fds)
			maxi_fds = this->_clients[i]->get_socket_client();
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


struct input	IRC_Server::parse_data(const std::string &line, IRC_Client &)
{
	struct input request_info;
	std::string word;

	const char *method[] = {
		"CAP",
		"PASS",
		"NICK",
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

bool	IRC_Server::cap(IRC_Client &client, const std::string &line)
{
	std::cout<<"iun cap"<<std::endl;//check que nego est bien fini toutes les infos set et user connecte
	if (get_word(line, 2) == "END")
		return true;
	std::string response = ":server CAP * LS :KICK INVITE TOPIC MODE\r\n";
	client.set_output_client(response);
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

bool	IRC_Server::pass(IRC_Client &client, const std::string &line)
{
	// if (_password != input.password)
	// {
	
		std::string response = ":464 MADAGUENAUFERRANIRC :Password incorrect\r\n";//mettre username

	// 	std::cout<<"\t\t\t wrong password"<<std::endl;
	// 	client.set_output_client(response);
	// 	client.set_state(ERROR);
	// }
	std::cout<<"in pssrd: "<<line<<std::endl;
	if (get_word(line, 2) == std::string(SUPERUSER))
	{
		client.set_role(SUPER_OPERATEUR);
		return true;
	}
	if (get_word(line, 2)!= this->_password)
	{
		client.set_output_client(response);
		return false;
	}
	//set passwd ok?
	return true;
}

bool	IRC_Server::nick(IRC_Client &c, const std::string &line)
{
	std::cout<<"in nick"<<std::endl;
	std::cout<<"line : "<<line<<std::endl;
	//std::string IRC_Client::get_prefix() const {
 //   return ":" + _nick + "!~" + _username + "@" + _ip;}//auser pouanement denic + autres trucs?
 //verifier le nick aussi !!!
	std::string response = c.get_prefix() + " NICK " + get_word(line, 2) + "\r\n";
	std::cout<<"nick res[ponse === " << response << std::endl;
	c.set_output_client(response);
	c.set_nickname(get_word(line, 2));
	std::cout <<"new nick == "<<c.get_nickname()<<std::endl;
	// 433 ERR_NICKNAMEINUSE
	//chekc si le nick est deja utilise  sinon le changer
	//:ancien_nick!user@host NICK :nouveau_nick
	// :irc.example.com 433 * nouveau_nick :Nickname is already in use
	// :irc.example.com 432 * mauvais_nick :Erroneous nickname
//si choix invalide ? char interdit?
	return true;
}

bool	IRC_Server::user(IRC_Client &client, const std::string &line)
{
	std::cout<<"in user"<<std::endl;
	//:nick!user@host prefix
	// USER madaguen madaguen localhost :Mathieu DAGUENET
	if (client.get_username().size() == 0)
	{
		std::string username = get_word(line, 2);
		client.set_username(username);
		client.set_state(INSTANCE_CONNECT);//pour testester a degager
	}
	return true;
}
/*:nick!user@host JOIN :#newchan
:server 332 nick #newchan :       ; topic vide
:server 353 nick = #newchan :nick
:server 366 nick #newchan :End of /NAMES list.*/

/*403	Channel interdit/banni/invalide	:server 403 nick #badchan :No such channel
405	L’utilisateur est déjà dans trop de channels	:server 405 nick #chan :You have joined too many channels
471	Channel plein (si tu limites le nombre)	:server 471 nick #chan :Cannot join channel (+l)
473	Channel sur invitation uniquement	:server 473 nick #chan :Cannot join channel (+i)
474	L’utilisateur est banni	:server 474 nick #chan :Cannot join channel (+b)
475	Mot de passe requis	:server 475 nick #chan :Cannot join channel (+k)*/

std::string	IRC_Server::get_users_channel(const std::string &channel)
{
	std::vector<std::string> clients_nicks = _channels[channel]->get_formated_channel_clients();
	std::string all_nicks = "";
	for (size_t i = 0; i < clients_nicks.size(); i++)
	{
			all_nicks += clients_nicks[i] + " ";
	}
	return all_nicks;
} 

bool	IRC_Server::join(IRC_Client &client, const std::string &line)
{
	std::cout<<"in join"<<std::endl;
	std::cout << "line : " << line << std::endl;
	std::vector<std::string>	channel_list = split_channels(line);
	std::string 				password = get_word(line, 3);
	std::string 				channel = get_word(line, 2);

	if (channel.size() == 0)
	{
		std::string response = ":MADAGUENAUFERRANIRC 461 " + client.get_nickname() + " JOIN :Not enough parameters\r\n";
		client.set_output_client(response);
	}
	for (std::vector<std::string>::iterator it = channel_list.begin(); it != channel_list.end(); it++)
	{
		channel = *it;
		std::cout<<"channel avant : "<<channel<<std::endl;
		if (channel.size() >= 1 && (channel[0] == '#' || channel[0] == '&'))
			channel.erase(0, 1);
		std::cout << "channel name :" << channel << std::endl;
		if (_channels.find(channel) == _channels.end()) 
		{
			IRC_Channel* new_channel = new IRC_Channel(channel, password);

			_channels[channel] = new_channel;
			new_channel->set_client_status(client.get_nickname(), OPERATOR);
		}
		else
		{
			int	role = client.get_role();
			if (role == SUPER_OPERATEUR)
				_channels[channel]->set_client_status(client.get_nickname(), SUPER_OPERATEUR);
			if (role != SUPER_OPERATEUR && !(_channels[channel]->get_invite() == false || _channels[channel]->is_invited(client.get_nickname())))
			{
				std::string response = std::string(":") + "MADAGUENAUFERRANIRC" + " 473 " + client.get_nickname() + " " + channel + " :Cannot join channel (+i)\r\n";
				client.set_output_client(response);
				return true;
			}
			int	limit = _channels[channel]->get_limit();
			std::cout << "channel limit == " << limit << "channle nb user == " << _channels[channel]->get_nb_user()<<std::endl;
			if (role != SUPER_OPERATEUR && !(limit == -1 || _channels[channel]->get_nb_user() < limit))
			{
				std::string response = std::string(":") + "MADAGUENAUFERRANIRC" + " 471 " + client.get_nickname() + " " + channel + " :Cannot join channel (+l)\r\n";
				client.set_output_client(response);
				return true;
			}
			if ((password.size() == 0 && _channels[channel]->get_password().size() == 0) || password == _channels[channel]->get_password())
				_channels[channel]->set_client_status(client.get_nickname(), MEMBER);
			else
			{
				std::string response = std::string(":") + " 475 " + client.get_nickname() + " " + *it + " :Cannot join channel (+k)\r\n";
				client.set_output_client(response);
				return true;
			}
		}
		std::string response = client.get_prefix() + " JOIN :" + *it + "\r\n"
    		+ ":server 353 " + client.get_nickname() + " = " + *it + " :" + get_users_channel(channel) + "\r\n"
    		+ ":server 366 " + client.get_nickname() + " " + *it + " :End of NAMES list\r\n";
		// std::string response = client.get_prefix() + " JOIN :" + *it + "\r\n"
		// 			+ ":server 353 " + client.get_nickname() + " = " + *it + " :" + client.get_nickname() + "\r\n"
		// 			+ ":server 366 " + client.get_nickname() + " " + *it + " :End of NAMES list\r\n";
		client.set_output_client(response);
	}
	return true;
}

bool	IRC_Server::kick(IRC_Client &, const std::string &)
{
	std::cout<<"in kick"<<std::endl;
	return true;
}

bool	IRC_Server::invite(IRC_Client &, const std::string &)
{
	std::cout<<"in invit"<<std::endl;
	return true;
}

bool	IRC_Server::topic(IRC_Client &, const std::string &)
{
	std::cout<<"in topic"<<std::endl;
	return true;
}

bool	IRC_Server::mode(IRC_Client& client, const std::string& line)
{
	std::cout<<"in mode"<<std::endl;

	std::string	channel = get_word(line, 2);
	std::string	arg = get_word(line, 3);

	if (channel.size() == 0 || arg.size() == 0)
	{
		std::string response = std::string(":") + "MADAGUENAUFERRANIRC" + " 461 " + client.get_nickname() + " MODE :Not enough parameters\r\n";
		client.set_output_client(response);
		return true;
	}
	if (channel[0] == '#' || channel[0] == '&')
		channel.erase(0, 1);
	std::cout << "in mode : arg == " << arg << " channel == "<< channel << std::endl;

	if (arg == "+i" || arg == "-i"){

		if (_channels.find(channel) == _channels.end())
		{
			client.set_output_client("MODE " + client.get_username() + " +i\r\n");
			return (true);
		}
		_channels[channel]->mode_i(client.get_nickname(), arg);
	}
	if (arg == "+t" || arg == "-t")
		_channels[channel]->mode_t(client.get_nickname(), arg);
	if (arg == "+k" || arg == "-k")
	{
		std::string	password = get_word(line, 4);
		if (password.size() == 0)
		{
			std::string response = std::string(":") + "MADAGUENAUFERRANIRC" + " 461 " + client.get_nickname() + " MODE :Not enough parameters\r\n";
			client.set_output_client(response);
			return true;
		}

		_channels[channel]->mode_k(client.get_nickname(), arg, password);
	}
	if (arg == "+o" || arg == "-o")
	{
		std::string	target = get_word(line, 4);
		if (target.size() == 0)
		{
			std::string response = std::string(":") + "MADAGUENAUFERRANIRC" + " 461 " + client.get_nickname() + " MODE :Not enough parameters\r\n";
			client.set_output_client(response);
			return true;
		}

		_channels[channel]->mode_o(client.get_nickname(), arg, target);
	}
	if (arg == "+l" || arg == "-l")
	{
		std::string	nb = get_word(line, 4); //si vide need more params
		if (nb.size() == 0)
		{
			std::string response = std::string(":") + "MADAGUENAUFERRANIRC" + " 461 " + client.get_nickname() + " MODE :Not enough parameters\r\n";
			client.set_output_client(response);
			return true;
		}
		char*		ptr;
		long	l = strtol(nb.c_str(), &ptr, 10);

		if (ptr - nb.c_str() > 9 || l > INT_MAX || l < 0 || *ptr != 0)
		{
			std::string	response = 
				std::string(":") + "MADAGUENAUFERRANIRC" + " 696 " + client.get_nickname() + " #" + channel + " +l " +  get_word(line, 4) + ":Invalid mode parameter\r\n";
			client.set_output_client(response);
			return true;
		}
		else
			_channels[channel]->mode_l(client.get_nickname(), arg, l);
	}
	std::string response = "MODE " + client.get_username() + " " + arg + "\r\n";

	return true;
}

bool	IRC_Server::privmsg(IRC_Client &, const std::string &line)
{
	std::cout<<"in privmsg : "<<line<<std::endl;
	(void) line;

	//:<nick_emetteur>!~<user_emetteur>@<host_emetteur> PRIVMSG <target> :<message> -> identifier le clent destinataire et set dans son output
	return true;
}

bool	IRC_Server::dcc(IRC_Client &, const std::string &)
{
	std::cout<<"in dcc"<<std::endl;
	//file transfert?
	return true;
}

bool	IRC_Server::ping(IRC_Client &client, const std::string &line)
{
	std::string response = "PONG " + get_word(line, 2) + "\r\n";
	client.set_output_client(response);
	return true;
}

bool	IRC_Server::whois(IRC_Client &, const std::string &)
{
	std::cout<<"in whois"<<std::endl;
	return true;
}

bool	IRC_Server::leave(IRC_Client &, const std::string &)
{
	std::cout<<"in leave"<<std::endl;
	return true;
}

bool	IRC_Server::part(IRC_Client &client, const std::string &line)
{
	std::vector<std::string>	channel_list = split_channels(line);
	std::string channel = get_word(line, 2);

	for(std::vector<std::string>::iterator it = channel_list.begin(); it != channel_list.end(); it++)
	{
		channel = *it;
		std::cout<<"channel avant : "<<channel<<std::endl;
		if (channel.size() >= 1 && (channel[0] == '#' || channel[0] == '&'))
			channel.erase(0, 1);
		std::cout<<"channel apres : "<<channel<<std::endl;
		if (channel.size() == 0 || _channels.find(channel) == _channels.end())
		{
			std::string response = ": 403 " + client.get_nickname() + " " + *it + " :No such channel\r\n";
			client.set_output_client(response);
			continue;
		}
		std::cout<<"in part"<<std::endl;
		std::string message = get_word(line, 3);
		std::string response = client.get_prefix() + " PART " + *it +" :" + message + "\r\n";
		client.set_output_client(response);
		send_to_channel(*_channels[channel], response, client);
		_channels[channel]->remove_client(client.get_nickname());
		if (_channels[channel]->get_channel_clients().size() == 0)
		{
			delete _channels[channel];
			_channels.erase(channel);
		}
	}
	return true;
}

void IRC_Server::send_to_channel(const IRC_Channel& channel, const std::string& response, IRC_Client& client)
{
	if (channel.in_channel(client.get_nickname()))
	{
		std::vector<std::string>	current_channel_clients = channel.get_channel_clients();
	
		for (size_t i = 0; i < current_channel_clients.size(); i++)
		{
			for (size_t j = 0; j < _clients.size(); j++)
			{
				std::cout<<"client de j == "<< _clients[j]->get_nickname()<<std::endl;
				if (_clients[j]->get_nickname() == current_channel_clients[i])
				{
					_clients[j]->set_output_client(response);
					std::cout<<"leave meessage : "<<response<<std::endl;
				}
			}				
		}
	}
}

bool	IRC_Server::quit(IRC_Client& client, const std::string& line)
{
	std::cout<<"in quit"<<std::endl;
	std::string	response;

	for (std::map<std::string, IRC_Channel*>::iterator it = _channels.begin(); it != _channels.end(); it++)
	{
		std::cout<<"user "<<client.get_nickname()<<" quitte le channel : "<<it->first;
		std::string arg = get_word(line, 3).size() == 0 ? "exited" : get_word(line, 3);
		response = client.get_prefix() + " QUIT " + it->first + " :" + arg + "\r\n";
		send_to_channel(*it->second, response, client);
		it->second->remove_client(client.get_nickname());
		//quit maintenant ou attendre que IRC close sa socket ?
	}
	return true;
} 

bool	IRC_Server::names(IRC_Client &, const std::string &)
{
	std::cout<<"in names"<<std::endl;
	return true;
}

bool	IRC_Server::list(IRC_Client &, const std::string &)
{
	std::cout<<"in list"<<std::endl;
	return true;

} 

bool	IRC_Server::launch_method(const struct input &user_input, const std::string &line, IRC_Client &client, int i)
{
	(void)i;
	// if (client.get_state() == NOT_CONNECTED && (struct_input.method != PASS && struct_input.method != CAP))
	// {
	// 	//ERR_PASSWDMISMATCH (464)
	// 	//reclamer le password
	// 	;
	// }
	// if (user_input.method > PASS && client.get_state() == NOT_CONNECTED)//aadapter + nC ne seraas not connected
	// {
	// 	client.set_output_client(":MADAGUENAUFERRAN 451 * PRIVMSG :You have not registered");
	// 	close(client.get_socket_client());
	// 	this->_clients.erase(this->_clients.begin() + i);
	// 	//envoyer le client se faire foutre 3.1.1 rfc
	// 	return ;
	// }
	if (client.get_state() == INSTANCE_CONNECT){
		std::string response = std::string(":server 001 ") + client.get_nickname() + " :Welcome to the MADAGUENAUFERRANIRC SERVER\r\n";
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
	bool (IRC_Server::*fun[])(IRC_Client&, const std::string &) = {
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
	//  void (IRC_Server::*fun[])(const struct input&, IRC_Client&) = {&IRC_Server::cap,&IRC_Server::join ,&IRC_Server::nick, &IRC_Server::kick, &IRC_Server::invite, &IRC_Server::topic, &IRC_Server::mode, &IRC_Server::privmsg,
	// &IRC_Server::dcc, &IRC_Server::pong,  &IRC_Server::pass, &IRC_Server::user, &IRC_Server::whois, &IRC_Server::leave, NULL};
	// MethodFunction fun = initializeFunctions();
	if (user_input.method < END_METHOD){
		if ((this->*fun[user_input.method])(client, line) == false){
			this->_clients.erase(this->_clients.begin() + i);
			delete(_clients[i]);
			return (false);
		}
	}
	return (true);
}
