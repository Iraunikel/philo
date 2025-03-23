/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:08:54 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/23 14:13:14 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

void	precise_sleep(int ms)
{
	long	start;

	start = get_time();
	while ((get_time() - start) < ms)
		usleep(500);
}

int	take_forks(t_philo *philo)
{
	if (philo->data->philo_count == 1)
	{
		pthread_mutex_lock(&philo->forks[philo->left_fork]);
		print_state(philo, "has taken a fork");
		precise_sleep(philo->data->time_to_die + 10);
		pthread_mutex_unlock(&philo->forks[philo->left_fork]);
		return (0);
	}
	pthread_mutex_lock(&philo->forks[philo->left_fork]);
	print_state(philo, "has taken a fork");
	pthread_mutex_lock(&philo->forks[philo->right_fork]);
	print_state(philo, "has taken a fork");
	return (1);
}

void	release_forks(t_philo *philo)
{
	pthread_mutex_unlock(&philo->forks[philo->left_fork]);
	pthread_mutex_unlock(&philo->forks[philo->right_fork]);
}

static int	philosopher_eat(t_philo *philo)
{
	if (!take_forks(philo))
		return (0);
	pthread_mutex_lock(&philo->meal_lock);
	philo->last_meal_time = get_time();
	philo->meals_eaten++;
	pthread_mutex_unlock(&philo->meal_lock);
	print_state(philo, "is eating");
	precise_sleep(philo->data->time_to_eat);
	release_forks(philo);
	return (1);
}

void	*philosopher_routine(void *arg)
{
	t_philo	*philo;

	philo = (t_philo *)arg;
	pthread_mutex_lock(&philo->meal_lock);
	philo->last_meal_time = get_time();
	pthread_mutex_unlock(&philo->meal_lock);
	if (philo->id % 2)
		precise_sleep(2);
	while (!philo->data->simulation_stop)
	{
		print_state(philo, "is thinking");
		if (!philosopher_eat(philo))
			break ;
		print_state(philo, "is sleeping");
		precise_sleep(philo->data->time_to_sleep);
		if (philo->data->meals_to_eat != -1
			&& philo->meals_eaten >= philo->data->meals_to_eat)
			break ;
	}
	return (NULL);
}
