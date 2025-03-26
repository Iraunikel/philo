/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   timekeeper.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:08:54 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/26 14:36:16 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** Initialize the timekeeper
*/
int	init_timekeeper(t_data *data)
{
	data->timekeeper = malloc(sizeof(t_timekeeper));
	if (!data->timekeeper)
		return (1);
	
	pthread_mutex_init(&data->timekeeper->lock, NULL);
	pthread_cond_init(&data->timekeeper->cond, NULL);
	data->timekeeper->current_time = 0;
	data->timekeeper->start_time = get_time();
	data->timekeeper->running = true;
	
	return (0);
}

/*
** Get the current simulation time
*/
long	get_sim_time(t_data *data)
{
	long current_time;
	
	pthread_mutex_lock(&data->timekeeper->lock);
	current_time = data->timekeeper->current_time;
	pthread_mutex_unlock(&data->timekeeper->lock);
	
	return (current_time);
}

/*
** Wait until a specific simulation time
*/
void	wait_until(t_philo *philo, long target_time)
{
	long current_time;
	
	while (!is_simulation_stopped(philo->data))
	{
		pthread_mutex_lock(&philo->data->timekeeper->lock);
		current_time = philo->data->timekeeper->current_time;
		
		if (current_time >= target_time)
		{
			pthread_mutex_unlock(&philo->data->timekeeper->lock);
			return;
		}
		
		pthread_cond_wait(&philo->data->timekeeper->cond, 
			&philo->data->timekeeper->lock);
		pthread_mutex_unlock(&philo->data->timekeeper->lock);
	}
}

/*
** The timekeeper thread that controls the simulation time
*/
void	*timekeeper_routine(void *arg)
{
	t_data *data;
	long real_time;
	long elapsed;
	long last_update;
	struct timespec ts;
	
	data = (t_data *)arg;
	last_update = get_time();
	
	while (!is_simulation_stopped(data))
	{
		// Get real current time with high precision
		real_time = get_time();
		elapsed = real_time - last_update;
		
		// Update timekeeper with small increments to ensure precision
		if (elapsed >= STEP_SIZE)
		{
			// Lock only during the critical section to reduce lock contention
			pthread_mutex_lock(&data->timekeeper->lock);
			// Increment by exact elapsed time (capped at MAX_SAFETY_MARGIN) for precision
			data->timekeeper->current_time += (elapsed > MAX_SAFETY_MARGIN) ? 
				STEP_SIZE : elapsed;
			// Immediately notify waiting threads
			pthread_cond_broadcast(&data->timekeeper->cond);
			pthread_mutex_unlock(&data->timekeeper->lock);
			
			last_update = real_time;
		}
		
		// Use nanosleep for more precise sleeping
		ts.tv_sec = 0;
		ts.tv_nsec = 50000; // 50 microseconds (0.05ms)
		nanosleep(&ts, NULL);
	}
	
	return (NULL);
} 