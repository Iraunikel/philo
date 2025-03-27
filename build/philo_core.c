/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_core.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 16:35:00 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/27 16:35:00 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	check_philosopher_death(t_philo *philo, long current_time)
{
	long	last_meal;
	long	time_since_last_meal;

	if (is_simulation_stopped(philo->data))
		return (0);
	pthread_mutex_lock(&philo->data->data_lock);
	last_meal = philo->last_meal_time;
	pthread_mutex_unlock(&philo->data->data_lock);
	if (last_meal == 0)
		time_since_last_meal = current_time;
	else
		time_since_last_meal = current_time - last_meal;
	if (time_since_last_meal >= philo->data->time_to_die)
	{
		set_simulation_stop(philo->data, true);
		pthread_mutex_lock(&philo->data->print_lock);
		printf("%ld %d died\n", current_time, philo->id);
		pthread_mutex_unlock(&philo->data->print_lock);
		return (1);
	}
	return (0);
}

int	check_all_ate_enough(t_data *data)
{
	int		i;
	int		meals;
	bool	all_ate_enough;

	if (data->meals_to_eat == -1 || is_simulation_stopped(data))
		return (0);
	all_ate_enough = true;
	i = 0;
	while (i < data->philo_count)
	{
		pthread_mutex_lock(&data->data_lock);
		meals = data->philosophers[i].meals_eaten;
		pthread_mutex_unlock(&data->data_lock);
		if (meals < data->meals_to_eat)
		{
			all_ate_enough = false;
			break;
		}
		i++;
	}
	if (all_ate_enough)
		set_simulation_stop(data, true);
	return (all_ate_enough);
}

void	*death_monitor(void *arg)
{
	t_data	*data;
	int		i;
	long	current_time;

	data = (t_data *)arg;
	usleep(1000);
	while (!is_simulation_stopped(data))
	{
		current_time = get_elapsed_time(data);
		i = 0;
		while (i < data->philo_count)
		{
			if (check_philosopher_death(&data->philosophers[i], current_time))
				return (NULL);
			i++;
		}
		if (check_all_ate_enough(data))
			return (NULL);
		usleep(500);
	}
	return (NULL);
}

int	philosopher_eat(t_philo *philo)
{
	long	current_time;

	if (is_simulation_stopped(philo->data))
		return (1);
	if (!take_forks(philo))
		return (1);
	current_time = get_elapsed_time(philo->data);
	pthread_mutex_lock(&philo->data->data_lock);
	philo->last_meal_time = current_time;
	pthread_mutex_unlock(&philo->data->data_lock);
	print_state(philo, "is eating");
	precise_sleep(philo->data->time_to_eat);
	pthread_mutex_lock(&philo->data->data_lock);
	philo->meals_eaten++;
	pthread_mutex_unlock(&philo->data->data_lock);
	release_forks(philo);
	return (0);
}

int	philosopher_sleep(t_philo *philo)
{
	if (is_simulation_stopped(philo->data))
		return (1);
	print_state(philo, "is sleeping");
	precise_sleep(philo->data->time_to_sleep);
	return (0);
}

int	philosopher_think(t_philo *philo)
{
	long	think_time;

	if (is_simulation_stopped(philo->data))
		return (1);
	print_state(philo, "is thinking");
	think_time = 0;
	if (philo->data->time_to_die > philo->data->time_to_eat + 
		philo->data->time_to_sleep)
	{
		think_time = (philo->data->time_to_die - philo->data->time_to_eat - 
			philo->data->time_to_sleep) / 2;
		if (think_time > 0)
			precise_sleep(think_time);
	}
	return (0);
}

void	*philosopher_routine(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	pthread_mutex_lock(&philo->data->data_lock);
	philo->last_meal_time = 0;
	pthread_mutex_unlock(&philo->data->data_lock);
	print_state(philo, "is thinking");
	if (philo->id % 2 != 0)
		usleep(1000);
	while (!is_simulation_stopped(philo->data))
	{
		if (philosopher_eat(philo))
			break;
		if (philosopher_sleep(philo))
			break;
		if (philosopher_think(philo))
			break;
	}
	return (NULL);
}
