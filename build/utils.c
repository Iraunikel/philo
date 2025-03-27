/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 16:35:30 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/27 16:35:30 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/* Time functions */

long	get_time(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

long	get_elapsed_time(t_data *data)
{
	return (get_time() - data->start_time);
}

int	precise_sleep(long ms)
{
	long	start;
	long	elapsed;

	start = get_time();
	while (1)
	{
		elapsed = get_time() - start;
		if (elapsed >= ms)
			return (0);
		usleep(500);
	}
}

/* State management functions */

bool	is_simulation_stopped(t_data *data)
{
	bool	stopped;

	pthread_mutex_lock(&data->data_lock);
	stopped = data->simulation_stop;
	pthread_mutex_unlock(&data->data_lock);
	return (stopped);
}

void	set_simulation_stop(t_data *data, bool value)
{
	pthread_mutex_lock(&data->data_lock);
	data->simulation_stop = value;
	pthread_mutex_unlock(&data->data_lock);
}

int	print_state(t_philo *philo, char *state)
{
	long	timestamp;

	if (is_simulation_stopped(philo->data))
		return (1);
	timestamp = get_elapsed_time(philo->data);
	pthread_mutex_lock(&philo->data->print_lock);
	if (!is_simulation_stopped(philo->data))
		printf("%ld %d %s\n", timestamp, philo->id, state);
	pthread_mutex_unlock(&philo->data->print_lock);
	return (0);
}

/* Fork handling functions */

int	take_forks(t_philo *philo)
{
	int	first_fork;
	int	second_fork;

	if (philo->data->philo_count == 1)
	{
		pthread_mutex_lock(&philo->data->forks[philo->left_fork]);
		print_state(philo, "has taken a fork");
		precise_sleep(philo->data->time_to_die);
		pthread_mutex_unlock(&philo->data->forks[philo->left_fork]);
		return (0);
	}
	if (is_simulation_stopped(philo->data))
		return (0);
	first_fork = (philo->id % 2 == 0) ? philo->left_fork : philo->right_fork;
	second_fork = (philo->id % 2 == 0) ? philo->right_fork : philo->left_fork;
	pthread_mutex_lock(&philo->data->forks[first_fork]);
	print_state(philo, "has taken a fork");
	pthread_mutex_lock(&philo->data->forks[second_fork]);
	print_state(philo, "has taken a fork");
	return (1);
}

void	release_forks(t_philo *philo)
{
	int	first_fork;
	int	second_fork;

	if (philo->data->philo_count == 1)
		return;
	first_fork = (philo->id % 2 == 0) ? philo->left_fork : philo->right_fork;
	second_fork = (philo->id % 2 == 0) ? philo->right_fork : philo->left_fork;
	pthread_mutex_unlock(&philo->data->forks[second_fork]);
	pthread_mutex_unlock(&philo->data->forks[first_fork]);
}

/* Initialization and cleanup functions */

int	init_data(t_data *data)
{
	data->simulation_stop = false;
	data->philosophers = malloc(sizeof(t_philo) * data->philo_count);
	if (!data->philosophers)
		return (1);
	data->forks = malloc(sizeof(pthread_mutex_t) * data->philo_count);
	if (!data->forks)
	{
		free(data->philosophers);
		return (1);
	}
	return (0);
}

int	init_mutexes(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->philo_count)
	{
		if (pthread_mutex_init(&data->forks[i], NULL) != 0)
			return (1);
		i++;
	}
	if (pthread_mutex_init(&data->print_lock, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&data->data_lock, NULL) != 0)
		return (1);
	return (0);
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
		data->philosophers[i].last_meal_time = 0;
		data->philosophers[i].data = data;
		i++;
	}
	return (0);
}

int	cleanup_simulation(t_data *data)
{
	int	i;

	if (data->forks)
	{
		i = 0;
		while (i < data->philo_count)
		{
			pthread_mutex_destroy(&data->forks[i]);
			i++;
		}
		free(data->forks);
	}
	pthread_mutex_destroy(&data->print_lock);
	pthread_mutex_destroy(&data->data_lock);
	if (data->philosophers)
		free(data->philosophers);
	return (0);
}

/* String to integer conversion */

int	ft_atoi(const char *str)
{
	int	i;
	int	sign;
	int	result;

	i = 0;
	sign = 1;
	result = 0;
	while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
		i++;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	while (str[i] >= '0' && str[i] <= '9')
	{
		if (result > INT_MAX / 10 || 
			(result == INT_MAX / 10 && str[i] - '0' > INT_MAX % 10))
			return (sign == 1 ? INT_MAX : INT_MIN);
		result = result * 10 + (str[i] - '0');
		i++;
	}
	return (result * sign);
}
