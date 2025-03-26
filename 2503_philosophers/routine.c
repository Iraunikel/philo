/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:08:54 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/26 14:37:06 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** Wait until a specific time has elapsed
*/
int	precise_sleep_until(t_philo *philo, long target_time)
{
	// If simulation has stopped, return immediately
	if (is_simulation_stopped(philo->data))
		return (1);
		
	// Wait until target time using the timekeeper
	wait_until(philo, target_time);
	
	// Check if simulation has stopped while waiting
	if (is_simulation_stopped(philo->data))
		return (1);
		
	return (0);
}

/*
** Philosopher eating action
*/
int	philosopher_eat(t_philo *philo)
{
	long	current_time;
	long	eat_end_time;

	// Skip if simulation has stopped
	if (is_simulation_stopped(philo->data))
		return (1);
		
	// Update philosopher state to hungry
	philo->state = STATE_HUNGRY;
	
	// Try to take forks, return if failed
	if (!take_forks(philo))
		return (1);
		
	// Update philosopher state to eating
	philo->state = STATE_EATING;
	
	// Get current simulation time immediately for precise meal timestamp
	current_time = get_sim_time(philo->data);
	
	// Update last meal time atomically
	pthread_mutex_lock(&philo->data->meal_lock);
	philo->last_meal_time = current_time;
	pthread_mutex_unlock(&philo->data->meal_lock);
	
	// Print eating state
	print_state(philo, "is eating");
	
	// Calculate exact time when eating should end
	eat_end_time = current_time + philo->data->time_to_eat;
	
	// Wait until exact end time using timekeeper for precise timing
	if (precise_sleep_until(philo, eat_end_time))
	{
		release_forks(philo);
		return (1);
	}
	
	// Increment meals eaten atomically
	pthread_mutex_lock(&philo->data->meal_lock);
	philo->meals_eaten++;
	pthread_mutex_unlock(&philo->data->meal_lock);
	
	// Always release forks before returning
	release_forks(philo);
	
	return (0);
}

/*
** Philosopher sleeping action
*/
int	philosopher_sleep(t_philo *philo)
{
	long current_time;
	long end_time;
	
	// Skip if simulation has stopped
	if (is_simulation_stopped(philo->data))
		return (1);
		
	// Update philosopher state to sleeping
	philo->state = STATE_SLEEPING;
	
	// Print sleeping state
	print_state(philo, "is sleeping");
	
	// Get current simulation time
	current_time = get_sim_time(philo->data);
	
	// Calculate when sleeping should end
	end_time = current_time + philo->data->time_to_sleep;
	
	// Wait until end time
	return precise_sleep_until(philo, end_time);
}

/*
** Philosopher thinking action
*/
int	philosopher_think(t_philo *philo)
{
	long current_time;
	long end_time;
	long think_time;
	
	// Skip if simulation has stopped
	if (is_simulation_stopped(philo->data))
		return (1);
		
	// Update philosopher state to thinking
	philo->state = STATE_THINKING;
	
	// Print thinking state
	print_state(philo, "is thinking");
	
	// Get current simulation time
	current_time = get_sim_time(philo->data);
	
	// Calculate a short thinking time (adaptive based on eating/sleeping times)
	think_time = 10;  // Minimum thinking time
	if (philo->data->time_to_die > philo->data->time_to_eat + philo->data->time_to_sleep)
	{
		// If there's time, think for a reasonable amount to avoid CPU spinning
		think_time = (philo->data->time_to_die - philo->data->time_to_eat - philo->data->time_to_sleep) / 2;
		if (think_time > 100)
			think_time = 100;  // Cap thinking time at 100ms
	}
	
	// Calculate when thinking should end
	end_time = current_time + think_time;
	
	// Wait until end time
	return precise_sleep_until(philo, end_time);
}

/*
** Main philosopher routine
*/
void	*philosopher_routine(void *arg)
{
	t_philo *philo;
	int meals_target;
	int current_meals;
	
	philo = (t_philo *)arg;
	meals_target = philo->data->meals_to_eat;
	
	// Initialize philosopher state
	philo->state = STATE_THINKING;
	
	// Set initial meal time
	pthread_mutex_lock(&philo->data->meal_lock);
	philo->last_meal_time = 0;
	pthread_mutex_unlock(&philo->data->meal_lock);
	
	// Print initial thinking state
	print_state(philo, "is thinking");
	
	// Give odd-numbered philosophers a head start to reduce contention
	if (philo->id % 2 != 0)
	{
		if (precise_sleep_until(philo, 20))
			return (NULL);
	}
	
	// Main philosopher loop
	while (!is_simulation_stopped(philo->data))
	{
		// Check if philosopher has eaten enough meals
		if (meals_target != -1)
		{
			pthread_mutex_lock(&philo->data->meal_lock);
			current_meals = philo->meals_eaten;
			pthread_mutex_unlock(&philo->data->meal_lock);
			
			if (current_meals >= meals_target)
			{
				philo->state = STATE_DONE;
				break;
			}
		}
		
		// Eat
		if (philosopher_eat(philo))
			break;
			
		// Sleep
		if (philosopher_sleep(philo))
			break;
			
		// Think
		if (philosopher_think(philo))
			break;
	}
	
	return (NULL);
}
