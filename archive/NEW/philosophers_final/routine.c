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
	pthread_mutex_lock(&philo->data->global_lock);
	pthread_mutex_lock(&philo->forks[philo->left_fork]);
	print_state(philo, "has taken a fork");
	pthread_mutex_unlock(&philo->forks[philo->left_fork]);
	pthread_mutex_unlock(&philo->data->global_lock);
	
	while (!is_simulation_stopped(philo->data))
		usleep(1000);
	return (0);
}

static int	take_first_fork(t_philo *philo, int first_fork)
{
	pthread_mutex_lock(&philo->data->global_lock);
	pthread_mutex_lock(&philo->forks[first_fork]);
	
	if (is_simulation_stopped(philo->data))
	{
		pthread_mutex_unlock(&philo->forks[first_fork]);
		pthread_mutex_unlock(&philo->data->global_lock);
		return (0);
	}
	
	print_state(philo, "has taken a fork");
	pthread_mutex_unlock(&philo->data->global_lock);
	return (1);
}

static int	take_second_fork(t_philo *philo, int first_fork, int second_fork)
{
	pthread_mutex_lock(&philo->data->global_lock);
	pthread_mutex_lock(&philo->forks[second_fork]);
	
	if (is_simulation_stopped(philo->data))
	{
		pthread_mutex_unlock(&philo->forks[second_fork]);
		pthread_mutex_unlock(&philo->forks[first_fork]);
		pthread_mutex_unlock(&philo->data->global_lock);
		return (0);
	}
	
	print_state(philo, "has taken a fork");
	pthread_mutex_unlock(&philo->data->global_lock);
	return (1);
}

int	take_forks(t_philo *philo)
{
	int	first_fork;
	int	second_fork;

	if (philo->data->philo_count == 1)
		return (handle_single_philosopher(philo));
	
	if (is_simulation_stopped(philo->data))
		return (0);
		
	first_fork = philo->left_fork;
	second_fork = philo->right_fork;
	
	// Always lock forks in ascending order to prevent deadlocks
	if (first_fork > second_fork)
	{
		int temp = first_fork;
		first_fork = second_fork;
		second_fork = temp;
	}
	
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

	first_fork = philo->left_fork;
	second_fork = philo->right_fork;
	
	// Always unlock in reverse order of locking
	if (first_fork > second_fork)
	{
		int temp = first_fork;
		first_fork = second_fork;
		second_fork = temp;
	}
	
	pthread_mutex_unlock(&philo->forks[second_fork]);
	pthread_mutex_unlock(&philo->forks[first_fork]);
}

static int	update_meal_time(t_philo *philo)
{
	long	start_eating;

	start_eating = get_time();
	update_last_meal_time(philo, start_eating);
	print_state(philo, "is eating");
	
	while ((get_time() - start_eating) < philo->data->time_to_eat)
	{
		if (is_simulation_stopped(philo->data))
		{
			release_forks(philo);
			return (0);
		}
		usleep(1000);
	}
	return (1);
}

static int	update_meal_count(t_philo *philo)
{
	int	current_meals;
	bool	should_continue;

	increment_meals_eaten(philo);
	current_meals = get_meals_eaten(philo);
	
	if (!is_simulation_stopped(philo->data))
	{
		pthread_mutex_lock(&philo->data->global_lock);
		pthread_mutex_lock(&philo->data->print_lock);
		printf("%lld %d finished eating meal %d/%d\n", 
			get_time() - philo->data->start_time, 
			philo->id, current_meals, philo->data->meals_to_eat);
		pthread_mutex_unlock(&philo->data->print_lock);
		pthread_mutex_unlock(&philo->data->global_lock);
	}
	
	release_forks(philo);
	should_continue = true;
	if (philo->data->meals_to_eat != -1 && 
		current_meals >= philo->data->meals_to_eat)
		should_continue = false;
	return (should_continue);
}

static int	philosopher_eat(t_philo *philo)
{
	int	current_meals;

	if (is_simulation_stopped(philo->data))
		return (0);
		
	current_meals = get_meals_eaten(philo);
	
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
	int	current_meals;
	int	target_meals;

	if (is_simulation_stopped(philo->data))
		return (1);
		
	target_meals = philo->data->meals_to_eat;
	current_meals = get_meals_eaten(philo);
	
	if (target_meals != -1 && current_meals >= target_meals)
	{
		pthread_mutex_lock(&philo->data->global_lock);
		pthread_mutex_lock(&philo->data->print_lock);
		printf("%lld %d has eaten enough (%d meals)\n", 
			get_time() - philo->data->start_time, philo->id, current_meals);
		pthread_mutex_unlock(&philo->data->print_lock);
		pthread_mutex_unlock(&philo->data->global_lock);
		return (1);
	}
	return (0);
}

static void	init_philosopher(t_philo *philo)
{
	int	target_meals;

	target_meals = philo->data->meals_to_eat;
	update_last_meal_time(philo, get_time());
	
	// Stagger philosopher start times to reduce contention
	if (philo->id % 2)
		usleep(1000);
		
	pthread_mutex_lock(&philo->data->global_lock);
	pthread_mutex_lock(&philo->data->print_lock);
	printf("Philosopher %d starting. Target: %d meals\n", 
		philo->id, target_meals);
	pthread_mutex_unlock(&philo->data->print_lock);
	pthread_mutex_unlock(&philo->data->global_lock);
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
	
	pthread_mutex_lock(&philo->data->global_lock);
	pthread_mutex_lock(&philo->data->print_lock);
	printf("Philosopher %d exiting\n", philo->id);
	pthread_mutex_unlock(&philo->data->print_lock);
	pthread_mutex_unlock(&philo->data->global_lock);
	return (NULL);
}
