#include "philo.h"


int	ft_atoi(char const *str)
{
	int	nbr;
	int	i;

	if (!str)
		return (0);
	nbr = 0;
	i = 0;
	while (str[i] >= '0' && str[i] <= '9')
	{
		nbr = nbr * 10 + (str[i] - '0');
		i++;
	}
	return (nbr);
}

void	print_state(t_philo *philo, char *message)
{
	pthread_mutex_lock(&philo->data->print_lock);
	if (!philo->data->simulation_stop)
		printf("%lld %d %s\n", get_time() - philo->data->start_time, philo->id,
			message);
	pthread_mutex_unlock(&philo->data->print_lock);
}

int	take_forks(t_philo *philo)
{
	// Special case for single philosopher
	if (philo->data->philo_count == 1)
	{
		pthread_mutex_lock(&philo->forks[philo->left_fork]);
		print_state(philo, "has taken a fork");
		precise_sleep(philo->data->time_to_die + 10); 
		pthread_mutex_unlock(&philo->forks[philo->left_fork]);
		return (0);
	}
	pthread_mutex_lock(&philo->forks[philo->left_fork]);
	print_state(philo, "has taken a fork");
	pthread_mutex_lock(&philo->forks[philo->right_fork]);
	print_state(philo, "has taken a fork");
	return (1);
}

void	release_forks(t_philo *philo)
{
	pthread_mutex_unlock(&philo->forks[philo->left_fork]);
	pthread_mutex_unlock(&philo->forks[philo->right_fork]);
}

int	check_death(t_philo *philo)
{
	long long	current_time;

	pthread_mutex_lock(&philo->data->death_lock);
	current_time = get_time();
	if ((current_time - philo->last_meal_time) >= philo->data->time_to_die)
	{
		pthread_mutex_unlock(&philo->data->death_lock);
		return (1);
	}
	pthread_mutex_unlock(&philo->data->death_lock);
	return (0);
}

static int	philosopher_eat(t_philo *philo)
{
	if (!take_forks(philo))
		return (0);
	pthread_mutex_lock(&philo->data->death_lock);
	philo->last_meal_time = get_time();
	philo->meals_eaten++;
	pthread_mutex_unlock(&philo->data->death_lock);
	print_state(philo, "is eating");
	precise_sleep(philo->data->time_to_eat);
	release_forks(philo);
	return (1);
}

void	*philosopher_routine(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	pthread_mutex_lock(&philo->data->death_lock);
	philo->last_meal_time = get_time();
	pthread_mutex_unlock(&philo->data->death_lock);
	if (philo->id % 2)
		precise_sleep(2);
	while (!philo->data->simulation_stop)
	{
		print_state(philo, "is thinking");
		if (!philosopher_eat(philo))
			break ;
		print_state(philo, "is sleeping");
		precise_sleep(philo->data->time_to_sleep);
		if (philo->data->meals_to_eat != -1
			&& philo->meals_eaten >= philo->data->meals_to_eat)
			break ;
	}
	return (NULL);
}

long	get_time(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void	precise_sleep(int ms)
{
	long	start;

	start = get_time();
	while ((get_time() - start) < ms)
		usleep(500);
}

int	init_mutexes(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->philo_count)
	{
		if (pthread_mutex_init(&data->forks[i], NULL))
			return (0);
		i++;
	}
	if (pthread_mutex_init(&data->print_lock, NULL))
		return (0);
	if (pthread_mutex_init(&data->death_lock, NULL))
		return (0);
	return (1);
}

int	init_philosophers(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->philo_count)
	{
		data->philosophers[i].id = i + 1;
		data->philosophers[i].left_fork = i;
		data->philosophers[i].right_fork = (i + 1) % data->philo_count;
		data->philosophers[i].meals_eaten = 0;
		data->philosophers[i].data = data;
		data->philosophers[i].forks = data->forks;
		data->philosophers[i].last_meal_time = data->start_time;
		i++;
	}
	return (1);
}

void	cleanup_simulation(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->philo_count)
	{
		pthread_join(data->philosophers[i].thread, NULL);
		pthread_mutex_destroy(&data->forks[i]);
		i++;
	}
	pthread_mutex_destroy(&data->print_lock);
	pthread_mutex_destroy(&data->death_lock);
	free(data->philosophers);
	free(data->forks);
}

void	handle_death(t_data *data, int i)
{
	pthread_mutex_lock(&data->print_lock);
	data->simulation_stop = 1;
	printf("%lld %d died\n", get_time() - data->start_time,
		data->philosophers[i].id);
	pthread_mutex_unlock(&data->print_lock);
}

int	check_philosophers(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->philo_count && !data->simulation_stop)
	{
		if (check_death(&data->philosophers[i]))
		{
			handle_death(data, i);
			return (1);
		}
		i++;
	}
	return (0);
}

void	*death_monitor(void *arg)
{
	t_data	*data;

	data = (t_data *)arg;
	while (!data->simulation_stop)
	{
		if (check_philosophers(data))
			break ;
		if (check_all_ate_enough(data))
		{
			data->simulation_stop = 1;
			break ;
		}
		usleep(100);
	}
	return (NULL);
}

int	check_all_ate_enough(t_data *data)
{
	int	i;
	int	all_ate;

	if (data->meals_to_eat == -1)
		return (0);
	i = 0;
	all_ate = 1;
	while (i < data->philo_count)
	{
		if (data->philosophers[i].meals_eaten < data->meals_to_eat)
		{
			all_ate = 0;
			break ;
		}
		i++;
	}
	return (all_ate);
}

int	init_data(t_data *data, int argc, char **argv)
{
	data->philo_count = ft_atoi(argv[1]);
	data->time_to_die = ft_atoi(argv[2]);
	data->time_to_eat = ft_atoi(argv[3]);
	data->time_to_sleep = ft_atoi(argv[4]);
	if (data->philo_count > 200)
	{
		printf("Error: Maximum number of philosophers is 200\n");
		return (0);
	}
	if (data->time_to_die < 60 || data->time_to_eat < 60
		|| data->time_to_sleep < 60)
	{
		printf("Error: Time values must be at least 60ms\n");
		return (0);
	}
	if (data->philo_count < 1)
	{
		printf(ERR_INPUT "\n");
		return (0);
	}
	return (init_meals(data, argc, argv));
}

int	init_meals(t_data *data, int argc, char **argv)
{
	data->meals_to_eat = -1;
	if (argc == 6)
	{
		data->meals_to_eat = ft_atoi(argv[5]);
		if (data->meals_to_eat <= 0)
		{
			printf(ERR_INPUT "\n");
			return (0);
		}
	}
	return (1);
}

int	allocate_resources(t_data *data)
{
	data->simulation_stop = 0;
	data->start_time = get_time();
	data->philosophers = malloc(sizeof(t_philo) * data->philo_count);
	if (!data->philosophers)
	{
		printf(ERR_MALLOC "\n");
		return (0);
	}
	data->forks = malloc(sizeof(pthread_mutex_t) * data->philo_count);
	if (!data->forks)
	{
		free(data->philosophers);
		printf(ERR_MALLOC "\n");
		return (0);
	}
	return (1);
}

int	create_threads(t_data *data, pthread_t *monitor)
{
	int	i;

	i = 0;
	while (i < data->philo_count)
	{
		if (pthread_create(&data->philosophers[i].thread, NULL,
				philosopher_routine, &data->philosophers[i]))
		{
			data->simulation_stop = 1;
			printf(ERR_THREAD "\n");
			cleanup_simulation(data);
			return (0);
		}
		i++;
	}
	if (pthread_create(monitor, NULL, death_monitor, data))
	{
		data->simulation_stop = 1;
		printf(ERR_THREAD "\n");
		cleanup_simulation(data);
		return (0);
	}
	return (1);
}

int	main(int argc, char **argv)
{
	t_data		data;
	pthread_t	monitor;

	if (argc != 5 && argc != 6)
	{
		printf(ERR_ARGS "\n");
		return (1);
	}
	if (!init_data(&data, argc, argv))
		return (1);
	if (!allocate_resources(&data))
		return (1);
	if (!init_mutexes(&data) || !init_philosophers(&data))
	{
		free(data.philosophers);
		free(data.forks);
		printf(ERR_MUTEX "\n");
		return (1);
	}
	if (!create_threads(&data, &monitor))
		return (1);
	pthread_join(monitor, NULL);
	data.simulation_stop = 1;
	cleanup_simulation(&data);
	return (0);
}
