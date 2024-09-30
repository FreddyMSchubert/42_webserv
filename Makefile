NAME = webserv

SRC = $(shell find ./src -name "*.cpp")
OBJ_DIR = ./obj
OBJ = $(SRC:./src/%.c=$(OBJ_DIR)/%.o)

HEADERS := -I ./include
LIBS := 
CFLAGS := -Wall -Wextra -Werror -std=c++17 -Wc++11-extensions

$(NAME): $(OBJ)
	c++ $(OBJ) $(LIBS) $(HEADERS) -o $(NAME)

$(OBJ_DIR)/%.o: ./src/%.c
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(dir $@)
	c++ $(CFLAGS) -o $@ -c $< $(HEADERS) -g

all: $(NAME)
	
clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

debug: CFLAGS += -g -O0
debug: fclean all

.PHONY: all clean fclean re ebug