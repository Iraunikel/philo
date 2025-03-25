/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitoring.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 20:26:10 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/25 21:47:46 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** Check if a philosopher has died due to starvation
** Uses multiple verification steps to ensure accuracy
*/
int check_philosopher_death(t_philo *philo, long current_time)
{
    long last_meal;
    int time_to_die;
    long time_since_last_meal;
    int meals_eaten;
    bool simulation_stopped;

    // First check if simulation is already stopped
    pthread_mutex_lock(&philo->data->global_lock);
    simulation_stopped = philo->data->simulation_stop;
    pthread_mutex_unlock(&philo->data->global_lock);
    
    if (simulation_stopped)
        return (0);
    
    time_to_die = philo->data->time_to_die;
    
    // Check if philosopher has eaten enough meals
    pthread_mutex_lock(&philo->data->global_lock);
    meals_eaten = philo->meals_eaten;
    pthread_mutex_unlock(&philo->data->global_lock);
    
    // If philosopher has eaten enough, don't check for death
    if (philo->data->meals_to_eat != -1 && meals_eaten >= philo->data->meals_to_eat)
        return (0);
    
    // Get last meal time with proper synchronization
    pthread_mutex_lock(&philo->data->global_lock);
    last_meal = philo->last_meal_time;
    pthread_mutex_unlock(&philo->data->global_lock);
    
    // Special case for initial state
    if (last_meal == 0)
    {
        // If philosopher hasn't eaten at all and time_to_die has passed
        time_since_last_meal = current_time - philo->data->start_time;
    }
    else
    {
        time_since_last_meal = current_time - last_meal;
    }
    
    // Check if philosopher has starved
    if (time_since_last_meal > time_to_die)
    {
        // Add a substantial delay to ensure all threads can update their state
        // This prevents false positives due to race conditions
        usleep(SAFETY_MARGIN * 1000);
        
        // Get current time again after delay
        current_time = get_time();
        
        // Re-check last meal time to ensure it hasn't been updated
        pthread_mutex_lock(&philo->data->global_lock);
        last_meal = philo->last_meal_time;
        pthread_mutex_unlock(&philo->data->global_lock);
        
        // Recalculate time since last meal
        if (last_meal == 0)
            time_since_last_meal = current_time - philo->data->start_time;
        else
            time_since_last_meal = current_time - last_meal;
        
        // Double-check if philosopher has starved
        if (time_since_last_meal > time_to_die)
        {
            // Triple-check meals eaten again before declaring death
            pthread_mutex_lock(&philo->data->global_lock);
            meals_eaten = philo->meals_eaten;
            simulation_stopped = philo->data->simulation_stop;
            pthread_mutex_unlock(&philo->data->global_lock);
            
            // Final verification that philosopher hasn't eaten enough and simulation isn't stopped
            if (!simulation_stopped && 
                (philo->data->meals_to_eat == -1 || meals_eaten < philo->data->meals_to_eat))
            {
                return (handle_death(philo, current_time));
            }
        }
    }
    
    return (0);
}

/*
** Handle philosopher death by stopping the simulation
*/
int handle_death(t_philo *philo, long current_time)
{
    bool was_stopped;

    // Set simulation stop flag with proper synchronization
    pthread_mutex_lock(&philo->data->global_lock);
    was_stopped = philo->data->simulation_stop;
    
    // Only handle death if simulation isn't already stopped
    if (!was_stopped)
    {
        philo->data->simulation_stop = true;
        pthread_mutex_unlock(&philo->data->global_lock);
        
        // Print death message
        pthread_mutex_lock(&philo->data->print_lock);
        printf("%ld %d died\n", current_time - philo->data->start_time, philo->id);
        pthread_mutex_unlock(&philo->data->print_lock);
        
        return (1);
    }
    else
    {
        pthread_mutex_unlock(&philo->data->global_lock);
    }
    
    return (0);
}

/*
** Main monitoring thread that checks for philosopher deaths
** and meal completion
*/
void *death_monitor(void *arg)
{
    t_data *data;
    int i;
    long current_time;
    bool simulation_stopped;

    data = (t_data *)arg;
    
    // Wait for all philosophers to be ready
    while (1)
    {
        pthread_mutex_lock(&data->global_lock);
        if (data->all_philos_ready)
        {
            pthread_mutex_unlock(&data->global_lock);
            break;
        }
        simulation_stopped = data->simulation_stop;
        pthread_mutex_unlock(&data->global_lock);
        
        if (simulation_stopped)
            return (NULL);
            
        usleep(1000);
    }
    
    // Main monitoring loop
    while (1)
    {
        // Check if simulation has stopped
        pthread_mutex_lock(&data->global_lock);
        simulation_stopped = data->simulation_stop;
        pthread_mutex_unlock(&data->global_lock);
        
        if (simulation_stopped)
            break;
        
        // Check if all philosophers have eaten enough
        if (data->meals_to_eat != -1)
        {
            if (check_all_ate_enough(data))
                break;
        }
        
        // Check each philosopher for death
        current_time = get_time();
        i = 0;
        while (i < data->philo_count)
        {
            if (check_philosopher_death(&data->philosophers[i], current_time))
                break;
            i++;
        }
        
        // Sleep between checks to reduce CPU usage
        usleep(5000);
    }
    
    // Print final meal counts for verification
    print_final_meal_counts(data);
    
    return (NULL);
}

/*
** Check if all philosophers have eaten enough meals
*/
int check_all_ate_enough(t_data *data)
{
    int i;
    int meals;
    bool all_ate_enough;
    bool simulation_stopped;

    // If no meal limit is set, return immediately
    if (data->meals_to_eat == -1)
        return (0);
    
    // Check if simulation has already stopped
    pthread_mutex_lock(&data->global_lock);
    simulation_stopped = data->simulation_stop;
    pthread_mutex_unlock(&data->global_lock);
    
    if (simulation_stopped)
        return (0);
    
    // Check if all philosophers have eaten enough
    all_ate_enough = true;
    
    pthread_mutex_lock(&data->global_lock);
    
    for (i = 0; i < data->philo_count; i++)
    {
        meals = data->philosophers[i].meals_eaten;
        
        if (meals < data->meals_to_eat)
        {
            all_ate_enough = false;
            break;
        }
    }
    
    // If all philosophers have eaten enough, stop the simulation
    if (all_ate_enough && !data->simulation_stop)
    {
        data->simulation_stop = true;
        pthread_mutex_unlock(&data->global_lock);
        
        // Print completion message
        pthread_mutex_lock(&data->print_lock);
        printf("All philosophers have eaten at least %d times. Simulation complete.\n", 
               data->meals_to_eat);
        pthread_mutex_unlock(&data->print_lock);
        
        return (1);
    }
    
    pthread_mutex_unlock(&data->global_lock);
    return (0);
}

/*
** Print final meal counts for verification
*/
void print_final_meal_counts(t_data *data)
{
    int i;
    int meals;
    bool all_ate_enough = true;
    
    // Wait a moment to ensure all threads have completed their actions
    usleep(10000);
    
    pthread_mutex_lock(&data->print_lock);
    printf("\n--- Final Meal Counts ---\n");
    
    for (i = 0; i < data->philo_count; i++)
    {
        pthread_mutex_lock(&data->global_lock);
        meals = data->philosophers[i].meals_eaten;
        pthread_mutex_unlock(&data->global_lock);
        
        printf("Philosopher %d ate %d times\n", data->philosophers[i].id, meals);
        
        if (data->meals_to_eat != -1 && meals < data->meals_to_eat)
            all_ate_enough = false;
    }
    
    if (data->meals_to_eat != -1)
    {
        if (all_ate_enough)
            printf("SUCCESS: All philosophers ate at least %d times\n", data->meals_to_eat);
        else
            printf("FAILURE: Not all philosophers ate at least %d times\n", data->meals_to_eat);
    }
    
    pthread_mutex_unlock(&data->print_lock);
}
