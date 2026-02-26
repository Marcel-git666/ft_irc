# === Variables ===
NAME        = ircserv
CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98
CPPFLAGS    = -I$(INC_DIR)

# === Directories ===
SRC_DIR     = src
INC_DIR     = inc
OBJ_DIR     = obj

# === Files ===
# Just list the filenames here!
SRC_FILES   =	main.cpp \
				Server.cpp \
				Client.cpp \
				Channel.cpp \
				cmdHandling.cpp \
				errorList.cpp \
				privateMsg.cpp \
				controlChannels.cpp \
				modeChannel.cpp

# addprefix adds "src/" to the start of every word in SRC_FILES
SRCS        = $(addprefix $(SRC_DIR)/, $(SRC_FILES))

# This calculates the object files: src/main.cpp -> obj/main.o
OBJS        = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS        = $(OBJS:.o=.d)

# === Rules ===
all: $(NAME)

$(NAME): $(OBJS)
	@echo "Linking $(NAME)..."
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "Ready!"

# Compile .cpp to .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

clean:
	@rm -rf $(OBJ_DIR)

fclean: clean
	@rm -f $(NAME)

re: fclean all

# Include dependencies (if they exist)
-include $(DEPS)

.PHONY: all clean fclean re
