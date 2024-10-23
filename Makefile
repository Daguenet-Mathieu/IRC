NAME = ircserv

SRCS =	main.cpp		\
		IRC_Server.cpp	\
		IRC_Client.cpp

OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

CC = c++

FLAGS = -Wall -Werror -Wextra -g -std=c++98 -g

$(NAME) : $(OBJS)
		 $(CC) $(FLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
		$(CC) $(FLAGS) -MMD -c $< -o $@

all : $(NAME)

clean :
		rm -f $(OBJS) $(DEPS)

fclean : clean
		rm -f $(NAME) $(DEPS)

re : fclean all

-include : $(DEPS)

.PHONY: all clean fclean re
