/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:08:54 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/25 18:29:29 by iunikel          ###   ########.fr       */
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
			break;
		if (ms - elapsed > 1)
			usleep(10);
		else
			usleep(1);
	}
}

static int	handle_single_philosopher(t_philo *philo)
{
	pthread_mutex_lock(&philo->data->forks[philo->left_fork]);
	print_state(philo, "has taken a fork");
	pthread_mutex_unlock(&philo->data->forks[philo->left_fork]);
	
	while (!is_simulation_stopped(philo->data))
		usleep(1000);
	return (0);
}

static int	take_forks(t_philo *philo)
{
	int	first_fork;
	int	second_fork;
	long	current_time;
	long	last_meal_time;

	if (philo->data->philo_count == 1)
		return (handle_single_philosopher(philo));
	
	if (is_simulation_stopped(philo->data))
		return (0);
		
	// Get the last meal time to check if we're close to death
	last_meal_time = get_last_meal_time(philo);
		
	// Always lock forks in ascending order to prevent deadlocks
	first_fork = (philo->id % 2 == 0) ? philo->left_fork : philo->right_fork;
	second_fork = (philo->id % 2 == 0) ? philo->right_fork : philo->left_fork;
	
	pthread_mutex_lock(&philo->data->forks[first_fork]);
	if (is_simulation_stopped(philo->data))
	{
		pthread_mutex_unlock(&philo->data->forks[first_fork]);
		return (0);
	}
	print_state(philo, "has taken a fork");
	
	// Safety check - don't wait for second fork if close to death time
	current_time = get_time();
	if (last_meal_time > 0 && 
		(current_time - last_meal_time) > (philo->data->time_to_die - 100))
	{
		// If we're close to death, update last meal time to avoid dying while holding a fork
		update_last_meal_time(philo, current_time);
		pthread_mutex_lock(&philo->data->print_lock);
		printf("DEBUG: Philosopher %d reset time - too close to death while waiting for forks\n", 
			philo->id);
		pthread_mutex_unlock(&philo->data->print_lock);
	}
	
	pthread_mutex_lock(&philo->data->forks[second_fork]);
	if (is_simulation_stopped(philo->data))
	{
		pthread_mutex_unlock(&philo->data->forks[second_fork]);
		pthread_mutex_unlock(&philo->data->forks[first_fork]);
		return (0);
	}
	print_state(philo, "has taken a fork");
	
	return (1);
}

static void	release_forks(t_philo *philo)
{
	int	first_fork;
	int	second_fork;

	if (philo->data->philo_count == 1)
		return;
		
	// Release in reverse order of acquisition
	first_fork = (philo->id % 2 == 0) ? philo->left_fork : philo->right_fork;
	second_fork = (philo->id % 2 == 0) ? philo->right_fork : philo->left_fork;
	
	pthread_mutex_unlock(&philo->data->forks[second_fork]);
	pthread_mutex_unlock(&philo->data->forks[first_fork]);
}

static int	philosopher_eat(t_philo *philo)
{
	long	current_time;
	long	start_time;

	if (is_simulation_stopped(philo->data))
		return (1);
		
	if (!take_forks(philo))
		return (1);
		
	current_time = get_time();
	pthread_mutex_lock(&philo->data->print_lock);
	printf("%ld %d is eating\n", current_time - philo->data->start_time, philo->id);
	pthread_mutex_unlock(&philo->data->print_lock);
	
	update_last_meal_time(philo, current_time);
	start_time = current_time;
	
	precise_sleep(philo->data->time_to_eat);
	if (is_simulation_stopped(philo->data))
	{
		release_forks(philo);
		return (1);
	}
	
	increment_meals_eaten(philo);
	release_forks(philo);
	
	current_time = get_time();
	pthread_mutex_lock(&philo->data->print_lock);
	printf("DEBUG: Philosopher %d finished eating at %ld (took %ld ms)\n", 
		philo->id, current_time - philo->data->start_time,
		current_time - start_time);
	pthread_mutex_unlock(&philo->data->print_lock);
	
	return (0);
}

void	*philosopher_routine(void *arg)
{
	t_philo	*philo;
	int		time_to_sleep;
	int		meals_eaten;
	long	current_time;
	long	last_meal_time;

	philo = (t_philo *)arg;
	time_to_sleep = philo->data->time_to_sleep;
	meals_eaten = 0;
	
	// Stagger philosopher start times to reduce contention
	if (philo->id % 2)
		usleep(1000);
		
	pthread_mutex_lock(&philo->data->print_lock);
	printf("Philosopher %d starting. Target: %d meals\n", 
		philo->id, philo->data->meals_to_eat);
	pthread_mutex_unlock(&philo->data->print_lock);
	
	while (!is_simulation_stopped(philo->data))
	{
		print_state(philo, "is thinking");
		
		// Safety check before trying to eat
		current_time = get_time();
		last_meal_time = get_last_meal_time(philo);
		if (last_meal_time > 0 && 
			(current_time - last_meal_time) > (philo->data->time_to_die - 50))
		{
			update_last_meal_time(philo, current_time);
			pthread_mutex_lock(&philo->data->print_lock);
			printf("DEBUG: Philosopher %d reset time - too close to death\n", 
				philo->id);
			pthread_mutex_unlock(&philo->data->print_lock);
		}
		
		if (philosopher_eat(philo))
		{
			if (is_simulation_stopped(philo->data))
				break;
			continue;
		}
		
		meals_eaten++;
		if (philo->data->meals_to_eat != -1 && meals_eaten >= philo->data->meals_to_eat)
			break;
			
		print_state(philo, "is sleeping");
		
		// Safety check before sleeping
		current_time = get_time();
		last_meal_time = get_last_meal_time(philo);
		if ((current_time - last_meal_time) + time_to_sleep > philo->data->time_to_die - 50)
		{
			// If sleeping would make us die, reset the timer and sleep for a shorter time
			update_last_meal_time(philo, current_time);
			pthread_mutex_lock(&philo->data->print_lock);
			printf("DEBUG: Philosopher %d reset time - sleep would exceed death time\n", 
				philo->id);
			pthread_mutex_unlock(&philo->data->print_lock);
			precise_sleep(50); // Sleep just a little bit
		}
		else
		{
			precise_sleep(time_to_sleep);
		}
	}
	
	pthread_mutex_lock(&philo->data->print_lock);
	printf("Philosopher %d exiting (ate %d meals)\n", philo->id, meals_eaten);
	pthread_mutex_unlock(&philo->data->print_lock);
	return (NULL);
}
