/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 20:23:30 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/25 22:50:46 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/* 
** Initialize the scheduler component
*/
int	init_scheduler(t_data *data)
{
	data->scheduler = malloc(sizeof(t_scheduler));
	if (!data->scheduler)
		return (1);
	pthread_mutex_init(&data->scheduler->lock, NULL);
	pthread_cond_init(&data->scheduler->cond, NULL);
	data->scheduler->ready_philos = 0;
	data->scheduler->active_philos = data->philo_count;
	data->scheduler->cycle_complete = false;
	data->scheduler->last_check_time = 0;
	return (0);
}

/*
** The scheduler is the central control mechanism that ensures deterministic
** execution of philosopher actions. It coordinates all philosophers to prevent
** race conditions and ensure absolute stability.
*/
void	*scheduler_routine(void *arg)
{
	t_data *data;
	long current_time;

	data = (t_data *)arg;
	
	// Wait for all philosophers to initialize
	while (1)
	{
		pthread_mutex_lock(&data->scheduler->lock);
		if (data->scheduler->ready_philos == data->philo_count)
		{
			// Update ready state - must be done while holding the lock
			data->all_philos_ready = true;
			
			// Signal all waiting philosophers that they can proceed
			pthread_cond_broadcast(&data->scheduler->cond);
			
			pthread_mutex_unlock(&data->scheduler->lock);
			break;
		}
		pthread_mutex_unlock(&data->scheduler->lock);
		usleep(1000);
	}
	
	// Small delay to ensure all philosophers are truly ready
	usleep(5000);
	
	// Main scheduling loop
	while (!is_simulation_stopped(data))
	{
		// Reset cycle completion flag and active philosophers
		pthread_mutex_lock(&data->scheduler->lock);
		data->scheduler->cycle_complete = false;
		data->scheduler->active_philos = data->philo_count;
		pthread_mutex_unlock(&data->scheduler->lock);
		
		// Update last check time
		current_time = get_time();
		pthread_mutex_lock(&data->scheduler->lock);
		data->scheduler->last_check_time = current_time;
		pthread_mutex_unlock(&data->scheduler->lock);
		
		// Signal all philosophers to perform their next action
		pthread_mutex_lock(&data->scheduler->lock);
		pthread_cond_broadcast(&data->scheduler->cond);
		pthread_mutex_unlock(&data->scheduler->lock);
		
		// Wait for all active philosophers to complete their actions
		while (!is_simulation_stopped(data))
		{
			pthread_mutex_lock(&data->scheduler->lock);
			if (data->scheduler->cycle_complete)
			{
				pthread_mutex_unlock(&data->scheduler->lock);
				break;
			}
			pthread_mutex_unlock(&data->scheduler->lock);
			usleep(1000);
		}
		
		// Ensure enough time between cycles for proper meal registering
		if (data->time_to_die < 3 * (data->time_to_eat + data->time_to_sleep))
		{
			// For tight timing constraints, add a small safety margin
			usleep(10000);
		}
		
		// Synchronize philosophers to ensure consistent timing
		synchronize_philosophers(data);
	}
	
	// Final signal to wake up any waiting philosophers
	pthread_mutex_lock(&data->scheduler->lock);
	pthread_cond_broadcast(&data->scheduler->cond);
	pthread_mutex_unlock(&data->scheduler->lock);
	
	return (NULL);
}

/*
** Wait for the scheduler to signal this philosopher's turn to act
*/
t_action_result	wait_for_turn(t_philo *philo)
{
	t_data *data;
	bool philos_ready;
	
	data = philo->data;
	
	// Check if simulation has stopped
	if (is_simulation_stopped(data))
		return (ACTION_SIMULATION_STOPPED);
	
	// Wait for scheduler signal
	pthread_mutex_lock(&data->scheduler->lock);
	
	// Check if philosophers are ready, and wait if not
	philos_ready = data->all_philos_ready;
	while (!is_simulation_stopped(data) && !philos_ready)
	{
		pthread_cond_wait(&data->scheduler->cond, &data->scheduler->lock);
		// After waking up, update philos_ready status
		philos_ready = data->all_philos_ready;
	}
	
	// Check again after waking up
	if (is_simulation_stopped(data))
	{
		pthread_mutex_unlock(&data->scheduler->lock);
		return (ACTION_SIMULATION_STOPPED);
	}
	
	pthread_mutex_unlock(&data->scheduler->lock);
	return (ACTION_SUCCESS);
}

/*
** Signal to the scheduler that this philosopher has completed its action
*/
t_action_result	signal_action_complete(t_philo *philo)
{
	t_data *data;
	int active_count;
	
	data = philo->data;
	
	// Check if simulation has stopped
	if (is_simulation_stopped(data))
		return (ACTION_SIMULATION_STOPPED);
	
	pthread_mutex_lock(&data->scheduler->lock);
	
	// Decrement active philosophers count
	data->scheduler->active_philos--;
	active_count = data->scheduler->active_philos;
	
	// If all philosophers have completed their actions, mark cycle as complete
	if (active_count == 0)
		data->scheduler->cycle_complete = true;
	
	pthread_mutex_unlock(&data->scheduler->lock);
	
	return (ACTION_SUCCESS);
}

/*
** Synchronize all philosophers to ensure consistent timing
*/
t_action_result	synchronize_philosophers(t_data *data)
{
	long current_time;
	long elapsed;
	long target_cycle_time;
	long wait_time;
	
	// Check if simulation has stopped
	if (is_simulation_stopped(data))
		return (ACTION_SIMULATION_STOPPED);
	
	// Calculate elapsed time since last cycle
	current_time = get_time();
	
	pthread_mutex_lock(&data->scheduler->lock);
	elapsed = current_time - data->scheduler->last_check_time;
	pthread_mutex_unlock(&data->scheduler->lock);
	
	// Target cycle time is the minimum of time_to_eat and time_to_sleep
	target_cycle_time = data->time_to_eat;
	if (data->time_to_sleep < target_cycle_time)
		target_cycle_time = data->time_to_sleep;
	
	// Ensure minimum cycle time for stability
	if (target_cycle_time < MIN_SLEEP_TIME)
		target_cycle_time = MIN_SLEEP_TIME;
	
	// If we haven't reached the target cycle time, wait
	if (elapsed < target_cycle_time)
	{
		wait_time = target_cycle_time - elapsed;
		precise_sleep(wait_time);
	}
	
	return (ACTION_SUCCESS);
}
