/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitoring.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/18 11:57:07 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/23 00:12:37 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static void	set_simulation_stop(t_data *data)
{
	pthread_mutex_lock(&data->death_lock);
	data->simulation_stop = 1;
	pthread_mutex_unlock(&data->death_lock);
}

static int	is_simulation_stopped(t_data *data)
{
	int	simulation_stopped;
	
	pthread_mutex_lock(&data->death_lock);
	simulation_stopped = data->simulation_stop;
	pthread_mutex_unlock(&data->death_lock);
	
	return (simulation_stopped);
}

void	*death_monitor(void *arg)
{
	t_data	*data;

	data = (t_data *)arg;
	
	// Brief initial pause to let philosophers get started
	usleep(5000);
	
	while (1)
	{
		// Check if simulation has already stopped
		if (is_simulation_stopped(data))
			break;
			
		// Check if any philosopher has died
		if (check_philosophers(data))
			break;
			
		// Check if all philosophers have eaten enough
		if (check_all_ate_enough(data))
			break;
		
		// Brief sleep to reduce CPU usage
		usleep(1000);
	}
	return (NULL);
}

void	handle_death(t_philo *philo)
{
	long time_since_start;
	
	set_simulation_stop(philo->data);
	
	pthread_mutex_lock(&philo->data->print_lock);
	time_since_start = get_elapsed_time(philo->data);
	printf("%ld %d died\n", time_since_start, philo->id);
	pthread_mutex_unlock(&philo->data->print_lock);
}

int	check_philosophers(t_data *data)
{
	int	i;
	
	if (is_simulation_stopped(data))
		return (0);
		
	i = 0;
	while (i < data->philo_count)
	{	
		if (check_death(&data->philosophers[i]))
		{
			handle_death(&data->philosophers[i]);
			return (1);
		}
		i++;
	}
	return (0);
}

int	check_death(t_philo *philo)
{
	long long	current_time;
	long long	last_meal;
	long long	time_to_die;
	long long	elapsed;
	
	// Get current time
	current_time = get_time();
	
	// Get last meal time
	pthread_mutex_lock(&philo->meal_lock);
	last_meal = philo->last_meal_time;
	pthread_mutex_unlock(&philo->meal_lock);
	
	// Get time to die parameter
	pthread_mutex_lock(&philo->data->death_lock);
	time_to_die = philo->data->time_to_die;
	pthread_mutex_unlock(&philo->data->death_lock);
	
	// Calculate time since last meal
	elapsed = current_time - last_meal;
	
	// Return true if the philosopher has died
	return (elapsed >= time_to_die);
}

int	check_all_ate_enough(t_data *data)
{
	int	i;
	int	required_meals;
	int	all_ate_enough;
	
	// Get required meals and check if we need to track meals
	pthread_mutex_lock(&data->death_lock);
	required_meals = data->meals_to_eat;
	pthread_mutex_unlock(&data->death_lock);
	
	if (required_meals < 0)
		return (0);
	
	// Check each philosopher's meal count
	all_ate_enough = 1;
	i = 0;
	while (i < data->philo_count)
	{
		pthread_mutex_lock(&data->philosophers[i].meal_lock);
		if (data->philosophers[i].meals_eaten < required_meals)
		{
			all_ate_enough = 0;
			pthread_mutex_unlock(&data->philosophers[i].meal_lock);
			break;
		}
		pthread_mutex_unlock(&data->philosophers[i].meal_lock);
		i++;
	}
	
	// All philosophers have eaten enough
	if (all_ate_enough)
	{
		// Before confirming, do one final thorough check of all philosophers
		all_ate_enough = 1;
		i = 0;
		while (i < data->philo_count)
		{
			pthread_mutex_lock(&data->philosophers[i].meal_lock);
			if (data->philosophers[i].meals_eaten < required_meals)
			{
				all_ate_enough = 0;
				pthread_mutex_unlock(&data->philosophers[i].meal_lock);
				break;
			}
			pthread_mutex_unlock(&data->philosophers[i].meal_lock);
			i++;
		}
		
		if (!all_ate_enough)
			return (0);
		
		// Print final meal counts
		pthread_mutex_lock(&data->print_lock);
		i = 0;
		while (i < data->philo_count)
		{
			pthread_mutex_lock(&data->philosophers[i].meal_lock);
			printf("Philosopher %d ate %d meals\n", 
				data->philosophers[i].id, data->philosophers[i].meals_eaten);
			pthread_mutex_unlock(&data->philosophers[i].meal_lock);
			i++;
		}
		printf("All philosophers have eaten enough meals\n");
		pthread_mutex_unlock(&data->print_lock);
		
		set_simulation_stop(data);
		return (1);
	}
	return (0);
}
