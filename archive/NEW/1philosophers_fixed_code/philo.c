/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/18 12:03:39 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/23 19:35:00 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int     create_threads(t_data *data, pthread_t *monitor)
{
        int     i;

        i = 0;
        // Set simulation_stop with mutex protection before creating threads
        pthread_mutex_lock(&data->death_lock);
        data->simulation_stop = 0;
        pthread_mutex_unlock(&data->death_lock);
        
        while (i < data->philo_count)
        {
                if (pthread_create(&data->philosophers[i].thread, NULL,
                                philosopher_routine, &data->philosophers[i]))
                {
                        // Use mutex protection when modifying simulation_stop
                        pthread_mutex_lock(&data->death_lock);
                        data->simulation_stop = 1;
                        pthread_mutex_unlock(&data->death_lock);
                        
                        printf(ERR_THREAD "\n");
                        cleanup_simulation(data);
                        return (0);
                }
                i++;
        }
        if (pthread_create(monitor, NULL, death_monitor, data))
        {
                // Use mutex protection when modifying simulation_stop
                pthread_mutex_lock(&data->death_lock);
                data->simulation_stop = 1;
                pthread_mutex_unlock(&data->death_lock);
                
                printf(ERR_THREAD "\n");
                cleanup_simulation(data);
                return (0);
        }
        return (1);
}

int     main(int argc, char **argv)
{
        t_data          data;
        pthread_t       monitor;

        if (argc != 5 && argc != 6)
        {
                printf(ERR_ARGS "\n");
                return (1);
        }
        if (!init_data(&data, argc, argv))
                return (1);
        if (!allocate_resources(&data))
                return (1);
        if (!init_mutexes(&data) || !init_philosophers(&data))
        {
                free(data.philosophers);
                free(data.forks);
                printf(ERR_MUTEX "\n");
                return (1);
        }
        if (!create_threads(&data, &monitor))
                return (1);
        
        // Wait for the monitor thread before setting simulation_stop
        pthread_join(monitor, NULL);
        
        // Set simulation_stop to make sure all philosophers exit
        pthread_mutex_lock(&data.death_lock);
        data.simulation_stop = 1;
        pthread_mutex_unlock(&data.death_lock);
        
        // Cleanup will join all philosopher threads and free resources
        cleanup_simulation(&data);
        return (0);
}
