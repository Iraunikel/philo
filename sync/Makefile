NAME = philo

CC = gcc
CFLAGS = -Wall -Wextra -Werror -pthread -g

SRC = philo.c \
      init.c \
      scheduler.c \
      routine.c \
      actions.c \
      monitoring.c \
      utils.c

OBJ = $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

test: $(NAME)
	./$(NAME) 5 800 200 200 7

test_helgrind: $(NAME)
	valgrind --tool=helgrind --suppressions=/dev/null --default-suppressions=no --history-level=full -v ./$(NAME) 5 800 200 200 7

test_no_death: $(NAME)
	./$(NAME) 5 800 200 200

test_one_death: $(NAME)
	./$(NAME) 4 310 200 100

test_one_philo: $(NAME)
	./$(NAME) 1 800 200 200

test_two_philos: $(NAME)
	./$(NAME) 2 310 200 100

test_all: test_helgrind test_no_death test_one_death test_one_philo test_two_philos

.PHONY: all clean fclean re test test_helgrind test_no_death test_one_death test_one_philo test_two_philos test_all
