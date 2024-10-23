#ifndef IRC_CLIENT
#define IRC_CLIENT
#include <vector>
#include "IRC_Server.hpp"
// #include "IRC_Channel.hpp"??

class IRC_Client
{
    public:

        IRC_Client(int port);
        IRC_Client(const IRC_Client &);
        ~IRC_Client();

        // struct	sockaddr_in get_client_addr(void);
        int     get_socket_client(void);
        void    set_socket_client(int);
        void    set_username(const std::string &);
        std::string    get_username();
        void    set_nickname(const std::string &);
        std::string    get_nickname();
        void    set_client_confirmation(bool);
        bool    get_client_confirmation();

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

            std::string _error_message;
        };

    private:

        std::string     _username;
        std::string     _nickname;
        int             _socket_client;
        bool            _client_confirmation;
        std::vector<std::string> _connected_channel;
        // struct	sockaddr_in	_client_addr;
        // std::vector<std::string>    _channels;     
};

#endif