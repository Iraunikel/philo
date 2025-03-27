/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:08:54 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/26 14:35:39 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** Get current timestamp in milliseconds
*/
long	get_time(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

/*
** Print philosopher state with proper timestamp
*/
int	print_state(t_philo *philo, char *state)
{
	long	timestamp;

	// Skip if simulation has stopped
	if (is_simulation_stopped(philo->data))
		return (1);
	
	// Get current simulation timestamp
	timestamp = get_sim_time(philo->data);
	
	// Print state with proper mutex protection
	pthread_mutex_lock(&philo->data->print_lock);
	
	if (!is_simulation_stopped(philo->data))
	{
		printf("%ld %d %s\n", timestamp, philo->id, state);
	}
	
	pthread_mutex_unlock(&philo->data->print_lock);
	
	return (0);
}

/*
** Handle whitespace and sign in atoi
*/
int	handle_whitespace_and_sign(const char *str, int *i, int *sign)
{
	// Skip whitespace
	while (str[*i] == ' ' || (str[*i] >= 9 && str[*i] <= 13))
		(*i)++;
	
	// Handle sign
	if (str[*i] == '-' || str[*i] == '+')
	{
		if (str[*i] == '-')
			*sign = -1;
		(*i)++;
	}
	
	return (0);
}

/*
** Convert string to integer with overflow checking
*/
int	convert_to_int(const char *str, int i, int sign)
{
	int	result;

	result = 0;
	
	// Convert to integer
	while (str[i] >= '0' && str[i] <= '9')
	{
		// Check for overflow
		if (result > INT_MAX / 10 || 
			(result == INT_MAX / 10 && str[i] - '0' > INT_MAX % 10))
		{
			if (sign == 1)
				return (INT_MAX);
			else
				return (INT_MIN);
		}
		
		result = result * 10 + (str[i] - '0');
		i++;
	}
	
	return (result * sign);
}

/*
** Parse string to integer with error checking
*/
int	ft_atoi(const char *str)
{
	int	i;
	int	sign;

	i = 0;
	sign = 1;
	
	// Handle whitespace and sign
	handle_whitespace_and_sign(str, &i, &sign);
	
	// Convert digits to integer with overflow checking
	return (convert_to_int(str, i, sign));
}

/*
** Check if simulation has been stopped
*/
bool	is_simulation_stopped(t_data *data)
{
	bool	stopped;

	pthread_mutex_lock(&data->stop_lock);
	stopped = data->simulation_stop;
	pthread_mutex_unlock(&data->stop_lock);
	
	return (stopped);
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

/*
** Get philosopher's last meal time
*/
long	get_last_meal_time(t_philo *philo)
{
	long	result;

	pthread_mutex_lock(&philo->data->meal_lock);
	result = philo->last_meal_time;
	pthread_mutex_unlock(&philo->data->meal_lock);
	return (result);
}

/*
** Update philosopher's last meal time
*/
void	update_last_meal_time(t_philo *philo, long time)
{
	pthread_mutex_lock(&philo->data->meal_lock);
	philo->last_meal_time = time;
	pthread_mutex_unlock(&philo->data->meal_lock);
}

/*
** Get philosopher's meals eaten count
*/
int	get_meals_eaten(t_philo *philo)
{
	int	result;

	pthread_mutex_lock(&philo->data->meal_lock);
	result = philo->meals_eaten;
	pthread_mutex_unlock(&philo->data->meal_lock);
	return (result);
}

/*
** Increment philosopher's meals eaten count
*/
void	increment_meals_eaten(t_philo *philo)
{
	pthread_mutex_lock(&philo->data->meal_lock);
	philo->meals_eaten++;
	pthread_mutex_unlock(&philo->data->meal_lock);
}
