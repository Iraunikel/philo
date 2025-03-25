/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 20:27:50 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/25 21:47:46 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** Initialize the data structure with program arguments
*/
int init_data(t_data *data)
{
    // Initialize all values to ensure clean state
    data->philo_count = 0;
    data->time_to_die = 0;
    data->time_to_eat = 0;
    data->time_to_sleep = 0;
    data->meals_to_eat = -1;
    data->start_time = 0;
    data->simulation_stop = false;
    data->all_philos_ready = false;
    data->forks = NULL;
    data->philosophers = NULL;
    data->scheduler = NULL;
    
    // Allocate memory for scheduler
    data->scheduler = (t_scheduler *)malloc(sizeof(t_scheduler));
    if (!data->scheduler)
        return (1);
    
    // Initialize scheduler values
    data->scheduler->active_philos = 0;
    data->scheduler->ready_philos = 0;
    data->scheduler->cycle_complete = false;
    data->scheduler->last_check_time = 0;
    
    return (0);
}

/*
** Initialize all mutexes used in the program
*/
int init_mutexes(t_data *data)
{
    int i;
    
    // Initialize global mutex
    if (pthread_mutex_init(&data->global_lock, NULL) != 0)
        return (1);
    
    // Initialize print mutex
    if (pthread_mutex_init(&data->print_lock, NULL) != 0)
    {
        pthread_mutex_destroy(&data->global_lock);
        return (1);
    }
    
    // Initialize scheduler mutex and condition variable
    if (pthread_mutex_init(&data->scheduler->lock, NULL) != 0)
    {
        pthread_mutex_destroy(&data->global_lock);
        pthread_mutex_destroy(&data->print_lock);
        return (1);
    }
    
    if (pthread_cond_init(&data->scheduler->cond, NULL) != 0)
    {
        pthread_mutex_destroy(&data->global_lock);
        pthread_mutex_destroy(&data->print_lock);
        pthread_mutex_destroy(&data->scheduler->lock);
        return (1);
    }
    
    // Allocate and initialize fork mutexes
    data->forks = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * data->philo_count);
    if (!data->forks)
    {
        pthread_mutex_destroy(&data->global_lock);
        pthread_mutex_destroy(&data->print_lock);
        pthread_mutex_destroy(&data->scheduler->lock);
        pthread_cond_destroy(&data->scheduler->cond);
        return (1);
    }
    
    // Initialize each fork mutex
    i = 0;
    while (i < data->philo_count)
    {
        if (pthread_mutex_init(&data->forks[i], NULL) != 0)
        {
            // Cleanup previously initialized mutexes
            while (--i >= 0)
                pthread_mutex_destroy(&data->forks[i]);
            pthread_mutex_destroy(&data->global_lock);
            pthread_mutex_destroy(&data->print_lock);
            pthread_mutex_destroy(&data->scheduler->lock);
            pthread_cond_destroy(&data->scheduler->cond);
            free(data->forks);
            return (1);
        }
        i++;
    }
    
    return (0);
}

/*
** Initialize philosopher data structures
*/
int init_philosophers(t_data *data)
{
    int i;
    
    // Allocate memory for philosophers
    data->philosophers = (t_philo *)malloc(sizeof(t_philo) * data->philo_count);
    if (!data->philosophers)
        return (1);
    
    // Initialize each philosopher
    i = 0;
    while (i < data->philo_count)
    {
        data->philosophers[i].id = i + 1;
        data->philosophers[i].left_fork = i;
        data->philosophers[i].right_fork = (i + 1) % data->philo_count;
        data->philosophers[i].meals_eaten = 0;
        data->philosophers[i].last_meal_time = 0;
        data->philosophers[i].state = STATE_THINKING;
        data->philosophers[i].has_left_fork = false;
        data->philosophers[i].has_right_fork = false;
        data->philosophers[i].data = data;
        i++;
    }
    
    return (0);
}

/*
** Initialize the scheduler and create all threads
*/
int init_scheduler(t_data *data)
{
    int i;
    
    // Record start time
    data->start_time = get_time();
    if (data->start_time == 0)
        return (1);
    
    // Create scheduler thread
    if (pthread_create(&data->scheduler_thread, NULL, scheduler_routine, data) != 0)
        return (1);
    
    // Create philosopher threads
    i = 0;
    while (i < data->philo_count)
    {
        if (pthread_create(&data->philosophers[i].thread, NULL, 
                          philosopher_routine, &data->philosophers[i]) != 0)
        {
            // Set simulation stop to signal all threads to exit
            set_simulation_stop(data, true);
            
            // Wait for created threads to exit
            while (--i >= 0)
                pthread_join(data->philosophers[i].thread, NULL);
            pthread_join(data->scheduler_thread, NULL);
            return (1);
        }
        i++;
    }
    
    // Create monitoring thread after philosophers are created
    if (pthread_create(&data->monitor_thread, NULL, death_monitor, data) != 0)
    {
        // Set simulation stop to signal threads to exit
        set_simulation_stop(data, true);
        
        // Wait for all created threads to exit
        i = 0;
        while (i < data->philo_count)
        {
            pthread_join(data->philosophers[i].thread, NULL);
            i++;
        }
        pthread_join(data->scheduler_thread, NULL);
        return (1);
    }
    
    return (0);
}

/*
** Clean up all resources used by the program
*/
int cleanup_simulation(t_data *data)
{
    int i;
    
    // Wait for all philosopher threads to exit
    i = 0;
    while (i < data->philo_count)
    {
        pthread_join(data->philosophers[i].thread, NULL);
        i++;
    }
    
    // Wait for scheduler and monitor threads to exit
    pthread_join(data->scheduler_thread, NULL);
    pthread_join(data->monitor_thread, NULL);
    
    // Destroy all mutexes
    i = 0;
    while (i < data->philo_count)
    {
        pthread_mutex_destroy(&data->forks[i]);
        i++;
    }
    
    // Destroy other mutexes
    pthread_mutex_destroy(&data->global_lock);
    pthread_mutex_destroy(&data->print_lock);
    pthread_mutex_destroy(&data->scheduler->lock);
    pthread_cond_destroy(&data->scheduler->cond);
    
    // Free allocated memory
    free(data->forks);
    free(data->philosophers);
    free(data->scheduler);
    
    return (0);
}
