NAME          = prog
INCLUDES      = -I./includes
SRC_DIR       = src/
OBJ_DIR       = obj/

CXX           = c++
CXXFLAGS      = -Wall -Wextra -Werror -std=c++98 $(INCLUDES)

SRC_FILES     = main \
				Server \
				Channel \
				Client \
				Commands\
				Join\
				Kick\
				Mode\
				Invite\
				Topic\


SRC           = $(addprefix $(SRC_DIR), $(addsuffix .cpp, $(SRC_FILES)))
OBJ           = $(addprefix $(OBJ_DIR), $(addsuffix .o, $(SRC_FILES)))

OBJ_CACHE     = .cache_exists

#####

all:			$(NAME)

$(NAME):		$(OBJ)
					$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
					@echo "$(NAME) compiled!"

$(OBJ_DIR)%.o : $(SRC_DIR)%.cpp | $(OBJ_CACHE)
					@echo "Compiling $<"
					@$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_CACHE):
					@mkdir -p $(OBJ_DIR)

clean:
					@rm -rf $(OBJ_DIR)
					@echo "$(NAME) object files cleaned!"

fclean:			clean
					@rm -f $(NAME)
					@echo "$(NAME) executable files cleaned!"

re:				fclean all
					@echo "Cleaned and rebuilt everything for $(NAME)!"

.PHONY:			all clean fclean re
