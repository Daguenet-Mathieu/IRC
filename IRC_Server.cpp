#include "IRC_Server.hpp"
#include <unistd.h>
#include <stdio.h>
#include <errno.h>


//CONSTRUCTOR

IRC_Server::IRC_Server(int port, const std::string& password) :_password(password),  _port(port){
	_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket == -1)
		throw ThrowException("ERROR SOCKET");
	memset(&_server_addr, 0, sizeof(_server_addr));
	_server_addr.sin_family = AF_INET;
	_server_addr.sin_addr.s_addr = INADDR_ANY;
	_server_addr.sin_port = htons(_port);
	_create_time = getCurrentDateTime();
}

//DESTRUCTOR

IRC_Server::~IRC_Server()
{
	if (_socket != -1)
		close(_socket);
	for (std::map<std::string, IRC_Channel*>::iterator it = _channels.begin(); it != _channels.end(); it++)
		delete it->second;
	for (size_t i = 0; i < _clients.size(); i++)
	{
		delete(_clients[i]);
	}
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
	std::string channel_list = line;
	size_t comma_index = 0;

	while ((comma_index = channel_list.find_first_of(',')) != std::string::npos)
	{
		channels.push_back(channel_list.substr(0, comma_index));
		channel_list.erase(0, comma_index + 1);
	}
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

    strftime(buffer, sizeof(buffer), "%d %B %Y %H:%M:%S", &tstruct);
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
		ip = ntohs(client_addr.sin_port);
		IRC_Client *client = new IRC_Client(client_fd, client_ip);
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

void IRC_Server::check_socket_client()
{
	for (int i = 0; i < static_cast<int>(this->_clients.size()); i++)
	{
		if (FD_ISSET(this->_clients[i]->get_socket_client(), &_exceptfds))
			throw std::runtime_error("error in socket client");

		if (FD_ISSET(this->_clients[i]->get_socket_client(), &_writefds))
		{
			std::string line;
			if (this->_clients[i]->get_input_client(line))
			{
				if (this->launch_method(this->parse_data(line, *this->_clients[i]), line, *this->_clients[i]) == false)
				{
					i++;
					continue;
				}
			}
			bool res = this->_clients[i]->send_output_client();
			if (this->_clients[i]->get_state() == ERROR && res == false)
			{
				IRC_Client* tmp = this->_clients[i];
				this->_clients.erase(this->_clients.begin() + i);
				delete tmp;
				continue;
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
		"PING",
		"WHOIS",
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
	request_info.method = i;
	return (request_info);
}

//COMMANDS

bool	IRC_Server::cap(IRC_Client &client, const std::string &line)
{
	if (get_word(line, 2) != "END"&& get_word(line, 2) != "LS")
	{
		client.set_output_client(error_unknown_command(client.get_nickname(), get_word(line, 2)));
		return (false);
	}
	if (get_word(line, 2) == "END")
	{
		if (client.get_username().size() == 0 || client.get_nickname().size() == 0)
			return false;
		return true;	
	}
	std::string response = ":server CAP * LS :KICK INVITE TOPIC MODE\r\n";
	client.set_output_client(response);
	return true;
}

bool	IRC_Server::pass(IRC_Client &client, const std::string &line)
{
	std::string response = ":464 MADAGUENAUFERRANIRC :Password incorrect\r\n";
	if (client.get_state() == CONNECTED)
		return (true);
	if (get_word(line, 2) == std::string(SUPERUSER))
	{
		client.set_role(SUPER_OPERATEUR);
		return true;
	}
	if (get_word(line, 2) != this->_password)
	{
		client.set_output_client(response);
		return false;
	}
	client.set_role(MEMBER);
	return true;
}

bool	IRC_Server::nick(IRC_Client &c, const std::string &line)
{
	std::string	nickname = get_word(line, 2);

	if (nickname.size() == 0)
	{
		c.set_output_client(error_not_enough_parameters(c.get_nickname(), "NICK"));
		return true;
	}
	for (size_t i = 0; i < nickname.size(); i++)
	{
		if (!isdigit(nickname[i]) && !isupper(nickname[i]) && !islower(nickname[i]))
		{
			c.set_output_client(error_nickname(nickname));
			return true;
		}
	}
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (nickname == _clients[i]->get_nickname())
		{
			c.set_output_client(error_nick_already_used(nickname));
			return true;
		}
	}
	if (c.get_role() != NONE && c.get_username().size() > 0 && c.get_nickname().size() > 0 && c.get_state() != CONNECTED)
		c.set_state(INSTANCE_CONNECT);
	std::string old_nick = c.get_nickname();
	std::string prefix = c.get_prefix();
	c.set_nickname(nickname);
	std::string response = prefix + " NICK " + nickname + "\r\n";
	for (std::map<std::string, IRC_Channel*>::iterator it = _channels.begin(); it != _channels.end(); it++)
	{
		it->second->update_nick(old_nick, nickname);
	}
	c.set_output_client(response);
	return true;
}

bool	IRC_Server::user(IRC_Client &client, const std::string &line)
{
	if (client.get_username().size() == 0)
	{
		std::string username = get_word(line, 2);
		client.set_username(username);
		if (client.get_role() != NONE && client.get_username().size() > 0 && client.get_nickname().size() > 0 && client.get_state() != CONNECTED)
			client.set_state(INSTANCE_CONNECT);
	}
	return true;
}

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
	std::vector<std::string>	channel_list = split_channels(get_word(line, 2));
	std::string 				password = get_word(line, 3);
	std::string 				channel = get_word(line, 2);

	if (channel.size() == 0)
	{
		client.set_output_client(error_not_enough_parameters(client.get_nickname(), "JOIN"));
		return true;
	}
	for (std::vector<std::string>::iterator it = channel_list.begin(); it != channel_list.end(); it++)
	{
		channel = *it;
		if (channel.size() >= 1 && (channel[0] == '#' || channel[0] == '&'))
			channel.erase(0, 1);
		else
		{
			client.set_output_client(error_no_nick_channel(client.get_nickname(), channel));
			continue ;
		}
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
			else
			{
				if (!(_channels[channel]->get_invite() == false || _channels[channel]->is_invited(client.get_nickname())))
				{
					client.set_output_client(error_cannot_join_channel("473", client.get_nickname(), channel, "i"));
					return true;
				}
				int	limit = _channels[channel]->get_limit();
				if (limit != -1 && _channels[channel]->get_nb_user() >= limit)
				{
					client.set_output_client(error_cannot_join_channel("471", client.get_nickname(), channel, "l"));
					return true;
				}
				if ((password.size() == 0 && _channels[channel]->get_password().size() == 0) || password == _channels[channel]->get_password())
					_channels[channel]->set_client_status(client.get_nickname(), MEMBER);
				else
				{
					client.set_output_client(error_cannot_join_channel("475", client.get_nickname(), *it, "k"));
					return true;
				}
			}
		}
		std::string response = client.get_prefix() + " JOIN :" + *it + "\r\n"
    		+ ":server 353 " + client.get_nickname() + " = " + *it + " :" + get_users_channel(channel) + "\r\n"
    		+ ":server 366 " + client.get_nickname() + " " + *it + " :End of NAMES list\r\n";
		client.set_output_client(response);
	}
	return true;
}

bool	IRC_Server::kick(IRC_Client& client, const std::string& line)
{
	std::string channel = get_word(line, 2);
	std::vector<std::string>	usernames = split_channels(get_word(line, 3));
	
	if (usernames.size() == 0 || channel.size() == 0)
	{
		client.set_output_client(error_not_enough_parameters(client.get_nickname(), "KICK"));
		return true;
	}
	std::string	reason = get_word(line, 4);
	if (reason.size() == 0)
		reason = "bye bye kicked user";
	else
	{
		size_t		pos = line.find(reason);
		std::string	message = line.substr(pos, line.size());
		reason = message;
	}
	size_t i;
	for (size_t j = 0; j < usernames.size(); j++)
	{
		std::string username = usernames[j];
		for (i = 0; i < _clients.size(); i++)
		{
			if (_clients[i]->get_nickname() == username)
				break;
		}
		if (i >= _clients.size())
		{
			client.set_output_client(error_no_nick_channel(client.get_nickname(), username));
			return true;
		}
		if (channel[0] == '#' || channel[0] == '&')
			channel.erase(0, 1);
		if (_channels.find(channel) == _channels.end())
		{
			client.set_output_client(error_no_nick_channel(client.get_nickname(), " #" + channel));
			return true;
		}
		if (_channels[channel]->in_channel(client.get_nickname()) == false)
		{
			client.set_output_client(error_not_on_channel(client.get_username(), " #" + channel));
			return true;
		}
		int level = _channels[channel]->get_client_status(client.get_nickname());
		if (level > OPERATOR || _channels[channel]->get_client_status(username) == SUPER_OPERATEUR)
		{
			client.set_output_client(error_not_operator(client.get_nickname(), "#" + channel));
			return true;
		}
		_channels[channel]->remove_client(username);
		std::string response = client.get_prefix() + " KICK " + " #" + channel + " " + username + " " + reason + "\r\n";
		_clients[i]->set_output_client(response);
		client.set_output_client(response);
		if (_channels[channel]->get_channel_clients().size() == 0)
		{
			delete _channels[channel];
			_channels.erase(channel);
		}
	}
	return true;
}

bool	IRC_Server::invite(IRC_Client &client, const std::string &line)
{
	std::string username = get_word(line, 2);
	std::string channel = get_word(line, 3);

	if (username.size() == 0 || channel.size() == 0)
	{
		client.set_output_client(error_not_enough_parameters(client.get_nickname(), "INVITE"));
		return true;
	}
	size_t i;
	for (i = 0; i < _clients.size(); i++)
	{
		if (_clients[i]->get_nickname() == username)
			break;
	}
	if (i >= _clients.size())
	{
		client.set_output_client(error_no_nick_channel(client.get_nickname(), username));
		return true;
	}
	if (channel[0] == '#' || channel[0] == '&')
		channel.erase(0, 1);
	if (_channels.find(channel) == _channels.end())
	{
		client.set_output_client(error_no_nick_channel(client.get_nickname(), " #" + channel));
		return true;
	}
	if (_channels[channel]->in_channel(client.get_nickname()) == false)
	{
		client.set_output_client(error_not_on_channel(client.get_nickname(), " #" + channel));
		return true;
	}
	if (_channels[channel]->get_invite() == true && _channels[channel]->get_client_status(client.get_nickname()) > OPERATOR)
	{
		client.set_output_client(error_not_operator(client.get_nickname(), " #" + channel));
		return true ;
	}
	_channels[channel]->set_invite(username);
	std::string response = client.get_prefix() + " INVITE " + username + " #" + channel + "\r\n";
	_clients[i]->set_output_client(response);
	client.set_output_client(response);
	return true;
}

bool	IRC_Server::topic(IRC_Client& client, const std::string& line)
{
	std::string	channel = get_word(line, 2);
	std::string	topic = get_word(line, 3);

	if (channel.size() == 0 || topic.size() == 0)
	{
		client.set_output_client(error_not_enough_parameters(client.get_nickname(), "TOPIC"));
		return true;
	}
	if (channel.size() >= 1 && (channel[0] == '#' || channel[0] == '&'))
		channel.erase(0, 1);
	else
	{
		client.set_output_client(error_no_nick_channel(client.get_nickname(), channel));
		return true;
	}
	if (_channels.find(channel) == _channels.end())
	{
		client.set_output_client(error_no_nick_channel(client.get_nickname(), " #" + channel));
		return true;
	}
	if (_channels[channel]->set_topic(topic, client.get_nickname()) == true)
	{
		std::string response = client.get_prefix() + std::string(" TOPIC ") + "#" + channel + " " + topic + "\r\n";
		send_to_channel(*_channels[channel], response, client, false);//ou true a tester
	}
	else
		client.set_output_client(error_not_operator(client.get_nickname(), "#" + channel));
	return true;
}

bool	IRC_Server::mode(IRC_Client& client, const std::string& line)
{
	std::string	channel = get_word(line, 2);
	std::string	arg = get_word(line, 3);
	std::string target = channel;
	std::string	nb;
	if (channel.size() == 0 || arg.size() == 0)
	{
		client.set_output_client(error_not_enough_parameters(client.get_nickname(), "MODE"));
		return true;
	}
	if (channel[0] == '#' || channel[0] == '&')
	{
		channel.erase(0, 1);
		if (_channels.find(channel) == _channels.end())
		{
			client.set_output_client(error_no_nick_channel(client.get_nickname(), " #" + channel));
			return true;
		}
	}
	else
	{
		if (_channels.find(channel) == _channels.end())
		{
			client.set_output_client("MODE " + client.get_username() + " +i\r\n");
			return (true);
		}
	}
	if (_channels[channel]->in_channel(client.get_nickname()) == false)
	{
		client.set_output_client(error_not_on_channel(client.get_nickname(), " #" + channel));
		return true;
	}
	if (_channels[channel]->get_client_status(client.get_nickname()) > OPERATOR)
	{
		client.set_output_client(error_not_operator(client.get_nickname(), "#" + channel));
		return true;
	} 
	if (arg == "+i" || arg == "-i"){
		_channels[channel]->mode_i(client.get_nickname(), arg);
	}
	if (arg == "+t" || arg == "-t")
		_channels[channel]->mode_t(client.get_nickname(), arg);
	if (arg == "+k" || arg == "-k")
	{
		std::string	password = get_word(line, 4);
		if (password.size() == 0)
		{
			client.set_output_client(error_not_enough_parameters(client.get_nickname(), "MODE"));
			return true;
		}
		std::cout << "password : " << _channels[channel]->get_password() << std::endl;
		_channels[channel]->mode_k(client.get_nickname(), arg, password);
		std::cout << "password : " << _channels[channel]->get_password() << std::endl;
	}
	if (arg == "+o" || arg == "-o")
	{
		target = get_word(line, 4);
		if (target.size() == 0)
		{
			client.set_output_client(error_not_enough_parameters(client.get_nickname(), "MODE"));
			return true;
		}

		_channels[channel]->mode_o(client.get_nickname(), arg, target);
	}
	if (arg == "+l" || arg == "-l")
	{
		nb = get_word(line, 4);
		long	l = -1;

		if (arg == "+l")
		{
			if (nb.size() == 0)
			{
				client.set_output_client(error_not_enough_parameters(client.get_nickname(), "MODE"));
				return true;
			}
			char*		ptr;
			l = strtol(nb.c_str(), &ptr, 10);

			if (ptr - nb.c_str() > 9 || l > INT_MAX || l < 0 || *ptr != 0)
			{
				client.set_output_client(error_invalid_mode_param(client.get_nickname(), "#" + channel, nb));
				return true;
			}
			_channels[channel]->mode_l(client.get_nickname(), arg, l);
		}
		else
			_channels[channel]->mode_l(client.get_nickname(), arg, l);
	}
	std::string response = client.get_prefix() + " MODE " + target + " " + arg + " " + nb + "\r\n";
	send_to_channel(*_channels[channel], response, client, true);
	return true;
}

bool	IRC_Server::privmsg(IRC_Client& client, const std::string& line)
{
	std::string	receiver = get_word(line, 2);
	std::string	response;
	std::string tmp;
	
	std::string	word = get_word(line, 3);
	size_t		pos = line.find(word);
	std::string	message = line.substr(pos, line.size());

	if (receiver.size() == 0 || message.size() == 0)
	{
		client.set_output_client(error_not_enough_parameters(client.get_nickname(), "PRIVMSG"));
		return true;
	}
	if (message[0] == ':')
		message.erase(0, 1);
	if (receiver[0] == '#' || receiver[0] == '&')
	{
		receiver.erase(0, 1);
		if (_channels.find(receiver) == _channels.end())
		{
			client.set_output_client(error_no_nick_channel(client.get_nickname(), receiver));
			return true;

		}
		if (_channels[receiver]->in_channel(client.get_nickname()) == false)
		{
			client.set_output_client(error_not_on_channel(client.get_username(), " #" + receiver));
			return true;
		}
		response = client.get_prefix() + " PRIVMSG #" + receiver + " :" + message + "\r\n";
		send_to_channel(*_channels[receiver], response, client, false);
	}
	else
	{
		size_t i;
		for (i = 0; i < _clients.size(); i++)
		{
			if (_clients[i]->get_nickname() == receiver)
				break ;
		}
		if (i >= _clients.size())
		{
			client.set_output_client(error_no_nick_channel(client.get_nickname(), receiver));
			return true;
		}
		else
		{
			response = client.get_prefix() + " PRIVMSG " + receiver + " :" + message + "\r\n";
			_clients[i]->set_output_client(response);
			return true;
		}

	}
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
	return true;
}

bool	IRC_Server::part(IRC_Client &client, const std::string &line)
{
	std::vector<std::string>	channel_list = split_channels(get_word(line, 2));
	std::string channel = get_word(line, 2);

	for(std::vector<std::string>::iterator it = channel_list.begin(); it != channel_list.end(); it++)
	{
		channel = *it;
		if (channel.size() >= 1 && (channel[0] == '#' || channel[0] == '&'))
			channel.erase(0, 1);
		if (channel.size() == 0 || _channels.find(channel) == _channels.end())
		{
			client.set_output_client(error_no_nick_channel(client.get_nickname(), *it));
			continue;
		}
		std::string message = get_word(line, 3);
		std::string response = client.get_prefix() + " PART #" + channel +" :" + message + "\r\n";
		client.set_output_client(response);
		send_to_channel(*_channels[channel], response, client, true);
		_channels[channel]->remove_client(client.get_nickname());
		if (_channels[channel]->get_channel_clients().size() == 0)
		{
			delete _channels[channel];
			_channels.erase(channel);
		}
	}
	return true;
}

void IRC_Server::send_to_channel(const IRC_Channel& channel, const std::string& response, IRC_Client& client, bool include_sender)
{
	if (channel.in_channel(client.get_nickname()))
	{
		std::vector<std::string>	current_channel_clients = channel.get_channel_clients();
	
		for (size_t i = 0; i < current_channel_clients.size(); i++)
		{
			for (size_t j = 0; j < _clients.size(); j++)
			{
				if (client.get_nickname() == _clients[j]->get_nickname() && include_sender == false)
					continue ;
				if (_clients[j]->get_nickname() == current_channel_clients[i])
					_clients[j]->set_output_client(response);
			}
		}
	}
}

bool	IRC_Server::quit(IRC_Client& client, const std::string& line)
{
	std::string	response;

	for (std::map<std::string, IRC_Channel*>::iterator it = _channels.begin(); it != _channels.end(); )
	{
		if (it->second->in_channel(client.get_nickname()))
		{
			std::string arg = get_word(line, 3).size() == 0 ? "exited" : get_word(line, 3);
			response = client.get_prefix() + " QUIT " + "#" + it->first + " :" + arg + "\r\n";
			send_to_channel(*it->second, response, client, true);
			it->second->remove_client(client.get_nickname());
			if (_channels[it->first]->get_channel_clients().size() == 0)
			{
				std::map<std::string, IRC_Channel*>::iterator next = it;
				next++;
				delete _channels[it->first];
				_channels.erase(it);
				it = next;
			}
		}
		it++;
	}
	return false;
} 

bool	IRC_Server::names(IRC_Client &, const std::string &)
{
	return true;
}

bool	IRC_Server::list(IRC_Client &, const std::string &)
{
	return true;

} 

void IRC_Server::bot_help(std::string& line, const std::string& prefix, const IRC_Client&)
{
	line = prefix + "Available commands: \r\n";
	line += prefix + "?help, !help : display bot options, \r\n";
	line += prefix + "?log : display your session logtime, \r\n";
	line += prefix + "?ping : check bot activity, \r\n";
	line += prefix + "?dice <size=6> : launch dice, \r\n";
	line += prefix + "?seen <user> : show last activity of a user.\r\n";
}

void	IRC_Server::bot_log(std::string& line, const std::string& prefix, const IRC_Client& c)
{
	line = prefix + " you are log since : " + c.get_connect_time() + "\r\n";
}


void	IRC_Server::bot_ping(std::string& line, const std::string&prefix, const IRC_Client&)
{
	line = prefix + "pong?\r\n";
	line += prefix + "rdv upf2!\r\n";
}

void	IRC_Server::bot_dice(std::string&line, const std::string&prefix, const IRC_Client&)
{
	char	*ptr;
	std::string nb = get_word(line, 4);
	long int dice = 6;
	if (nb.size() > 0)
	{
		dice = strtol(nb.c_str(), &ptr, 10);
		if (ptr - nb.c_str() > 9 || dice > 100 || dice < 0 || *ptr != 0)
		{
			dice = 42;
		}
	}
	int result = (rand() % dice) + 1;
	std::ostringstream oss;
	oss << prefix << "dice of " << dice << " result : " << result << "\r\n";
	line = oss.str();
}

void	IRC_Server::bot_seen(std::string&line, const std::string&prefix, const IRC_Client&client)
{
	std::string target = get_word(line, 4);
	size_t i;
	for (i = 0; i < _clients.size(); i++)
	{
			if (_clients[i]->get_nickname() == target)
			break ;
	}
	if (i >= _clients.size())
	{
		line = prefix + error_no_nick_channel("BOT", target);
		return ;
	}

	std::ostringstream oss;
	oss << prefix << target << "last activity : " << size_t(client.get_time() -_clients[i]->get_last_activity()) << "seconds\r\n";
	line = oss.str();
	
}


bool IRC_Server::do_bot_actions(int input, std::string &line, IRC_Client &client)
{
	const char *bot_prefix = ":BOT!BOT@bot.com PRIVMSG #";
    void (IRC_Server::*const bot_functions[])(std::string&, const std::string&, const IRC_Client&) = {
        &IRC_Server::bot_help,
        &IRC_Server::bot_help,
        &IRC_Server::bot_log,
        &IRC_Server::bot_ping,
        &IRC_Server::bot_dice,
        &IRC_Server::bot_seen
    };

    if (input != PRIVMSG)
        return false;

    size_t i;
    std::string word = get_word(line, 3);
	std::string channel = get_word(line, 2);
	
	//check channel exite et user dans channel
	//add le channel au prefix sauf a la 1ere ligne erase la ligne
	if (word[0] == ':')
	{
		word.erase(0, 1);
	}
    for (i = 0; BOT_ACTIONS[i]; i++)
    {
		if (word == BOT_ACTIONS[i])
		break;
}

	if (BOT_ACTIONS[i] == NULL)
		return false;

	if (channel[0] == '#' || channel[0] == '&')
		channel.erase(0, 1);
	if (_channels.find(channel) == _channels.end() || _channels[channel]->in_channel(client.get_username()) == false)
		return false;
    std::string prefix = std::string(bot_prefix) + channel + " :";
    (this->*bot_functions[i])(line, prefix, client);
	client.set_output_client(line);
	return true;
}

bool	IRC_Server::launch_method(const struct input &user_input, std::string &line, IRC_Client &client)
{
	int method = user_input.method;
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
	if (client.get_state() == CONNECTED)
	{
		if (do_bot_actions(user_input.method, line, client))
			return true;
	}
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
	&IRC_Server::ping,      // Gestion du ping
	&IRC_Server::whois,     // Info sur un user
	&IRC_Server::part,      // Alternative à leave
	&IRC_Server::quit,      // Déconnexion
	&IRC_Server::names,     // Liste des users
	&IRC_Server::list,      // Liste des channels
	NULL                    // Fin du tableau
	};

	if (method < END_METHOD)
	{
		if ((this->*fun[method])(client, line) == false){
			// this->_clients.erase(this->_clients.begin() + i);
			// delete(_clients[i]);
			client.set_state(ERROR);
			return (true);
		}
	}
	else
		client.set_output_client(error_unknown_command(client.get_nickname(), get_word(line, 1)));
	return (true);
}

//ERRORS

std::string	IRC_Server::error_nickname(const std::string& nickname) const
{
	return std::string(":MADAGUENAUFERRANIRC 432 ") +  nickname + " :Erroneous nickname\r\n";
}

std::string	IRC_Server::error_nick_already_used(const std::string& nickname) const
{
	return std::string(":MADAGUENAUFERRANIRC 433 ") +  nickname + " :Nickname is already in use\r\n";
}

std::string	IRC_Server::error_not_enough_parameters(const std::string& nickname, const std::string& command) const
{
	return std::string(":") + "MADAGUENAUFERRANIRC" + " 461 " + nickname + " " + command + " :Not enough parameters\r\n";
}

std::string	IRC_Server::error_no_nick_channel(const std::string& nickname, const std::string& target) const
{
	return std::string(":MADAGUENAUFERRANIRC 401 ") + nickname + " " + target + " :No such nick/channel\r\n";
}

std::string	IRC_Server::error_not_on_channel(const std::string& username, const std::string& receiver) const
{
	return std::string(":MADAGUENAUFERRANIRC 442 ") + username + " " + receiver + " :You're not on that channel\r\n";
}

std::string	IRC_Server::error_cannot_join_channel(const std::string& code, const std::string& nickname, const std::string& target, const std::string& arg) const
{
	return std::string(":") + " " + code + " " + nickname + " " + target + " :Cannot join channel (+" + arg + ")\r\n";
}

std::string	IRC_Server::error_invalid_mode_param(const std::string& nickname, const std::string& channel, const std::string& word) const
{
	return std::string(":") + "MADAGUENAUFERRANIRC" + " 696 " + nickname + " " + channel + " +l " +  word + ":Invalid mode parameter\r\n";
}

std::string	IRC_Server::error_not_operator(const std::string& nickname, const std::string& channel) const
{
	return std::string(":MADAGUENAUFERRANIRC 482 ") + nickname + " " + channel + " :You're not channel operator\r\n";
}

std::string	IRC_Server::error_unknown_command(const std::string& nickname, const std::string& word) const
{
	return std::string(":MADAGUENAUFERRANIRC 421 ") + nickname + " " + word +  " :Unknown command\r\n";
}