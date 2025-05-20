#ifndef IRC_CLIENT_HPP
#define IRC_CLIENT_HPP
#include <vector>
#include "IRC_Server.hpp"
#include <ctime>
# include <sys/time.h>

#define BUFFER_SIZE 2048
#define SUPERUSER "ADMIN"
typedef struct timeval	t_time;

enum nego
{
	NOT_CONNECTED,
    INSTANCE_CONNECT,
    CONNECTED,
    ERROR
};


class IRC_Client
{
    public:

        IRC_Client(int port);
        IRC_Client(int port, const std::string &host);
        IRC_Client(const IRC_Client &);
        ~IRC_Client();

        int             get_socket_client(void) const;
        void            set_socket_client(int);
        void            set_client_info(bool);
        bool            get_client_info() const;
        void            close_socket() const;
        void            fill_input_client(char *buffer, ssize_t size);
        bool            send_output_client();
        void            set_output_client(const std::string &output);
        bool            get_input_client(std::string &);
        void            set_state(int state);
        int             get_state() const;
        std::string     get_username() const;
        std::string     get_nickname() const;
        std::string     get_host() const;
        void            set_username(const std::string &);
        void            set_nickname(const std::string &);
        void            set_host(const std::string &);
        std::string     get_prefix() const;
        int             get_role() const;
        void            set_role(int);
        size_t          get_time() const;
        size_t          get_last_activity() const;
        std::string     get_connect_time() const;


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

        int				            _socket_client;
        bool				        _client_info;
        int				            _state;
        int                         _role;
        std::string                      _connect_time;
        size_t                      _last_activity;
        std::string			        _username;
        std::string			        _nickname;
        std::string			        _host;
        std::vector<std::string>	_connected_channel;
        std::vector<std::string>	_invited_channel;
        std::vector<char>		    _output_client;
        std::vector<char>		    _input_client;
        std::string                 getCurrentDateTime();
};

#endif
