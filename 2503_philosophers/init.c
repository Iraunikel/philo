/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:08:54 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/25 22:19:00 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** Initialize data structure with default values
*/
int	init_data(t_data *data)
{
	data->philosophers = NULL;
	data->forks = NULL;
	data->start_time = 0;
	data->simulation_stop = false;
	data->all_philos_ready = false;
	data->scheduler = NULL;
	return (0);
}

/*
** Initialize mutexes used for synchronization
*/
int	init_mutexes(t_data *data)
{
	int	i;

	// Initialize fork mutexes
	data->forks = malloc(sizeof(pthread_mutex_t) * data->philo_count);
	if (!data->forks)
		return (1);
	
	i = 0;
	while (i < data->philo_count)
	{
		if (pthread_mutex_init(&data->forks[i], NULL) != 0)
			return (1);
		i++;
	}
	
	// Initialize other synchronization mutexes
	if (pthread_mutex_init(&data->print_lock, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&data->death_lock, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&data->meal_lock, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&data->stop_lock, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&data->global_lock, NULL) != 0)
		return (1);
	
	// Initialize scheduler
	if (init_scheduler(data) != 0)
		return (1);
		
	return (0);
}

/*
** Initialize philosopher data structures
*/
int	init_philosophers(t_data *data)
{
	int	i;

	data->philosophers = malloc(sizeof(t_philo) * data->philo_count);
	if (!data->philosophers)
		return (1);
	
	i = 0;
	while (i < data->philo_count)
	{
		data->philosophers[i].id = i + 1;
		data->philosophers[i].left_fork = i;
		data->philosophers[i].right_fork = (i + 1) % data->philo_count;
		data->philosophers[i].meals_eaten = 0;
		data->philosophers[i].last_meal_time = 0;
		data->philosophers[i].state = STATE_THINKING;
		data->philosophers[i].data = data;
		i++;
	}
	
	return (0);
}

/*
** Create philosopher threads and the scheduler thread
*/
int	create_philosopher_threads(t_data *data)
{
	int	i;

	// Record start time for all philosophers
	data->start_time = get_time();
	
	// Create scheduler thread first
	if (pthread_create(&data->scheduler->thread, NULL, scheduler_routine, data) != 0)
		return (1);
	
	// Small delay to allow scheduler to initialize
	usleep(1000);
	
	// Create philosopher threads
	i = 0;
	while (i < data->philo_count)
	{
		if (pthread_create(&data->philosophers[i].thread, NULL, 
				philosopher_routine, &data->philosophers[i]) != 0)
			return (1);
		
		// Small delay between thread creation for better distribution
		usleep(100);
		i++;
	}
	
	// Create death monitoring thread
	if (pthread_create(&data->monitor_thread, NULL, death_monitor, data) != 0)
		return (1);
		
	return (0);
}

/*
** Clean up resources used by the simulation
*/
int	cleanup_simulation(t_data *data)
{
	int	i;

	// Destroy fork mutexes
	i = 0;
	if (data->forks)
	{
		while (i < data->philo_count)
		{
			pthread_mutex_destroy(&data->forks[i]);
			i++;
		}
		free(data->forks);
	}
	
	// Destroy other mutexes
	pthread_mutex_destroy(&data->print_lock);
	pthread_mutex_destroy(&data->death_lock);
	pthread_mutex_destroy(&data->meal_lock);
	pthread_mutex_destroy(&data->stop_lock);
	pthread_mutex_destroy(&data->global_lock);
	
	// Clean up scheduler
	if (data->scheduler)
	{
		pthread_mutex_destroy(&data->scheduler->lock);
		pthread_cond_destroy(&data->scheduler->cond);
		free(data->scheduler);
	}
	
	// Free philosopher array
	if (data->philosophers)
		free(data->philosophers);
		
	return (0);
}
