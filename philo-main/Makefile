# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/03/27 17:02:00 by iunikel           #+#    #+#              #
#    Updated: 2025/03/31 20:38:48 by iunikel          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = philo

CC = gcc
CFLAGS = -Wall -Wextra -Werror -pthread
DEBUG_FLAGS = -g -O0 -pthread
RM = rm -f

SRCS = 	main.c \
		time_utils.c \
		state_management.c \
		fork_management.c \
		philo_lifecycle.c \
		monitoring.c \
		initialization.c \
		utils.c \
		simulation.c \
		death_check.c \
		fork_strategies.c

OBJS = $(SRCS:.c=.o)
DEBUG_OBJS = $(SRCS:.c=.debug.o)

all: $(NAME)

debug: CFLAGS = $(DEBUG_FLAGS)
debug: $(OBJS)
	$(CC) $(DEBUG_FLAGS) $(OBJS) -o $(NAME)_debug

helgrind: CFLAGS = $(DEBUG_FLAGS)
helgrind: $(OBJS)
	$(CC) $(DEBUG_FLAGS) $(OBJS) -o $(NAME)_helgrind
	valgrind --tool=helgrind ./$(NAME)_helgrind 4 410 200 200

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

clean:
	$(RM) $(OBJS) $(DEBUG_OBJS)

fclean: clean
	$(RM) $(NAME) $(NAME)_debug $(NAME)_helgrind

re: fclean all

.PHONY: all clean fclean re debug helgrind 