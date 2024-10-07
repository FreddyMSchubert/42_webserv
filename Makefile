NAME = webserv

SRC = $(shell find ./src -name "*.cpp")
OBJ_DIR = ./obj
OBJ = $(SRC:./src/%.c=$(OBJ_DIR)/%.o)

HEADERS := -I ./include
LIBS := 
CFLAGS := -Wall -Wextra -Werror -std=c++17 -Wc++11-extensions

$(NAME): $(OBJ)
	c++ $(CFLAGS) $(OBJ) $(LIBS) $(HEADERS) -o $(NAME)

all: $(NAME)
	
clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

run: all
	./$(NAME)

debug: CFLAGS += -g -O0 -fsanitize=address -g
debug: fclean all

.PHONY: all clean fclean re run debug