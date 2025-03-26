/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:08:54 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/26 14:12:25 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** Parse command line arguments
** Usage: ./philo number_of_philosophers time_to_die time_to_eat time_to_sleep
**        [number_of_times_each_philosopher_must_eat]
*/
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
	
	// Validate arguments
	if (data->philo_count <= 0 || data->philo_count > 200 ||
		data->time_to_die <= 0 || data->time_to_eat <= 0 ||
		data->time_to_sleep <= 0 || (argc == 6 && data->meals_to_eat <= 0))
		return (1);
		
	return (0);
}

/*
** Initialize the philosophers simulation
*/
int	init_simulation(t_data *data)
{
	// Initialize data structures and mutexes
	if (init_data(data) != 0)
		return (1);
	if (init_mutexes(data) != 0)
	{
		cleanup_simulation(data);
		return (1);
	}
	if (init_philosophers(data) != 0)
	{
		cleanup_simulation(data);
		return (1);
	}
	
	return (0);
}

/*
** Create philosopher threads and death monitor
*/
int	create_threads(t_data *data)
{
	if (create_philosopher_threads(data) != 0)
	{
		set_simulation_stop(data, true);
		cleanup_simulation(data);
		return (1);
	}
	
	return (0);
}

/*
** Wait for threads to finish and clean up resources
*/
int	wait_and_cleanup(t_data *data)
{
	int	i;

	// Join philosopher threads
	i = 0;
	while (i < data->philo_count)
	{
		pthread_join(data->philosophers[i].thread, NULL);
		i++;
	}
	
	// Join monitoring thread
	pthread_join(data->monitor_thread, NULL);
	
	// Join timekeeper thread
	pthread_join(data->timekeeper->thread, NULL);
	
	// Clean up resources
	cleanup_simulation(data);
	
	return (0);
}

/*
** Main function for the philosophers program
*/
int	main(int argc, char **argv)
{
	t_data	data;

	// Parse command line arguments
	if (parse_args(argc, argv, &data) != 0)
	{
		printf("Error: Invalid arguments\n");
		printf("Usage: %s number_of_philosophers time_to_die time_to_eat ", 
			argv[0]);
		printf("time_to_sleep [number_of_times_each_philosopher_must_eat]\n");
		return (1);
	}
	
	// Initialize simulation
	if (init_simulation(&data) != 0)
	{
		printf("Error: Failed to initialize simulation\n");
		return (1);
	}
	
	// Create threads
	if (create_threads(&data) != 0)
	{
		printf("Error: Failed to create threads\n");
		return (1);
	}
	
	// Wait for threads to finish and clean up
	wait_and_cleanup(&data);
	
	return (0);
}
