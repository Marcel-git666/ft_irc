# === Variables ===
NAME        = ircserv
BOT_NAME    = ircbot
CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98
CPPFLAGS    = -I$(INC_DIR)

# === Directories ===
SRC_DIR     = src
INC_DIR     = inc
OBJ_DIR     = obj
BOT_DIR     = bot

# === Files ===
# Just list the filenames here!
SRC_FILES   =	main.cpp \
				Server.cpp \
				Client.cpp \
				Channel.cpp \
				cmdHandling.cpp \
				errorList.cpp \
				privateMsg.cpp \
				controlChannels.cpp

# addprefix adds "src/" to the start of every word in SRC_FILES
SRCS        = $(addprefix $(SRC_DIR)/, $(SRC_FILES))

# This calculates the object files: src/main.cpp -> obj/main.o
OBJS        = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# === Files (Bot) ===
BOT_FILES   = main.cpp Bot.cpp
BOT_SRCS    = $(addprefix $(BOT_DIR)/, $(BOT_FILES))
# Bot objects will be inside obj/bot/
BOT_OBJS    = $(patsubst $(BOT_DIR)/%.cpp,$(OBJ_DIR)/bot/%.o,$(BOT_SRCS))

DEPS        = $(OBJS:.o=.d) $(BOT_OBJS:.o=.d)

# === Rules ===
all: $(NAME)

$(NAME): $(OBJS)
	@echo "Linking $(NAME)..."
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "Ready!"

# --- BONUS RULE (Bot compilation) ---
bonus: $(BOT_NAME)

$(BOT_NAME): $(BOT_OBJS)
		@echo "Linking $(BOT_NAME)..."
		@$(CXX) $(CXXFLAGS) $(BOT_OBJS) -o $(BOT_NAME)
		@echo "Bot $(BOT_NAME) Ready!"

# Compile .cpp to .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

# Compile Bot .cpp to .o
$(OBJ_DIR)/bot/%.o: $(BOT_DIR)/%.cpp
		@mkdir -p $(dir $@)
		@echo "Compiling $<..."
		@$(CXX) $(CXXFLAGS) -I$(BOT_DIR) -MMD -MP -c $< -o $@

clean:
	@rm -rf $(OBJ_DIR)

fclean: clean
	@rm -f $(NAME) $(BOT_NAME)

re: fclean all

# Include dependencies (if they exist)
-include $(DEPS)

.PHONY: all clean fclean re bonus
