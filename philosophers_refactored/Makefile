NAME = philo
CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread
SRCFILES = philo.c init.c routine.c monitoring.c utils.c timekeeper.c fork_utils.c
OBJFILES = $(SRCFILES:.c=.o)

all: $(NAME)

$(NAME): $(OBJFILES)
	$(CC) $(CFLAGS) $(OBJFILES) -o $(NAME)

clean:
	rm -f $(OBJFILES)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
