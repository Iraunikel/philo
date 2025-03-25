/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:08:54 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/25 22:51:24 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** Precise sleep function that ensures accurate timing
*/
void	precise_sleep(int ms)
{
	long	start;
	long	current;
	long	elapsed;

	start = get_time();
	while (1)
	{
		current = get_time();
		elapsed = current - start;
		if (elapsed >= ms)
			break;
		if (ms - elapsed > 10)
			usleep(500);
		else
			usleep(100);
	}
}

/*
** Philosopher eating action
*/
int	philosopher_eat(t_philo *philo)
{
	long	current_time;

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
	
	// Get current time for meal timestamp
	current_time = get_time();
	
	// Print eating state and update last meal time IMMEDIATELY
	print_state(philo, "is eating");
	
	// Update last meal time with proper mutex protection
	pthread_mutex_lock(&philo->data->meal_lock);
	philo->last_meal_time = current_time;
	pthread_mutex_unlock(&philo->data->meal_lock);
	
	// Sleep for time_to_eat duration
	precise_sleep(philo->data->time_to_eat);
	
	// Check if simulation stopped during eating
	if (is_simulation_stopped(philo->data))
	{
		release_forks(philo);
		return (1);
	}
	
	// Increment meals eaten with proper mutex protection
	pthread_mutex_lock(&philo->data->meal_lock);
	philo->meals_eaten++;
	pthread_mutex_unlock(&philo->data->meal_lock);
	
	// Release forks
	release_forks(philo);
	
	return (0);
}

/*
** Philosopher sleeping action
*/
int	philosopher_sleep(t_philo *philo)
{
	// Skip if simulation has stopped
	if (is_simulation_stopped(philo->data))
		return (1);
		
	// Update philosopher state to sleeping
	philo->state = STATE_SLEEPING;
	
	// Print sleeping state
	print_state(philo, "is sleeping");
	
	// Sleep for time_to_sleep duration
	precise_sleep(philo->data->time_to_sleep);
	
	return (0);
}

/*
** Philosopher thinking action
*/
int	philosopher_think(t_philo *philo)
{
	// Skip if simulation has stopped
	if (is_simulation_stopped(philo->data))
		return (1);
		
	// Update philosopher state to thinking
	philo->state = STATE_THINKING;
	
	// Print thinking state
	print_state(philo, "is thinking");
	
	// Think for a very short time - prioritize getting back to eating
	usleep(500);
	
	return (0);
}

/*
** Main philosopher routine that coordinates with the scheduler
*/
void	*philosopher_routine(void *arg)
{
	t_philo *philo;
	int meals_target;
	int current_meals;
	long current_time;

	philo = (t_philo *)arg;
	meals_target = philo->data->meals_to_eat;
	
	// Initialize philosopher state
	philo->state = STATE_THINKING;
	
	// Signal to scheduler that this philosopher is ready
	pthread_mutex_lock(&philo->data->scheduler->lock);
	philo->data->scheduler->ready_philos++;
	pthread_mutex_unlock(&philo->data->scheduler->lock);
	
	// Initial thinking state - set initial meal time
	current_time = get_time();
	pthread_mutex_lock(&philo->data->meal_lock);
	philo->last_meal_time = current_time;
	pthread_mutex_unlock(&philo->data->meal_lock);
	print_state(philo, "is thinking");
	
	// Main philosopher loop
	while (!is_simulation_stopped(philo->data))
	{
		// Wait for scheduler permission to act
		if (wait_for_turn(philo) == ACTION_SIMULATION_STOPPED)
			break;
		
		// Check if philosopher has eaten enough meals
		if (meals_target != -1)
		{
			pthread_mutex_lock(&philo->data->meal_lock);
			current_meals = philo->meals_eaten;
			pthread_mutex_unlock(&philo->data->meal_lock);
			
			if (current_meals >= meals_target)
			{
				philo->state = STATE_DONE;
				signal_action_complete(philo);
				break;
			}
		}
		
		// Perform philosopher actions in sequence, prioritizing eating
		if (philosopher_eat(philo))
		{
			signal_action_complete(philo);
			break;
		}
		
		if (philosopher_sleep(philo))
		{
			signal_action_complete(philo);
			break;
		}
		
		if (philosopher_think(philo))
		{
			signal_action_complete(philo);
			break;
		}
		
		// Signal to scheduler that this cycle is complete
		signal_action_complete(philo);
	}
	
	return (NULL);
}
