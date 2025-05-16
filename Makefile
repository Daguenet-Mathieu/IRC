NAME = ircserv

SRCS =	main.cpp 	\
	IRC_Server.cpp	\
	IRC_Client.cpp

OBJDIR = obj
DEPDIR = dep

OBJS = $(addprefix $(OBJDIR)/,$(SRCS:.cpp=.o))
DEPS = $(addprefix $(DEPDIR)/,$(SRCS:.cpp=.d))

CC = c++

FLAGS = -Wall -Werror -Wextra -g -std=c++98

$(NAME) : $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $(NAME)

$(OBJDIR)/%.o : %.cpp | $(OBJDIR) $(DEPDIR)
	$(CC) $(FLAGS) -MMD -c $< -o $@
	mv $(OBJDIR)/$(<:.cpp=.d) $(DEPDIR)/

$(OBJDIR) $(DEPDIR):
	mkdir -p $@

all : $(NAME)

clean :
	rm -f $(OBJS) $(DEPS)
	rm -rf $(OBJDIR) $(DEPDIR)

fclean : clean
	rm -f $(NAME)

re : fclean all

-include $(DEPS)

.PHONY: all clean fclean re