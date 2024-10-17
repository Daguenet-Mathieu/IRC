#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

int	main(int agrc, char** agrv)
{
	SOCKET	sock_server = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		std::cerr << "INVALID SOCKET" << std::endl;
		return 0;
	}

	int					port = 4444;
	struct sockaddrr_in	address;

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	if (bind(sock_server, (struct sockaddr *)&address, sizeof(address)) == -1)
	{
		std::cerr << "BIND ERROR" << std::endl;
		close(sock_server);
		return 0;
	}
	std::cout << "Successful socket binding, port : " << port << std::endl;<
}
