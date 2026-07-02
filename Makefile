# ---------------------------------------------------------------------------- #
#                                  ft_ssl_md5                                  #
# ---------------------------------------------------------------------------- #

NAME		:=	ft_ssl

CC			:=	cc
CFLAGS		:=	-Wall -Wextra -Werror

# ----------------------------- Libft (submodule) ---------------------------- #

LIBFT_DIR	:=	libft
LIBFT		:=	$(LIBFT_DIR)/libftfull.a

INCLUDES	:=	-Iincludes -I$(LIBFT_DIR)/include

# ---------------------------------- Sources --------------------------------- #

SRC_DIR		:=	src
OBJ_DIR		:=	build

SRC			:=	$(shell find $(SRC_DIR) -name '*.c')
OBJ			:=	$(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

# ---------------------------------- Regles ---------------------------------- #

all: $(NAME)

$(NAME): $(LIBFT) $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LIBFT) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(LIBFT):
	@git submodule update --init --recursive
	$(MAKE) -C $(LIBFT_DIR)

clean:
	rm -rf $(OBJ_DIR)
	$(MAKE) -C $(LIBFT_DIR) clean

fclean: clean
	rm -rf $(NAME)
	$(MAKE) -C $(LIBFT_DIR) fclean

re: fclean all

.PHONY: all clean fclean re

