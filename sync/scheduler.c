/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 20:23:30 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/25 21:49:16 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** The scheduler is the central control mechanism that ensures deterministic
** execution of philosopher actions. It coordinates all philosophers to prevent
** race conditions and ensure absolute stability.
*/

void *scheduler_routine(void *arg)
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
            data->all_philos_ready = true;
            pthread_cond_broadcast(&data->scheduler->cond);
            pthread_mutex_unlock(&data->scheduler->lock);
            break;
        }
        pthread_mutex_unlock(&data->scheduler->lock);
        usleep(1000);
    }
    
    // Main scheduling loop
    while (!is_simulation_stopped(data))
    {
        // Reset cycle completion flag 
        pthread_mutex_lock(&data->scheduler->lock);
        data->scheduler->cycle_complete = false;
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
        
        // Sleep between cycles for stability
        usleep(SYNC_INTERVAL * 1000);
    }
    
    // Final signal to wake up any waiting philosophers
    pthread_mutex_lock(&data->scheduler->lock);
    pthread_cond_broadcast(&data->scheduler->cond);
    pthread_mutex_unlock(&data->scheduler->lock);
    
    return (NULL);
}

/*
** Check if the simulation has been stopped
*/
bool is_simulation_stopped(t_data *data)
{
    bool stopped;
    
    pthread_mutex_lock(&data->global_lock);
    stopped = data->simulation_stop;
    pthread_mutex_unlock(&data->global_lock);
    
    return (stopped);
}
