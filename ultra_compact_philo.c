/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 16:53:20 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/27 16:53:20 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <limits.h>

/* Macros for code reduction */
#define LOCK(x) pthread_mutex_lock(x)
#define UNLOCK(x) pthread_mutex_unlock(x)
#define ELAPSED(d) (get_time() - (d)->start_time)
#define STOPPED(d) (get_sim_state(d))
#define PRINT(p, s) print_state(p, s)
#define SLEEP(t) precise_sleep(t)

/* Data structures */
typedef struct s_philo
{
	int			id;
	int			left_fork;
	int			right_fork;
	int			meals_eaten;
	long		last_meal_time;
	pthread_t	thread;
	struct s_data *data;
}	t_philo;

typedef struct s_data
{
	int			philo_count;
	int			time_to_die;
	int			time_to_eat;
	int			time_to_sleep;
	int			meals_to_eat;
	int			simulation_stop;
	long		start_time;
	pthread_mutex_t *forks;
	pthread_mutex_t	lock;
	t_philo		*philosophers;
	pthread_t	monitor_thread;
}	t_data;

/* Function prototypes */
long	get_time(void);
int		get_sim_state(t_data *data);
void	set_sim_state(t_data *data, int value);
int		print_state(t_philo *philo, char *state);
int		precise_sleep(long ms);
int		handle_forks(t_philo *philo, int take);
int		ft_atoi(const char *str);
int		init_simulation(t_data *data);
int		cleanup_simulation(t_data *data);
void	*philo_routine(void *arg);
void	*monitor_routine(void *arg);

/* Time utility functions */
long	get_time(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

int	precise_sleep(long ms)
{
	long	start;

	start = get_time();
	while (get_time() - start < ms)
		usleep(500);
	return (0);
}

/* State management functions */
int	get_sim_state(t_data *data)
{
	int	state;

	LOCK(&data->lock);
	state = data->simulation_stop;
	UNLOCK(&data->lock);
	return (state);
}

void	set_sim_state(t_data *data, int value)
{
	LOCK(&data->lock);
	data->simulation_stop = value;
	UNLOCK(&data->lock);
}

int	print_state(t_philo *philo, char *state)
{
	long	timestamp;

	if (STOPPED(philo->data))
		return (1);
	timestamp = ELAPSED(philo->data);
	LOCK(&philo->data->lock);
	if (!philo->data->simulation_stop)
		printf("%ld %d %s\n", timestamp, philo->id, state);
	UNLOCK(&philo->data->lock);
	return (0);
}

/* Fork handling function */
int	handle_forks(t_philo *philo, int take)
{
	int	first, second;

	if (philo->data->philo_count == 1 && take)
	{
		LOCK(&philo->data->forks[philo->left_fork]);
		PRINT(philo, "has taken a fork");
		SLEEP(philo->data->time_to_die);
		UNLOCK(&philo->data->forks[philo->left_fork]);
		return (0);
	}
	if (!take)
	{
		if (philo->data->philo_count == 1)
			return (0);
		UNLOCK(&philo->data->forks[philo->right_fork]);
		UNLOCK(&philo->data->forks[philo->left_fork]);
		return (1);
	}
	if (STOPPED(philo->data))
		return (0);
	first = (philo->id % 2) ? philo->right_fork : philo->left_fork;
	second = (philo->id % 2) ? philo->left_fork : philo->right_fork;
	LOCK(&philo->data->forks[first]);
	PRINT(philo, "has taken a fork");
	LOCK(&philo->data->forks[second]);
	PRINT(philo, "has taken a fork");
	return (1);
}

/* Philosopher routine */
void	*philo_routine(void *arg)
{
	t_philo	*philo;
	long	think_time;

	philo = (t_philo *)arg;
	LOCK(&philo->data->lock);
	philo->last_meal_time = 0;
	UNLOCK(&philo->data->lock);
	PRINT(philo, "is thinking");
	if (philo->id % 2)
		usleep(1000);
	while (!STOPPED(philo->data))
	{
		if (!handle_forks(philo, 1))
			break;
		LOCK(&philo->data->lock);
		philo->last_meal_time = ELAPSED(philo->data);
		philo->meals_eaten++;
		UNLOCK(&philo->data->lock);
		PRINT(philo, "is eating");
		SLEEP(philo->data->time_to_eat);
		handle_forks(philo, 0);
		if (STOPPED(philo->data))
			break;
		PRINT(philo, "is sleeping");
		SLEEP(philo->data->time_to_sleep);
		if (STOPPED(philo->data))
			break;
		PRINT(philo, "is thinking");
		if (philo->data->time_to_die > philo->data->time_to_eat + philo->data->time_to_sleep)
		{
			think_time = (philo->data->time_to_die - philo->data->time_to_eat - 
				philo->data->time_to_sleep) / 2;
			if (think_time > 0)
				SLEEP(think_time);
		}
	}
	return (NULL);
}

/* Monitor routine */
void	*monitor_routine(void *arg)
{
	t_data	*data;
	int		i, meals;
	long	current_time, last_meal, time_since_meal;
	int		all_ate_enough;

	data = (t_data *)arg;
	usleep(1000);
	while (!STOPPED(data))
	{
		current_time = ELAPSED(data);
		all_ate_enough = (data->meals_to_eat != -1);
		i = -1;
		while (++i < data->philo_count)
		{
			LOCK(&data->lock);
			last_meal = data->philosophers[i].last_meal_time;
			meals = data->philosophers[i].meals_eaten;
			UNLOCK(&data->lock);
			time_since_meal = (last_meal == 0) ? current_time : current_time - last_meal;
			if (time_since_meal >= data->time_to_die)
			{
				set_sim_state(data, 1);
				LOCK(&data->lock);
				printf("%ld %d died\n", current_time, data->philosophers[i].id);
				UNLOCK(&data->lock);
				return (NULL);
			}
			if (all_ate_enough && meals < data->meals_to_eat)
				all_ate_enough = 0;
		}
		if (all_ate_enough)
		{
			set_sim_state(data, 1);
			return (NULL);
		}
		usleep(500);
	}
	return (NULL);
}

/* Initialization and cleanup */
int	init_simulation(t_data *data)
{
	int	i;

	data->simulation_stop = 0;
	data->philosophers = malloc(sizeof(t_philo) * data->philo_count);
	if (!data->philosophers)
		return (1);
	data->forks = malloc(sizeof(pthread_mutex_t) * data->philo_count);
	if (!data->forks)
	{
		free(data->philosophers);
		return (1);
	}
	if (pthread_mutex_init(&data->lock, NULL) != 0)
	{
		free(data->philosophers);
		free(data->forks);
		return (1);
	}
	i = -1;
	while (++i < data->philo_count)
		if (pthread_mutex_init(&data->forks[i], NULL) != 0)
			return (1);
	i = -1;
	while (++i < data->philo_count)
	{
		data->philosophers[i].id = i + 1;
		data->philosophers[i].left_fork = i;
		data->philosophers[i].right_fork = (i + 1) % data->philo_count;
		data->philosophers[i].meals_eaten = 0;
		data->philosophers[i].last_meal_time = 0;
		data->philosophers[i].data = data;
	}
	return (0);
}

int	cleanup_simulation(t_data *data)
{
	int	i;

	if (data->forks)
	{
		i = -1;
		while (++i < data->philo_count)
			pthread_mutex_destroy(&data->forks[i]);
		free(data->forks);
	}
	pthread_mutex_destroy(&data->lock);
	if (data->philosophers)
		free(data->philosophers);
	return (0);
}

/* String to integer conversion */
int	ft_atoi(const char *str)
{
	int	i, sign, result;

	i = 0;
	sign = 1;
	result = 0;
	while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
		i++;
	if (str[i] == '-' || str[i] == '+')
		sign = (str[i++] == '-') ? -1 : 1;
	while (str[i] >= '0' && str[i] <= '9')
	{
		if (result > INT_MAX / 10 || (result == INT_MAX / 10 && str[i] - '0' > INT_MAX % 10))
			return (sign == 1 ? INT_MAX : INT_MIN);
		result = result * 10 + (str[i++] - '0');
	}
	return (result * sign);
}

/* Main function */
int	main(int argc, char **argv)
{
	t_data	data;
	int		i;

	if (argc < 5 || argc > 6)
	{
		printf("Usage: %s number_of_philosophers time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]\n", argv[0]);
		return (1);
	}
	data.philo_count = ft_atoi(argv[1]);
	data.time_to_die = ft_atoi(argv[2]);
	data.time_to_eat = ft_atoi(argv[3]);
	data.time_to_sleep = ft_atoi(argv[4]);
	data.meals_to_eat = (argc == 6) ? ft_atoi(argv[5]) : -1;
	if (data.philo_count <= 0 || data.philo_count > 200 || data.time_to_die <= 0 || 
		data.time_to_eat <= 0 || data.time_to_sleep <= 0 || (argc == 6 && data.meals_to_eat <= 0))
	{
		printf("Error: Invalid arguments\n");
		return (1);
	}
	if (init_simulation(&data) != 0)
	{
		printf("Error: Failed to initialize simulation\n");
		return (1);
	}
	data.start_time = get_time();
	i = -1;
	while (++i < data.philo_count)
		if (pthread_create(&data.philosophers[i].thread, NULL, philo_routine, &data.philosophers[i]) != 0)
		{
			set_sim_state(&data, 1);
			cleanup_simulation(&data);
			return (1);
		}
	if (pthread_create(&data.monitor_thread, NULL, monitor_routine, &data) != 0)
	{
		set_sim_state(&data, 1);
		cleanup_simulation(&data);
		return (1);
	}
	i = -1;
	while (++i < data.philo_count)
		pthread_join(data.philosophers[i].thread, NULL);
	pthread_join(data.monitor_thread, NULL);
	cleanup_simulation(&data);
	return (0);
}
