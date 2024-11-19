NAME = webserv

SRC = $(shell find ./src -name "*.cpp")
HFL = $(shell find ./include -name "*.hpp")
OBJ_DIR = ./obj
OBJ = $(patsubst ./src/%.cpp,$(OBJ_DIR)/%.o,$(SRC))
TOTAL_FILES := $(words $(OBJ))

HEADERS := -I ./include -I ./include/Packets -I ./include/Config -I ./include/Utils
LIBS :=
CFLAGS := -Wall -Wextra -Werror -std=c++17 -g -fsanitize=address

# Colors and Emojis 🌈✨
RESET := \033[0m
RED := \033[31m
GREEN := \033[32m
YELLOW := \033[33m
BLUE := \033[34m
MAGENTA := \033[35m
CYAN := \033[36m
WHITE := \033[37m
BOLD := \033[1m

EMOJI_STAGE := 🚀
EMOJI_DONE := 🎉
EMOJI_COMPILING := 🔨
EMOJI_PROGRESS := 🟩
EMOJI_REMAINING := ⬜

$(NAME): $(OBJ)
	@printf "$(BOLD)$(BLUE)$(EMOJI_STAGE) Starting compilation of $(NAME)...$(RESET)\n"
	@c++ $(CFLAGS) $(OBJ) $(LIBS) $(HEADERS) -o $(NAME)
	@printf "$(GREEN)$(EMOJI_DONE) Compilation of $(NAME) done!$(RESET)\n"

$(OBJ_DIR)/%.o: ./src/%.cpp $(HFL)
	@mkdir -p $(dir $@)
	@c++ $(CFLAGS) -c $< -o $@ $(HEADERS)
	@CURRENT_FILES=$$(find $(OBJ_DIR) -type f -name '*.o' | wc -l | tr -d ' '); \
	BAR_LENGTH=22; \
	if [ $$CURRENT_FILES -eq $(TOTAL_FILES) ]; then \
		PROGRESS=100; \
		BAR="$$(printf '$(EMOJI_PROGRESS)%.0s' $$(seq 1 $$BAR_LENGTH))"; \
	else \
		PROGRESS=$$((CURRENT_FILES * 100 / $(TOTAL_FILES))); \
		FILLED=$$((BAR_LENGTH * CURRENT_FILES / $(TOTAL_FILES))); \
		EMPTY=$$((BAR_LENGTH - FILLED)); \
		BAR="$$(printf '$(EMOJI_PROGRESS)%.0s' $$(seq 1 $$FILLED))$$(printf '$(EMOJI_REMAINING)%.0s' $$(seq 1 $$EMPTY))"; \
	fi; \
	printf "\r$(YELLOW)%-100s$(RESET)" ""; \
	printf "\r$(YELLOW)[%s] %d%% $(MAGENTA)$(EMOJI_COMPILING) %s $(RESET)" "$$BAR" "$$PROGRESS" "$<"; \
	if [ $$CURRENT_FILES -eq $(TOTAL_FILES) ]; then printf "\n"; fi

all: $(NAME)

clean:
	@printf "$(RED)🧹 Cleaning object files...$(RESET)\n"
	@rm -rf $(OBJ_DIR)

fclean: clean
	@printf "$(RED)🔥 Removing executable...$(RESET)\n"
	@rm -f $(NAME)

re: fclean all

run: all
	@printf "$(GREEN)🏃 Running $(NAME)...$(RESET)\n"
	@./$(NAME) ./config/www.conf

debug: re
	@printf "$(CYAN)🐛 Debugging $(NAME)...$(RESET)\n"
	@$(MAKE) CFLAGS+='-g -O0 -fsanitize=address -DLOG_INCOMING_PACKETS -DLOG_OUTGOING_PACKETS' $(NAME)

parrot:
	@printf "$(BOLD)$(GREEN)🦜 Parrot mode activated!$(RESET)\n"
	@curl parrot.live

rick:
	@printf "$(BOLD)$(GREEN)🦜 Never gonna give you up!$(RESET)\n"
	@curl -s -L https://raw.githubusercontent.com/keroserene/rickrollrc/master/roll.sh | bash

.PHONY: all clean fclean re run debug parrot rick
