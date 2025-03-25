/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:09:05 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/24 19:25:10 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

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

int	allocate_resources(t_data *data)
{
	data->simulation_stop = false;
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
		if (pthread_mutex_init(&data->philosophers[i].meal_lock, NULL))
		{
			while (--i >= 0)
				pthread_mutex_destroy(&data->philosophers[i].meal_lock);
			return (0);
		}
		i++;
	}
	return (1);
}

int	init_mutexes(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->philo_count)
	{
		if (pthread_mutex_init(&data->forks[i], NULL))
		{
			while (--i >= 0)
				pthread_mutex_destroy(&data->forks[i]);
			return (0);
		}
		i++;
	}
	if (pthread_mutex_init(&data->print_lock, NULL))
	{
		i = 0;
		while (i < data->philo_count)
			pthread_mutex_destroy(&data->forks[i++]);
		return (0);
	}
	if (pthread_mutex_init(&data->death_lock, NULL) || 
		pthread_mutex_init(&data->global_lock, NULL))
	{
		pthread_mutex_destroy(&data->print_lock);
		i = 0;
		while (i < data->philo_count)
			pthread_mutex_destroy(&data->forks[i++]);
		return (0);
	}
	return (1);
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
