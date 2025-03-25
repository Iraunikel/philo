/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:08:54 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/24 11:47:30 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

void	precise_sleep(int ms)
{
	long	start;
	long	current;
	long	elapsed;

	start = get_time();
	while (1)
	{
		current = get_time();
		elapsed = current - start;
		if (elapsed >= ms)
			break ;
		if (ms - elapsed > 5)
			usleep(1000);
		else
			usleep(100);
	}
}

static int	handle_single_philosopher(t_philo *philo)
{
	t_data	*data;
	int		stop;

	data = philo->data;
	pthread_mutex_lock(&philo->forks[philo->left_fork]);
	print_state(philo, "has taken a fork");
	pthread_mutex_unlock(&philo->forks[philo->left_fork]);
	while (1)
	{
		pthread_mutex_lock(&data->death_lock);
		stop = data->simulation_stop;
		pthread_mutex_unlock(&data->death_lock);
		if (stop)
			return (0);
		usleep(100);
	}
	return (0);
}

static int	take_first_fork(t_philo *philo, int first_fork)
{
	int	stop;

	pthread_mutex_lock(&philo->forks[first_fork]);
	pthread_mutex_lock(&philo->data->death_lock);
	stop = philo->data->simulation_stop;
	pthread_mutex_unlock(&philo->data->death_lock);
	if (stop)
	{
		pthread_mutex_unlock(&philo->forks[first_fork]);
		return (0);
	}
	print_state(philo, "has taken a fork");
	return (1);
}

static int	take_second_fork(t_philo *philo, int first_fork, int second_fork)
{
	int	stop;

	pthread_mutex_lock(&philo->forks[second_fork]);
	pthread_mutex_lock(&philo->data->death_lock);
	stop = philo->data->simulation_stop;
	pthread_mutex_unlock(&philo->data->death_lock);
	if (stop)
	{
		pthread_mutex_unlock(&philo->forks[second_fork]);
		pthread_mutex_unlock(&philo->forks[first_fork]);
		return (0);
	}
	print_state(philo, "has taken a fork");
	return (1);
}

int	take_forks(t_philo *philo)
{
	t_data	*data;
	int		stop;
	int		first_fork;
	int		second_fork;

	data = philo->data;
	if (data->philo_count == 1)
		return (handle_single_philosopher(philo));
	pthread_mutex_lock(&data->death_lock);
	stop = data->simulation_stop;
	pthread_mutex_unlock(&data->death_lock);
	if (stop)
		return (0);
	first_fork = (philo->left_fork < philo->right_fork) ? 
		philo->left_fork : philo->right_fork;
	second_fork = (philo->left_fork < philo->right_fork) ? 
		philo->right_fork : philo->left_fork;
	if (!take_first_fork(philo, first_fork))
		return (0);
	if (!take_second_fork(philo, first_fork, second_fork))
		return (0);
	return (1);
}

void	release_forks(t_philo *philo)
{
	int	first_fork;
	int	second_fork;

	first_fork = (philo->left_fork < philo->right_fork) ? 
		philo->left_fork : philo->right_fork;
	second_fork = (philo->left_fork < philo->right_fork) ? 
		philo->right_fork : philo->left_fork;
	pthread_mutex_unlock(&philo->forks[second_fork]);
	pthread_mutex_unlock(&philo->forks[first_fork]);
}

static int	update_meal_time(t_philo *philo)
{
	int		stop;
	long	start_eating;

	start_eating = get_time();
	pthread_mutex_lock(&philo->meal_lock);
	philo->last_meal_time = start_eating;
	pthread_mutex_unlock(&philo->meal_lock);
	print_state(philo, "is eating");
	while ((get_time() - start_eating) < philo->data->time_to_eat)
	{
		pthread_mutex_lock(&philo->data->death_lock);
		stop = philo->data->simulation_stop;
		pthread_mutex_unlock(&philo->data->death_lock);
		if (stop)
		{
			release_forks(philo);
			return (0);
		}
		usleep(100);
	}
	return (1);
}

static int	update_meal_count(t_philo *philo)
{
	int	current_meals;
	int	stop;
	int	should_continue;

	pthread_mutex_lock(&philo->meal_lock);
	philo->meals_eaten++;
	current_meals = philo->meals_eaten;
	pthread_mutex_unlock(&philo->meal_lock);
	pthread_mutex_lock(&philo->data->death_lock);
	stop = philo->data->simulation_stop;
	pthread_mutex_unlock(&philo->data->death_lock);
	if (!stop)
	{
		pthread_mutex_lock(&philo->data->print_lock);
		printf("%lld %d finished eating meal %d/%d\n", 
			get_time() - philo->data->start_time, 
			philo->id, current_meals, philo->data->meals_to_eat);
		pthread_mutex_unlock(&philo->data->print_lock);
	}
	release_forks(philo);
	should_continue = 1;
	if (philo->data->meals_to_eat != -1 && 
		current_meals >= philo->data->meals_to_eat)
		should_continue = 0;
	return (should_continue);
}

static int	philosopher_eat(t_philo *philo)
{
	int	current_meals;
	int	stop;

	pthread_mutex_lock(&philo->data->death_lock);
	stop = philo->data->simulation_stop;
	pthread_mutex_unlock(&philo->data->death_lock);
	if (stop)
		return (0);
	pthread_mutex_lock(&philo->meal_lock);
	current_meals = philo->meals_eaten;
	pthread_mutex_unlock(&philo->meal_lock);
	if (philo->data->meals_to_eat != -1 && 
		current_meals >= philo->data->meals_to_eat)
		return (0);
	if (!take_forks(philo))
		return (0);
	if (!update_meal_time(philo))
		return (0);
	return (update_meal_count(philo));
}

static int	check_exit_conditions(t_philo *philo)
{
	int	stop;
	int	current_meals;
	int	target_meals;

	pthread_mutex_lock(&philo->data->death_lock);
	stop = philo->data->simulation_stop;
	pthread_mutex_unlock(&philo->data->death_lock);
	if (stop)
		return (1);
	target_meals = philo->data->meals_to_eat;
	pthread_mutex_lock(&philo->meal_lock);
	current_meals = philo->meals_eaten;
	pthread_mutex_unlock(&philo->meal_lock);
	if (target_meals != -1 && current_meals >= target_meals)
	{
		pthread_mutex_lock(&philo->data->print_lock);
		printf("%lld %d has eaten enough (%d meals)\n", 
			get_time() - philo->data->start_time, philo->id, current_meals);
		pthread_mutex_unlock(&philo->data->print_lock);
		return (1);
	}
	return (0);
}

static void	init_philosopher(t_philo *philo)
{
	int	target_meals;

	target_meals = philo->data->meals_to_eat;
	pthread_mutex_lock(&philo->meal_lock);
	philo->last_meal_time = get_time();
	pthread_mutex_unlock(&philo->meal_lock);
	if (philo->id % 2)
		usleep(1000);
	pthread_mutex_lock(&philo->data->print_lock);
	printf("Philosopher %d starting. Target: %d meals\n", 
		philo->id, target_meals);
	pthread_mutex_unlock(&philo->data->print_lock);
}

void	*philosopher_routine(void *arg)
{
	t_philo	*philo;
	int		time_to_sleep;

	philo = (t_philo *)arg;
	time_to_sleep = philo->data->time_to_sleep;
	init_philosopher(philo);
	while (!check_exit_conditions(philo))
	{
		print_state(philo, "is thinking");
		if (!philosopher_eat(philo))
		{
			if (check_exit_conditions(philo))
				break ;
		}
		if (check_exit_conditions(philo))
			break ;
		print_state(philo, "is sleeping");
		precise_sleep(time_to_sleep);
	}
	pthread_mutex_lock(&philo->data->print_lock);
	printf("Philosopher %d exiting\n", philo->id);
	pthread_mutex_unlock(&philo->data->print_lock);
	return (NULL);
}
