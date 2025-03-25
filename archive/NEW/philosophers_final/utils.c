/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/18 11:51:47 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/24 11:48:30 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	ft_atoi(char const *str)
{
	int	nbr;
	int	i;

	if (!str)
		return (0);
	nbr = 0;
	i = 0;
	while (str[i] >= '0' && str[i] <= '9')
	{
		nbr = nbr * 10 + (str[i] - '0');
		i++;
	}
	return (nbr);
}

long	get_time(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void	print_state(t_philo *philo, char *message)
{
	bool	stopped;

	// Use global lock for thread synchronization
	pthread_mutex_lock(&philo->data->global_lock);
	
	pthread_mutex_lock(&philo->data->death_lock);
	stopped = philo->data->simulation_stop;
	pthread_mutex_unlock(&philo->data->death_lock);
	
	if (!stopped)
	{
		pthread_mutex_lock(&philo->data->print_lock);
		printf("%lld %d %s\n", get_time() - philo->data->start_time, 
			philo->id, message);
		pthread_mutex_unlock(&philo->data->print_lock);
	}
	
	pthread_mutex_unlock(&philo->data->global_lock);
}

void	cleanup_simulation(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->philo_count)
	{
		pthread_join(data->philosophers[i].thread, NULL);
		i++;
	}
	i = 0;
	while (i < data->philo_count)
	{
		pthread_mutex_destroy(&data->forks[i]);
		pthread_mutex_destroy(&data->philosophers[i].meal_lock);
		i++;
	}
	pthread_mutex_destroy(&data->print_lock);
	pthread_mutex_destroy(&data->death_lock);
	pthread_mutex_destroy(&data->global_lock);
	free(data->philosophers);
	free(data->forks);
}

bool	is_simulation_stopped(t_data *data)
{
	bool	stopped;

	pthread_mutex_lock(&data->global_lock);
	pthread_mutex_lock(&data->death_lock);
	stopped = data->simulation_stop;
	pthread_mutex_unlock(&data->death_lock);
	pthread_mutex_unlock(&data->global_lock);
	return (stopped);
}

void	set_simulation_stop(t_data *data, bool value)
{
	pthread_mutex_lock(&data->global_lock);
	pthread_mutex_lock(&data->death_lock);
	data->simulation_stop = value;
	pthread_mutex_unlock(&data->death_lock);
	pthread_mutex_unlock(&data->global_lock);
}

long	get_last_meal_time(t_philo *philo)
{
	long	time;

	pthread_mutex_lock(&philo->data->global_lock);
	pthread_mutex_lock(&philo->meal_lock);
	time = philo->last_meal_time;
	pthread_mutex_unlock(&philo->meal_lock);
	pthread_mutex_unlock(&philo->data->global_lock);
	return (time);
}

void	update_last_meal_time(t_philo *philo, long time)
{
	pthread_mutex_lock(&philo->data->global_lock);
	pthread_mutex_lock(&philo->meal_lock);
	philo->last_meal_time = time;
	pthread_mutex_unlock(&philo->meal_lock);
	pthread_mutex_unlock(&philo->data->global_lock);
}

int	get_meals_eaten(t_philo *philo)
{
	int	meals;

	pthread_mutex_lock(&philo->data->global_lock);
	pthread_mutex_lock(&philo->meal_lock);
	meals = philo->meals_eaten;
	pthread_mutex_unlock(&philo->meal_lock);
	pthread_mutex_unlock(&philo->data->global_lock);
	return (meals);
}

void	increment_meals_eaten(t_philo *philo)
{
	pthread_mutex_lock(&philo->data->global_lock);
	pthread_mutex_lock(&philo->meal_lock);
	philo->meals_eaten++;
	pthread_mutex_unlock(&philo->meal_lock);
	pthread_mutex_unlock(&philo->data->global_lock);
}
