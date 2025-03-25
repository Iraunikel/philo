/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 20:25:15 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/25 21:49:16 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** Precise sleep function with active waiting for maximum timing accuracy
*/
void precise_sleep(int ms)
{
    long start;
    long current;
    long elapsed;

    // Ensure minimum sleep time for stability
    if (ms < MIN_SLEEP_TIME)
        ms = MIN_SLEEP_TIME;
        
    start = get_time();
    while (1)
    {
        current = get_time();
        elapsed = current - start;
        if (elapsed >= ms)
            break;
        // Use longer sleep intervals for better CPU efficiency
        // while maintaining precision
        if (ms - elapsed > 20)
            usleep(2000);
        else
            usleep(500);
    }
}

/*
** Main philosopher routine that coordinates with the scheduler
*/
void *philosopher_routine(void *arg)
{
    t_philo *philo;
    t_data *data;
    t_action_result result;

    philo = (t_philo *)arg;
    data = philo->data;
    
    // Initialize philosopher state
    philo->state = STATE_THINKING;
    philo->has_left_fork = false;
    philo->has_right_fork = false;
    
    // Initialize last_meal_time to start_time
    pthread_mutex_lock(&data->global_lock);
    philo->last_meal_time = data->start_time;
    pthread_mutex_unlock(&data->global_lock);
    
    // Signal to scheduler that this philosopher is ready
    pthread_mutex_lock(&data->scheduler->lock);
    data->scheduler->ready_philos++;
    pthread_mutex_unlock(&data->scheduler->lock);
    
    // Wait for all philosophers to be ready
    pthread_mutex_lock(&data->scheduler->lock);
    while (!data->all_philos_ready && !is_simulation_stopped(data))
    {
        pthread_cond_wait(&data->scheduler->cond, &data->scheduler->lock);
    }
    pthread_mutex_unlock(&data->scheduler->lock);
    
    // If simulation has already stopped, just exit
    if (is_simulation_stopped(data))
        return (NULL);
        
    // Initial thinking state
    log_action(philo, "is thinking");
    
    // Even-numbered philosophers wait a bit to avoid deadlocks
    if (philo->id % 2 == 0)
        precise_sleep(data->time_to_eat / 2);
    
    // Main philosopher lifecycle
    while (!is_simulation_stopped(data))
    {
        // Execute action based on current state
        switch (philo->state)
        {
            case STATE_THINKING:
                result = philosopher_eat(philo);
                break;
            case STATE_EATING:
                result = philosopher_sleep(philo);
                break;
            case STATE_SLEEPING:
                result = philosopher_think(philo);
                break;
            default:
                result = ACTION_SIMULATION_STOPPED;
                break;
        }
        
        if (result == ACTION_SIMULATION_STOPPED)
            break;
            
        // Wait a bit before next action to avoid hogging resources
        usleep(1000);
    }
    
    // Ensure forks are released if philosopher was interrupted while eating
    if (philo->has_left_fork || philo->has_right_fork)
        release_forks(philo);
    
    return (NULL);
}

/*
** Philosopher eating action with robust synchronization
*/
t_action_result philosopher_eat(t_philo *philo)
{
    t_action_result result;
    
    // Check if simulation has stopped
    if (is_simulation_stopped(philo->data))
        return (ACTION_SIMULATION_STOPPED);
    
    // Take forks with centralized control
    result = take_forks(philo);
    if (result != ACTION_SUCCESS)
        return (result);
    
    // Log eating state
    log_action(philo, "is eating");
    
    // Update state to eating
    pthread_mutex_lock(&philo->data->global_lock);
    philo->state = STATE_EATING;
    pthread_mutex_unlock(&philo->data->global_lock);
    
    // Update last meal time
    result = update_meal_time(philo);
    if (result != ACTION_SUCCESS)
    {
        release_forks(philo);
        return (result);
    }
    
    // Sleep for time_to_eat duration
    precise_sleep(philo->data->time_to_eat);
    
    // Increment meals eaten
    result = increment_meals_eaten(philo);
    if (result != ACTION_SUCCESS)
    {
        release_forks(philo);
        return (result);
    }
    
    // Release forks
    release_forks(philo);
    
    return (ACTION_SUCCESS);
}

/*
** Philosopher sleeping action
*/
t_action_result philosopher_sleep(t_philo *philo)
{
    // Check if simulation has stopped
    if (is_simulation_stopped(philo->data))
        return (ACTION_SIMULATION_STOPPED);
    
    // Log sleeping state
    log_action(philo, "is sleeping");
    
    // Update state to sleeping
    pthread_mutex_lock(&philo->data->global_lock);
    philo->state = STATE_SLEEPING;
    pthread_mutex_unlock(&philo->data->global_lock);
    
    // Sleep for time_to_sleep duration
    precise_sleep(philo->data->time_to_sleep);
    
    return (ACTION_SUCCESS);
}

/*
** Philosopher thinking action
*/
t_action_result philosopher_think(t_philo *philo)
{
    // Check if simulation has stopped
    if (is_simulation_stopped(philo->data))
        return (ACTION_SIMULATION_STOPPED);
    
    // Log thinking state
    log_action(philo, "is thinking");
    
    // Update state to thinking
    pthread_mutex_lock(&philo->data->global_lock);
    philo->state = STATE_THINKING;
    pthread_mutex_unlock(&philo->data->global_lock);
    
    return (ACTION_SUCCESS);
}
