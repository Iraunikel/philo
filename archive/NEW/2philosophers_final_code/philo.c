/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/18 12:03:39 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/24 11:44:30 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static int	init_simulation_state(t_data *data)
{
	pthread_mutex_lock(&data->death_lock);
	data->simulation_stop = 0;
	pthread_mutex_unlock(&data->death_lock);
	return (1);
}

static int	handle_thread_error(t_data *data)
{
	pthread_mutex_lock(&data->death_lock);
	data->simulation_stop = 1;
	pthread_mutex_unlock(&data->death_lock);
	printf(ERR_THREAD "\n");
	cleanup_simulation(data);
	return (0);
}

static int	create_philosopher_threads(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->philo_count)
	{
		if (pthread_create(&data->philosophers[i].thread, NULL,
				philosopher_routine, &data->philosophers[i]))
			return (handle_thread_error(data));
		i++;
	}
	return (1);
}

int	create_threads(t_data *data, pthread_t *monitor)
{
	init_simulation_state(data);
	if (!create_philosopher_threads(data))
		return (0);
	if (pthread_create(monitor, NULL, death_monitor, data))
		return (handle_thread_error(data));
	return (1);
}

static int	handle_init_error(t_data *data, int error_type)
{
	if (error_type == 1)
	{
		free(data->philosophers);
		free(data->forks);
		printf(ERR_MUTEX "\n");
	}
	return (1);
}

static int	initialize_simulation(t_data *data, int argc, char **argv)
{
	if (!init_data(data, argc, argv))
		return (1);
	if (!allocate_resources(data))
		return (1);
	if (!init_mutexes(data) || !init_philosophers(data))
		return (handle_init_error(data, 1));
	return (0);
}

static void	wait_and_cleanup(t_data *data, pthread_t monitor)
{
	pthread_join(monitor, NULL);
	pthread_mutex_lock(&data->death_lock);
	data->simulation_stop = 1;
	pthread_mutex_unlock(&data->death_lock);
	cleanup_simulation(data);
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
	if (initialize_simulation(&data, argc, argv))
		return (1);
	if (!create_threads(&data, &monitor))
		return (1);
	wait_and_cleanup(&data, monitor);
	return (0);
}
