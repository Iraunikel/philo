/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 20:28:45 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/25 21:47:46 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** Parse and validate command line arguments
*/
int parse_args(int argc, char **argv, t_data *data)
{
    // Check argument count
    if (argc < 5 || argc > 6)
    {
        printf("Error: Invalid number of arguments\n");
        printf("Usage: %s number_of_philosophers time_to_die time_to_eat ", argv[0]);
        printf("time_to_sleep [number_of_times_each_philosopher_must_eat]\n");
        return (1);
    }
    
    // Parse number of philosophers
    data->philo_count = ft_atoi(argv[1]);
    if (data->philo_count <= 0 || data->philo_count > MAX_PHILOSOPHERS)
    {
        printf("Error: Invalid number of philosophers (must be between 1 and %d)\n", 
               MAX_PHILOSOPHERS);
        return (1);
    }
    
    // Parse time to die
    data->time_to_die = ft_atoi(argv[2]);
    if (data->time_to_die < MIN_SLEEP_TIME)
    {
        printf("Error: time_to_die must be at least %d ms\n", MIN_SLEEP_TIME);
        return (1);
    }
    
    // Parse time to eat
    data->time_to_eat = ft_atoi(argv[3]);
    if (data->time_to_eat < MIN_SLEEP_TIME)
    {
        printf("Error: time_to_eat must be at least %d ms\n", MIN_SLEEP_TIME);
        return (1);
    }
    
    // Parse time to sleep
    data->time_to_sleep = ft_atoi(argv[4]);
    if (data->time_to_sleep < MIN_SLEEP_TIME)
    {
        printf("Error: time_to_sleep must be at least %d ms\n", MIN_SLEEP_TIME);
        return (1);
    }
    
    // Parse optional number of meals (if provided)
    if (argc == 6)
    {
        data->meals_to_eat = ft_atoi(argv[5]);
        if (data->meals_to_eat <= 0)
        {
            printf("Error: number_of_times_each_philosopher_must_eat must be positive\n");
            return (1);
        }
    }
    else
    {
        data->meals_to_eat = -1; // No meal limit
    }
    
    return (0);
}

/*
** Initialize the simulation
*/
int init_simulation(t_data *data)
{
    // Initialize data structure
    if (init_data(data) != 0)
    {
        printf("Error: Failed to initialize data\n");
        return (1);
    }
    
    // Initialize mutexes
    if (init_mutexes(data) != 0)
    {
        printf("Error: Failed to initialize mutexes\n");
        return (1);
    }
    
    // Initialize philosophers
    if (init_philosophers(data) != 0)
    {
        printf("Error: Failed to initialize philosophers\n");
        cleanup_simulation(data);
        return (1);
    }
    
    return (0);
}

/*
** Create all threads and start the simulation
*/
int create_threads(t_data *data)
{
    // Initialize scheduler and create threads
    if (init_scheduler(data) != 0)
    {
        printf("Error: Failed to create threads\n");
        cleanup_simulation(data);
        return (1);
    }
    
    return (0);
}

/*
** Wait for all threads to complete and clean up resources
*/
int wait_and_cleanup(t_data *data)
{
    // Wait until monitor thread detects end condition
    pthread_join(data->monitor_thread, NULL);
    
    // Now ensure simulation is marked as stopped 
    set_simulation_stop(data, true);
    
    // Signal the scheduler to wake up philosophers
    pthread_mutex_lock(&data->scheduler->lock);
    pthread_cond_broadcast(&data->scheduler->cond);
    pthread_mutex_unlock(&data->scheduler->lock);
    
    // Clean up all resources
    cleanup_simulation(data);
    
    return (0);
}

/*
** Main entry point
*/
int main(int argc, char **argv)
{
    t_data data;
    
    // Parse command line arguments
    if (parse_args(argc, argv, &data) != 0)
        return (1);
    
    // Initialize the simulation
    if (init_simulation(&data) != 0)
        return (1);
    
    // Create threads and start the simulation
    if (create_threads(&data) != 0)
        return (1);
    
    // Wait for all threads to complete and clean up
    wait_and_cleanup(&data);
    
    return (0);
}
