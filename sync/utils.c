/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 20:27:15 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/25 21:47:46 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** Get current time in milliseconds with high precision
*/
long get_time(void)
{
    struct timeval tv;
    
    if (gettimeofday(&tv, NULL) == -1)
        return (0);
    return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

/*
** Print philosopher state with proper synchronization
*/
int print_state(t_philo *philo, char *state)
{
    long current_time;
    bool simulation_stopped;
    
    // Get current timestamp first to be accurate
    current_time = get_time();
    
    // Check if simulation has stopped
    pthread_mutex_lock(&philo->data->global_lock);
    simulation_stopped = philo->data->simulation_stop;
    pthread_mutex_unlock(&philo->data->global_lock);
    
    if (simulation_stopped)
        return (0);
    
    pthread_mutex_lock(&philo->data->print_lock);
    printf("%ld %d %s\n", 
           current_time - philo->data->start_time, 
           philo->id, 
           state);
    pthread_mutex_unlock(&philo->data->print_lock);
    
    return (1);
}

/*
** Log philosopher action with timestamp
*/
void log_action(t_philo *philo, const char *action)
{
    print_state(philo, (char *)action);
}

/*
** Set simulation stop flag with proper synchronization
*/
void set_simulation_stop(t_data *data, bool value)
{
    pthread_mutex_lock(&data->global_lock);
    data->simulation_stop = value;
    pthread_mutex_unlock(&data->global_lock);
}

/*
** Convert string to integer with error checking
*/
int ft_atoi(const char *str)
{
    int i;
    int sign;
    long result;

    i = 0;
    sign = 1;
    result = 0;
    
    // Skip whitespace
    while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
        i++;
    
    // Handle sign
    if (str[i] == '-' || str[i] == '+')
    {
        if (str[i] == '-')
            sign = -1;
        i++;
    }
    
    // Convert digits
    while (str[i] >= '0' && str[i] <= '9')
    {
        result = result * 10 + (str[i] - '0');
        
        // Check for overflow
        if (result > INT_MAX && sign == 1)
            return (INT_MAX);
        if (result > (long)INT_MAX + 1 && sign == -1)
            return (INT_MIN);
        
        i++;
    }
    
    return ((int)(result * sign));
}
