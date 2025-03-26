/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitoring.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:08:54 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/26 14:37:06 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** Check if a philosopher has died due to starvation
*/
int	check_philosopher_death(t_philo *philo, long current_time)
{
	long	last_meal;
	long	time_since_last_meal;
	int		time_to_die;

	// Check if simulation has already been stopped
	if (is_simulation_stopped(philo->data))
		return (0);
		
	time_to_die = philo->data->time_to_die;
	
	// Get last meal time with proper mutex protection
	pthread_mutex_lock(&philo->data->meal_lock);
	last_meal = philo->last_meal_time;
	pthread_mutex_unlock(&philo->data->meal_lock);
	
	// Calculate time since last meal with zero tolerance
	if (last_meal == 0)
	{
		// Philosopher hasn't eaten yet, use simulation time directly
		time_since_last_meal = current_time;
	}
	else
	{
		time_since_last_meal = current_time - last_meal;
	}
	
	// Exact timing check - don't allow even 1ms of delay
	// Handle death exactly when time_since_last_meal equals time_to_die
	if (time_since_last_meal >= time_to_die)
	{
		// Prioritize this death check over other operations
		return (handle_death(philo, time_to_die));
	}
	
	return (0);
}

/*
** Handle philosopher death - update state and print message
*/
int	handle_death(t_philo *philo, long death_time)
{
	// Check if simulation was already stopped
	if (is_simulation_stopped(philo->data))
		return (0);
	
	// Set simulation to stopped state immediately
	set_simulation_stop(philo->data, true);
	
	// Print death message with the exact death time
	pthread_mutex_lock(&philo->data->print_lock);
	printf("%ld %d died\n", death_time, philo->id);
	pthread_mutex_unlock(&philo->data->print_lock);
	
	// Signal all waiting threads to wake up immediately
	pthread_mutex_lock(&philo->data->timekeeper->lock);
	pthread_cond_broadcast(&philo->data->timekeeper->cond);
	pthread_mutex_unlock(&philo->data->timekeeper->lock);
	
	return (1);
}

/*
** Monitor all philosophers for death conditions
*/
static int	monitor_philosophers(t_data *data)
{
	int		i;
	long	current_time;

	// Skip if simulation has already been stopped
	if (is_simulation_stopped(data))
		return (1);
		
	current_time = get_sim_time(data);
	
	// Check each philosopher for death
	i = 0;
	while (i < data->philo_count)
	{
		if (check_philosopher_death(&data->philosophers[i], current_time))
			return (1);
		i++;
	}
	
	// Check if all philosophers have eaten enough
	if (check_all_ate_enough(data))
		return (1);
		
	return (0);
}

/*
** Death monitor thread that periodically checks all philosophers
*/
void	*death_monitor(void *arg)
{
	t_data	*data;
	struct timespec ts;

	data = (t_data *)arg;
	
	// Very minimal startup delay
	usleep(200);
	
	// Configure nanosleep parameters
	ts.tv_sec = 0;
	ts.tv_nsec = 500000; // 0.5ms
	
	// Main monitoring loop with high-frequency checks
	while (!is_simulation_stopped(data))
	{
		// Check philosophers' status with high priority
		if (monitor_philosophers(data))
			break;
			
		// Ultra-precise sleep to maintain timing accuracy
		nanosleep(&ts, NULL);
	}
	
	return (NULL);
}

/*
** Check if all philosophers have eaten enough meals
*/
int	check_all_ate_enough(t_data *data)
{
	int		i;
	int		meals;
	bool	all_ate_enough;

	// Skip if no meal limit set
	if (data->meals_to_eat == -1)
		return (0);
	
	// Skip if simulation has already been stopped
	if (is_simulation_stopped(data))
		return (0);
	
	// Check if all philosophers have eaten enough
	all_ate_enough = true;
	i = 0;
	while (i < data->philo_count)
	{
		pthread_mutex_lock(&data->meal_lock);
		meals = data->philosophers[i].meals_eaten;
		pthread_mutex_unlock(&data->meal_lock);
		
		if (meals < data->meals_to_eat)
		{
			all_ate_enough = false;
			break;
		}
		i++;
	}
	
	// All philosophers have eaten enough
	if (all_ate_enough)
	{
		set_simulation_stop(data, true);
		
		// Wake up any waiting philosophers
		pthread_mutex_lock(&data->timekeeper->lock);
		pthread_cond_broadcast(&data->timekeeper->cond);
		pthread_mutex_unlock(&data->timekeeper->lock);
		
		return (1);
	}
	
	return (0);
}
