/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:08:54 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/25 22:24:55 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** Get current time in milliseconds
*/
long	get_time(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

/*
** Print philosopher state change with proper mutex protection
*/
int	print_state(t_philo *philo, char *state)
{
	long	current_time;

	// Skip if simulation has been stopped
	if (is_simulation_stopped(philo->data))
		return (0);

	current_time = get_time();
	
	// Use print lock to prevent interleaved output
	pthread_mutex_lock(&philo->data->print_lock);
	
	// Double check simulation status before printing
	if (!is_simulation_stopped(philo->data))
	{
		printf("%ld %d %s\n", 
			current_time - philo->data->start_time, philo->id, state);
	}
	
	pthread_mutex_unlock(&philo->data->print_lock);
	return (1);
}

/*
** Convert string to integer with error checking
*/
int	ft_atoi(const char *str)
{
	int			i;
	int			sign;
	long		result;

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
		if (result * sign > INT_MAX || result * sign < INT_MIN)
			return (sign == 1 ? -1 : 0);
		i++;
	}
	
	return (result * sign);
}

/*
** Check if simulation should stop
*/
bool	is_simulation_stopped(t_data *data)
{
	bool	result;

	// Always use stop_lock for simulation_stop, not global_lock
	pthread_mutex_lock(&data->stop_lock);
	result = data->simulation_stop;
	pthread_mutex_unlock(&data->stop_lock);
	
	return (result);
}

/*
** Set simulation stop flag
*/
void	set_simulation_stop(t_data *data, bool value)
{
	pthread_mutex_lock(&data->stop_lock);
	data->simulation_stop = value;
	pthread_mutex_unlock(&data->stop_lock);
}

long    get_last_meal_time(t_philo *philo)
{
        long    result;

        pthread_mutex_lock(&philo->data->meal_lock);
        result = philo->last_meal_time;
        pthread_mutex_unlock(&philo->data->meal_lock);
        return (result);
}

void    update_last_meal_time(t_philo *philo, long time)
{
        pthread_mutex_lock(&philo->data->meal_lock);
        philo->last_meal_time = time;
        pthread_mutex_unlock(&philo->data->meal_lock);
}

int     get_meals_eaten(t_philo *philo)
{
        int     result;

        pthread_mutex_lock(&philo->data->meal_lock);
        result = philo->meals_eaten;
        pthread_mutex_unlock(&philo->data->meal_lock);
        return (result);
}

void    increment_meals_eaten(t_philo *philo)
{
        pthread_mutex_lock(&philo->data->meal_lock);
        philo->meals_eaten++;
        pthread_mutex_unlock(&philo->data->meal_lock);
}
