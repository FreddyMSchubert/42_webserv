NAME = webserv

SRC = $(shell find ./src -name "*.cpp")
HFL = $(shell find ./include -name "*.hpp")
OBJ_DIR = ./obj
OBJ = $(SRC:./src/%.c=$(OBJ_DIR)/%.o)

HEADERS := -I ./include -I ./include/Packets
LIBS := 
CFLAGS := -Wall -Wextra  -std=c++17 -Wc++11-extensions

$(NAME): $(OBJ)
	c++ $(CFLAGS) $(OBJ) $(LIBS) $(HEADERS) -o $(NAME)

%.o: %.cpp $(HFL)
	c++ $(FLAGS) -c $< -o $@ $(HEADERS)

all: $(NAME)
	
clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

run: all
	./$(NAME)

debug: CFLAGS += -g -O0 -fsanitize=address -g
debug: CFLAGS += -DLOG_INCOMING_PACKETS -DLOG_OUTGOING_PACKETS

.PHONY: all clean fclean re run debug