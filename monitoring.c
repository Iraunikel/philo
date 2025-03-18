/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitoring.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/18 11:57:07 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/18 12:26:41 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

void	*death_monitor(void *arg)
{
	t_data	*data;

	data = (t_data *)arg;
	while (!data->simulation_stop)
	{
		if (check_philosophers(data))
			break ;
		if (check_all_ate_enough(data))
		{
			data->simulation_stop = 1;
			break ;
		}
		usleep(100);
	}
	return (NULL);
}

void	handle_death(t_data *data, int i)
{
	pthread_mutex_lock(&data->print_lock);
	data->simulation_stop = 1;
	printf("%lld %d died\n", get_time() - data->start_time,
		data->philosophers[i].id);
	pthread_mutex_unlock(&data->print_lock);
}

int	check_philosophers(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->philo_count && !data->simulation_stop)
	{
		if (check_death(&data->philosophers[i]))
		{
			handle_death(data, i);
			return (1);
		}
		i++;
	}
	return (0);
}

int	check_death(t_philo *philo)
{
	long long	current_time;

	pthread_mutex_lock(&philo->data->death_lock);
	current_time = get_time();
	if ((current_time - philo->last_meal_time) >= philo->data->time_to_die)
	{
		pthread_mutex_unlock(&philo->data->death_lock);
		return (1);
	}
	pthread_mutex_unlock(&philo->data->death_lock);
	return (0);
}

int	check_all_ate_enough(t_data *data)
{
	int	i;
	int	all_ate;

	if (data->meals_to_eat == -1)
		return (0);
	i = 0;
	all_ate = 1;
	while (i < data->philo_count)
	{
		if (data->philosophers[i].meals_eaten < data->meals_to_eat)
		{
			all_ate = 0;
			break ;
		}
		i++;
	}
	return (all_ate);
}
