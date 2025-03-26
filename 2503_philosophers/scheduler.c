/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 20:23:30 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/25 23:07:50 by iunikel          ###   ########.fr       */
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
** Wait for all philosophers to initialize and mark them as ready
*/
void	wait_for_all_philos_init(t_data *data)
{
	while (1)
	{
		pthread_mutex_lock(&data->scheduler->lock);
		if (data->scheduler->ready_philos == data->philo_count)
		{
			data->all_philos_ready = true;
			pthread_cond_broadcast(&data->scheduler->cond);
			pthread_mutex_unlock(&data->scheduler->lock);
			break;
		}
		pthread_mutex_unlock(&data->scheduler->lock);
		usleep(1000);
	}
	usleep(5000);
}

/*
** Reset the cycle state and update timing
*/
void	reset_cycle_state(t_data *data)
{
	long current_time;
	
	pthread_mutex_lock(&data->scheduler->lock);
	data->scheduler->cycle_complete = false;
	data->scheduler->active_philos = data->philo_count;
	pthread_mutex_unlock(&data->scheduler->lock);
	
	current_time = get_time();
	pthread_mutex_lock(&data->scheduler->lock);
	data->scheduler->last_check_time = current_time;
	pthread_mutex_unlock(&data->scheduler->lock);
}

/*
** Signal all philosophers to start their next action
*/
void	signal_all_philosophers(t_data *data)
{
	pthread_mutex_lock(&data->scheduler->lock);
	pthread_cond_broadcast(&data->scheduler->cond);
	pthread_mutex_unlock(&data->scheduler->lock);
}

/*
** Wait for all philosophers to complete their actions
*/
void	wait_for_cycle_completion(t_data *data)
{
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
	
	if (data->time_to_die < 3 * (data->time_to_eat + data->time_to_sleep))
		usleep(10000);
}

/*
** The scheduler is the central control mechanism that ensures deterministic
** execution of philosopher actions. It coordinates all philosophers to prevent
** race conditions and ensure absolute stability.
*/
void	*scheduler_routine(void *arg)
{
	t_data *data;

	data = (t_data *)arg;
	
	wait_for_all_philos_init(data);
	
	while (!is_simulation_stopped(data))
	{
		reset_cycle_state(data);
		signal_all_philosophers(data);
		wait_for_cycle_completion(data);
		synchronize_philosophers(data);
	}
	
	signal_all_philosophers(data);
	
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
	
	if (is_simulation_stopped(data))
		return (ACTION_SIMULATION_STOPPED);
	
	pthread_mutex_lock(&data->scheduler->lock);
	
	philos_ready = data->all_philos_ready;
	while (!is_simulation_stopped(data) && !philos_ready)
	{
		pthread_cond_wait(&data->scheduler->cond, &data->scheduler->lock);
		philos_ready = data->all_philos_ready;
	}
	
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
	
	if (is_simulation_stopped(data))
		return (ACTION_SIMULATION_STOPPED);
	
	pthread_mutex_lock(&data->scheduler->lock);
	
	data->scheduler->active_philos--;
	active_count = data->scheduler->active_philos;
	
	if (active_count == 0)
		data->scheduler->cycle_complete = true;
	
	pthread_mutex_unlock(&data->scheduler->lock);
	
	return (ACTION_SUCCESS);
}

/*
** Calculate the target cycle time for synchronization
*/
long	get_target_cycle_time(t_data *data)
{
	long target_cycle_time;
	
	target_cycle_time = data->time_to_eat;
	if (data->time_to_sleep < target_cycle_time)
		target_cycle_time = data->time_to_sleep;
	
	if (target_cycle_time < MIN_SLEEP_TIME)
		target_cycle_time = MIN_SLEEP_TIME;
		
	return (target_cycle_time);
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
	
	if (is_simulation_stopped(data))
		return (ACTION_SIMULATION_STOPPED);
	
	current_time = get_time();
	
	pthread_mutex_lock(&data->scheduler->lock);
	elapsed = current_time - data->scheduler->last_check_time;
	pthread_mutex_unlock(&data->scheduler->lock);
	
	target_cycle_time = get_target_cycle_time(data);
	
	if (elapsed < target_cycle_time)
	{
		wait_time = target_cycle_time - elapsed;
		precise_sleep(wait_time);
	}
	
	return (ACTION_SUCCESS);
}
