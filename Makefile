
OBJ_DIR = obj
SRC_DIR = src
INCLUDE_DIR = includes/

SRC_FILES =	main.cpp \
			utils.cpp \
			Client/Client.cpp \
			Client/Client_canon.cpp \
			Config/ConfigParser.cpp \
			Config/ConfigParser_server.cpp \
			Config/ConfigParser_location.cpp \
			Config/ConfigParser_validate.cpp \
			Config/ConfigTokenizer.cpp \
			Config/Config_Structs.cpp \
			Request/Request_canon.cpp \
			Request/Request_helpers.cpp \
			Router/RouterHandlers.cpp \
			Router/RouterHelpers.cpp	\
			Router/Router.cpp \
			Response/ErrorBuilder.cpp \
			Response/Logger.cpp \
			Response/Response_builder.cpp \
			Response/Response_canon.cpp \
			Response/MimeType.cpp \
			Response/Autoindex.cpp \
			Webserv/Listener_canon.cpp \
			Webserv/Webserv_canon.cpp \
			Webserv/Webserv_main_loop.cpp \
			Webserv/Webserv_graceful_termination.cpp \
			Webserv/Webserv_epoll.cpp \
			Webserv/Webserv_handlers.cpp \
			Webserv/Webserv_signals.cpp \
			Webserv/Webserv_sockets.cpp \
			Webserv/Webserv_timeout.cpp \
			CGI/CgiHandlers.cpp \
			CGI/CgiChild.cpp \
			CGI/CgiSetup.cpp \
			CGI/CgiHelpers.cpp

SRC = $(addprefix $(SRC_DIR), $(SRC_FILES))
OBJ = $(addprefix $(OBJ_DIR)/, $(SRC_FILES:.cpp=.o))
DEP = $(OBJ:.o=.d)

CXX = c++
FLAGS = -Wall -Wextra -Werror -std=c++98 -MMD -MP -Wshadow -g

NAME = webserv

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(FLAGS) $(OBJ) -I$(INCLUDE_DIR) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(FLAGS) -I$(INCLUDE_DIR) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

run: $(NAME)
	./$(NAME) config.conf

leaks: $(NAME)
	valgrind --show-leak-kinds=all --leak-check=full --track-fds=all -s \
	--track-origins=yes --trace-children=yes ./$(NAME) config.conf

script: $(NAME)
	valgrind --track-fds=all --trace-children=yes ./$(NAME) config.conf

-include $(DEP)

test:
	./tests/functional.sh

stress:
	./tests/siege_test.sh

.PHONY: all clean fclean re run leaks