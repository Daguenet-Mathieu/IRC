#include "IRC_Server.hpp"

void signalHandler(int signal) {
    std::cout << "Signal " << signal << " received.\n";
    throw std::bad_exception();  // Lancer l'exception lorsqu'un signal est reçu
}

int	check_arg_port(char* argument)
{
	errno = 0;
	char*	endptr;
	long	port = strtol(argument, &endptr, 10);

	if (errno == ERANGE || *endptr != '\0' || port < 1000 || port > 9999)
		return -1 ;
	return static_cast<int>(port);
}

int	main(int argc, char** argv)
{
    signal(SIGINT, signalHandler);
    signal(SIGQUIT, signalHandler);

	if (argc != 3)
	{
		std::cerr << "Invalid number of arguments" << std::endl;
		return 0;
	}

	int	port = check_arg_port(argv[1]);

	if (port == -1)
	{
		std::cerr << "Invalid input, port range : 1000 - 9999" << std::endl;
		return 0;
	}
	try
	{
		std::string	password(argv[2]);
		IRC_Server	serv(port, password);

		serv.manage();
	}
	catch (std::bad_exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	catch (std::exception & e)
	{
		std::cerr << e.what() << std::endl;
	}
	return 1;
}
