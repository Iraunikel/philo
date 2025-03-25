/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitoring.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:08:54 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/25 22:23:52 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** Check if a philosopher has died due to starvation
*/
int	check_philosopher_death(t_philo *philo, long current_time)
{
	long	last_meal;
	int		time_to_die;
	long	time_since_last_meal;
	bool	tight_timing;

	// Check if simulation has already been stopped
	if (is_simulation_stopped(philo->data))
		return (0);
		
	time_to_die = philo->data->time_to_die;
	
	// Determine if we're operating with tight timing constraints
	tight_timing = (time_to_die < 3 * (philo->data->time_to_eat + philo->data->time_to_sleep));
	
	// Get last meal time with proper mutex protection
	pthread_mutex_lock(&philo->data->meal_lock);
	last_meal = philo->last_meal_time;
	pthread_mutex_unlock(&philo->data->meal_lock);
	
	// If philosopher hasn't eaten yet (just started), use start time
	if (last_meal == 0)
		last_meal = philo->data->start_time;
	
	// Calculate time since last meal
	time_since_last_meal = current_time - last_meal;
	
	// Add a small grace period for tight timing constraints
	if (tight_timing && time_since_last_meal <= time_to_die + 25)
		return (0);
	
	// If philosopher has exceeded time_to_die, handle death
	if (time_since_last_meal > time_to_die)
	{
		// Double-check to prevent false positives - wait longer for tight timing
		usleep(tight_timing ? 5000 : 1000);
		current_time = get_time();
		
		pthread_mutex_lock(&philo->data->meal_lock);
		last_meal = philo->last_meal_time;
		pthread_mutex_unlock(&philo->data->meal_lock);
		
		// If philosopher hasn't eaten yet, use start time
		if (last_meal == 0)
			last_meal = philo->data->start_time;
		
		// Recalculate time since last meal
		time_since_last_meal = current_time - last_meal;
		
		// Add grace period for tight timing during the recheck
		if (tight_timing && time_since_last_meal <= time_to_die + 25)
			return (0);
		
		// If still over time_to_die, handle death
		if (time_since_last_meal > time_to_die)
			return (handle_death(philo, current_time));
	}
	
	return (0);
}

/*
** Handle philosopher death - update state and print message
*/
int	handle_death(t_philo *philo, long current_time)
{
	// Check if simulation was already stopped
	if (is_simulation_stopped(philo->data))
		return (0);
	
	// Set simulation to stopped state
	set_simulation_stop(philo->data, true);
	
	// Print death message
	pthread_mutex_lock(&philo->data->print_lock);
	printf("%ld %d died\n", 
		current_time - philo->data->start_time, philo->id);
	pthread_mutex_unlock(&philo->data->print_lock);
	
	// Wake up scheduler to notify all philosophers
	pthread_mutex_lock(&philo->data->scheduler->lock);
	pthread_cond_broadcast(&philo->data->scheduler->cond);
	pthread_mutex_unlock(&philo->data->scheduler->lock);
	
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
		
	current_time = get_time();
	
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
	bool	tight_timing;
	bool	philos_ready;

	data = (t_data *)arg;
	
	// Determine if we're operating with tight timing constraints
	tight_timing = (data->time_to_die < 3 * (data->time_to_eat + data->time_to_sleep));
	
	// Wait for all philosophers to be ready
	while (!is_simulation_stopped(data))
	{
		// Check all_philos_ready with proper mutex protection
		pthread_mutex_lock(&data->scheduler->lock);
		philos_ready = data->all_philos_ready;
		pthread_mutex_unlock(&data->scheduler->lock);
		
		if (philos_ready)
			break;
		
		usleep(1000);
	}
	
	// Give philosophers a short time to start eating - longer for tight timing
	usleep(tight_timing ? 20000 : 5000);
	
	// Main monitoring loop
	while (!is_simulation_stopped(data))
	{
		if (monitor_philosophers(data))
			break;
			
		// Sleep to reduce CPU usage but still check frequently enough
		// Use a shorter interval for tight timing
		usleep(tight_timing ? 500 : 1000);
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
		
		// Wake up scheduler to notify all philosophers
		pthread_mutex_lock(&data->scheduler->lock);
		pthread_cond_broadcast(&data->scheduler->cond);
		pthread_mutex_unlock(&data->scheduler->lock);
		
		return (1);
	}
	
	return (0);
}
