NAME = philo

CC = gcc
CFLAGS = -Wall -Wextra -Werror


all: $(NAME)

$(NAME):
	$(CC) $(CFLAGS) -o $(NAME) philo.c

clean:
	rm -f $(NAME)
