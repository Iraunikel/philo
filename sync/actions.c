/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   actions.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 20:24:30 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/25 21:47:46 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** Centralized fork acquisition with deadlock prevention
** Always acquires forks in ascending order regardless of philosopher ID
*/
t_action_result take_forks(t_philo *philo)
{
    t_data *data;
    int first_fork;
    int second_fork;

    data = philo->data;
    
    // Handle single philosopher case
    if (data->philo_count == 1)
    {
        pthread_mutex_lock(&data->forks[philo->left_fork]);
        philo->has_left_fork = true;
        log_action(philo, "has taken a fork");
        
        // Single philosopher can never eat, wait until death
        precise_sleep(data->time_to_die + 10);
        
        // Release the fork before returning
        pthread_mutex_unlock(&data->forks[philo->left_fork]);
        philo->has_left_fork = false;
        
        return (ACTION_SIMULATION_STOPPED);
    }
    
    // Check if simulation has stopped
    if (is_simulation_stopped(data))
        return (ACTION_SIMULATION_STOPPED);
    
    // Always lock forks in ascending order to prevent deadlocks
    if (philo->left_fork < philo->right_fork)
    {
        first_fork = philo->left_fork;
        second_fork = philo->right_fork;
    }
    else
    {
        first_fork = philo->right_fork;
        second_fork = philo->left_fork;
    }
    
    // Acquire first fork
    pthread_mutex_lock(&data->forks[first_fork]);
    if (first_fork == philo->left_fork)
        philo->has_left_fork = true;
    else
        philo->has_right_fork = true;
    
    // Check if simulation stopped after acquiring first fork
    if (is_simulation_stopped(data))
    {
        release_forks(philo);
        return (ACTION_SIMULATION_STOPPED);
    }
    
    log_action(philo, "has taken a fork");
    
    // Acquire second fork
    pthread_mutex_lock(&data->forks[second_fork]);
    if (second_fork == philo->left_fork)
        philo->has_left_fork = true;
    else
        philo->has_right_fork = true;
    
    // Check if simulation stopped after acquiring second fork
    if (is_simulation_stopped(data))
    {
        release_forks(philo);
        return (ACTION_SIMULATION_STOPPED);
    }
    
    log_action(philo, "has taken a fork");
    
    return (ACTION_SUCCESS);
}

/*
** Safely release forks, checking which ones the philosopher actually holds
*/
void release_forks(t_philo *philo)
{
    t_data *data;

    data = philo->data;
    
    // Only release forks that the philosopher actually holds
    if (philo->has_right_fork)
    {
        pthread_mutex_unlock(&data->forks[philo->right_fork]);
        philo->has_right_fork = false;
    }
    
    if (philo->has_left_fork)
    {
        pthread_mutex_unlock(&data->forks[philo->left_fork]);
        philo->has_left_fork = false;
    }
}

/*
** Update the philosopher's last meal time with proper synchronization
*/
t_action_result update_meal_time(t_philo *philo)
{
    t_data *data;
    long current_time;

    data = philo->data;
    
    // Check if simulation has stopped
    if (is_simulation_stopped(data))
        return (ACTION_SIMULATION_STOPPED);
    
    current_time = get_time();
    
    // Update last meal time with global lock for absolute thread safety
    pthread_mutex_lock(&data->global_lock);
    philo->last_meal_time = current_time;
    pthread_mutex_unlock(&data->global_lock);
    
    return (ACTION_SUCCESS);
}

/*
** Increment the philosopher's meal count with proper synchronization
*/
t_action_result increment_meals_eaten(t_philo *philo)
{
    t_data *data;

    data = philo->data;
    
    // Check if simulation has stopped
    if (is_simulation_stopped(data))
        return (ACTION_SIMULATION_STOPPED);
    
    // Increment meals eaten with global lock for absolute thread safety
    pthread_mutex_lock(&data->global_lock);
    philo->meals_eaten++;
    pthread_mutex_unlock(&data->global_lock);
    
    // Check if all philosophers have eaten enough
    if (data->meals_to_eat != -1 && philo->meals_eaten >= data->meals_to_eat)
        check_meal_completion(data);
    
    return (ACTION_SUCCESS);
}

/*
** Check if all philosophers have eaten enough meals to end the simulation
*/
t_action_result check_meal_completion(t_data *data)
{
    int i;
    bool all_ate_enough;
    
    // Check if simulation has stopped
    if (is_simulation_stopped(data))
        return (ACTION_SIMULATION_STOPPED);
    
    // If no meal limit is set, return immediately
    if (data->meals_to_eat == -1)
        return (ACTION_SUCCESS);
    
    all_ate_enough = true;
    
    // Check each philosopher's meal count
    pthread_mutex_lock(&data->global_lock);
    
    for (i = 0; i < data->philo_count; i++)
    {
        if (data->philosophers[i].meals_eaten < data->meals_to_eat)
        {
            all_ate_enough = false;
            break;
        }
    }
    
    // If all philosophers have eaten enough, stop the simulation
    if (all_ate_enough)
    {
        data->simulation_stop = true;
        pthread_mutex_unlock(&data->global_lock);
        
        // Print completion message
        pthread_mutex_lock(&data->print_lock);
        printf("All philosophers have eaten at least %d times. Simulation complete.\n", 
               data->meals_to_eat);
        pthread_mutex_unlock(&data->print_lock);
        
        return (ACTION_SIMULATION_STOPPED);
    }
    
    pthread_mutex_unlock(&data->global_lock);
    return (ACTION_SUCCESS);
}
