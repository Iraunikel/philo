/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:08:54 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/24 12:45:30 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	parse_args(int argc, char **argv, t_data *data)
{
	if (argc < 5 || argc > 6)
		return (1);
	data->philo_count = ft_atoi(argv[1]);
	data->time_to_die = ft_atoi(argv[2]);
	data->time_to_eat = ft_atoi(argv[3]);
	data->time_to_sleep = ft_atoi(argv[4]);
	if (argc == 6)
		data->meals_to_eat = ft_atoi(argv[5]);
	else
		data->meals_to_eat = -1;
	if (data->philo_count <= 0 || data->time_to_die <= 0
		|| data->time_to_eat <= 0 || data->time_to_sleep <= 0
		|| (argc == 6 && data->meals_to_eat <= 0))
		return (1);
	return (0);
}

int	init_simulation(t_data *data)
{
	if (init_data(data) != 0)
		return (1);
	if (init_mutexes(data) != 0)
		return (1);
	if (init_philosophers(data) != 0)
		return (1);
	return (0);
}

int	create_threads(t_data *data)
{
	if (pthread_create(&data->monitor_thread, NULL, death_monitor, data) != 0)
		return (1);
	if (create_philosopher_threads(data) != 0)
		return (1);
	return (0);
}

int	wait_and_cleanup(t_data *data)
{
	int	i;

	// Wait for all philosopher threads to complete
	i = 0;
	while (i < data->philo_count)
	{
		if (pthread_join(data->philosophers[i].thread, NULL) != 0)
		{
			printf("Error joining philosopher thread %d\n", i + 1);
		}
		i++;
	}

	// Wait for monitor thread to complete
	if (pthread_join(data->monitor_thread, NULL) != 0)
	{
		printf("Error joining monitor thread\n");
	}

	// Clean up resources
	cleanup_simulation(data);
	return (0);
}

int	main(int argc, char **argv)
{
	t_data	data;

	if (parse_args(argc, argv, &data) != 0)
	{
		printf("Error: Invalid arguments\n");
		printf("Usage: %s number_of_philosophers time_to_die time_to_eat "
			"time_to_sleep [number_of_times_each_philosopher_must_eat]\n", 
			argv[0]);
		return (1);
	}
	if (init_simulation(&data) != 0)
	{
		printf("Error: Failed to initialize simulation\n");
		cleanup_simulation(&data);
		return (1);
	}
	if (create_threads(&data) != 0)
	{
		printf("Error: Failed to create threads\n");
		cleanup_simulation(&data);
		return (1);
	}
	wait_and_cleanup(&data);
	return (0);
}
