/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/18 12:03:39 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/22 21:33:57 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	create_threads(t_data *data, pthread_t *monitor)
{
	int	i;

	i = 0;
	while (i < data->philo_count)
	{
		if (pthread_create(&data->philosophers[i].thread, NULL,
				philosopher_routine, &data->philosophers[i]))
		{
			pthread_mutex_lock(&data->death_lock);
			data->simulation_stop = 1;
			pthread_mutex_unlock(&data->death_lock);
			
			printf(ERR_THREAD "\n");
			cleanup_simulation(data);
			return (0);
		}
		i++;
	}
	if (pthread_create(monitor, NULL, death_monitor, data))
	{
		pthread_mutex_lock(&data->death_lock);
		data->simulation_stop = 1;
		pthread_mutex_unlock(&data->death_lock);
		
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
	
	// Protect setting simulation_stop with the mutex
	pthread_mutex_lock(&data.death_lock);
	data.simulation_stop = 1;
	pthread_mutex_unlock(&data.death_lock);
	
	cleanup_simulation(&data);
	return (0);
}
