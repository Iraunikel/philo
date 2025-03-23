/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitoring.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/18 11:57:07 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/23 19:05:33 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

void	*death_monitor(void *arg)
{
	t_data	*data;
	int		stop;

	data = (t_data *)arg;
	stop = 0;
	while (!stop)
	{
		pthread_mutex_lock(&data->death_lock);
		stop = data->simulation_stop;
		pthread_mutex_unlock(&data->death_lock);
		
		if (stop)
			break;
			
		if (check_philosophers(data))
			break;
			
		if (check_all_ate_enough(data))
		{
			pthread_mutex_lock(&data->death_lock);
			data->simulation_stop = 1;
			pthread_mutex_unlock(&data->death_lock);
			break;
		}
		usleep(100);
	}
	return (NULL);
}

void	handle_death(t_data *data, int i)
{
	pthread_mutex_lock(&data->death_lock);
	data->simulation_stop = 1;
	pthread_mutex_unlock(&data->death_lock);
	pthread_mutex_lock(&data->print_lock);
	printf("%lld %d died\n", get_time() - data->start_time,
		data->philosophers[i].id);
	pthread_mutex_unlock(&data->print_lock);
}

int	check_philosophers(t_data *data)
{
	int	i;
	int	stop;

	i = 0;
	while (i < data->philo_count)
	{
		pthread_mutex_lock(&data->death_lock);
		stop = data->simulation_stop;
		pthread_mutex_unlock(&data->death_lock);
		
		if (stop)
			return (0);
			
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
	long long	last_meal;

	pthread_mutex_lock(&philo->meal_lock);
	last_meal = philo->last_meal_time;
	pthread_mutex_unlock(&philo->meal_lock);
	current_time = get_time();
	return ((current_time - last_meal) >= philo->data->time_to_die);
}

int	check_all_ate_enough(t_data *data)
{
	int	i;
	int	all_ate;
	int	meals;

	if (data->meals_to_eat == -1)
		return (0);
	i = 0;
	all_ate = 1;
	while (i < data->philo_count)
	{
		pthread_mutex_lock(&data->philosophers[i].meal_lock);
		meals = data->philosophers[i].meals_eaten;
		pthread_mutex_unlock(&data->philosophers[i].meal_lock);
		if (meals < data->meals_to_eat)
		{
			all_ate = 0;
			break ;
		}
		i++;
	}
	return (all_ate);
}
