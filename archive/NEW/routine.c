/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:08:54 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/24 16:10:50 by iunikel          ###   ########.fr       */
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
		
		// Sleep for shorter intervals to improve accuracy
		// without consuming too much CPU
		if (ms - elapsed > 5)
			usleep(1000);
		else
			usleep(100);
	}
}

int	take_forks(t_philo *philo)
{
	t_data	*data;
	int		stop;

	data = philo->data;
	
	// Special case for a single philosopher (will die)
	if (data->philo_count == 1)
	{
		pthread_mutex_lock(&philo->forks[philo->left_fork]);
		print_state(philo, "has taken a fork");
		pthread_mutex_unlock(&philo->forks[philo->left_fork]);
		
		// Wait until death
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
	
	// Check if simulation has stopped before proceeding
	pthread_mutex_lock(&data->death_lock);
	stop = data->simulation_stop;
	pthread_mutex_unlock(&data->death_lock);
	
	if (stop)
		return (0);
	
	// Implement better fork taking strategy:
	// Even philosophers take right fork first, odd take left first
	// This prevents circular wait deadlocks
	if (philo->id % 2 == 0)
	{
		pthread_mutex_lock(&philo->forks[philo->right_fork]);
		print_state(philo, "has taken a fork");
		pthread_mutex_lock(&philo->forks[philo->left_fork]);
		print_state(philo, "has taken a fork");
	}
	else
	{
		// Add small delay for odd philosophers to reduce contention
		usleep(100);
		pthread_mutex_lock(&philo->forks[philo->left_fork]);
		print_state(philo, "has taken a fork");
		pthread_mutex_lock(&philo->forks[philo->right_fork]);
		print_state(philo, "has taken a fork");
	}
	
	// Check again if simulation stopped while taking forks
	pthread_mutex_lock(&data->death_lock);
	stop = data->simulation_stop;
	pthread_mutex_unlock(&data->death_lock);
	
	if (stop)
	{
		release_forks(philo);
		return (0);
	}
	
	return (1);
}

void	release_forks(t_philo *philo)
{
	pthread_mutex_unlock(&philo->forks[philo->left_fork]);
	pthread_mutex_unlock(&philo->forks[philo->right_fork]);
}

static int	philosopher_eat(t_philo *philo)
{
	int should_continue;
	int current_meals;
	int stop;
	long long start_eating;
	long long current_time;
	
	// Check if simulation already stopped
	pthread_mutex_lock(&philo->data->death_lock);
	stop = philo->data->simulation_stop;
	pthread_mutex_unlock(&philo->data->death_lock);
	
	if (stop)
		return (0);
	
	// Try to take forks
	if (!take_forks(philo))
		return (0);
	
	// Record eating start time
	start_eating = get_time();
	
	// Update the last_meal_time IMMEDIATELY to avoid death detection issues
	pthread_mutex_lock(&philo->meal_lock);
	philo->last_meal_time = start_eating;
	pthread_mutex_unlock(&philo->meal_lock);
	
	// Print eating state
	print_state(philo, "is eating");
	
	// Eat for the required time, but check frequently for stop signals
	// This ensures fast response to deaths or simulation stop
	current_time = start_eating;
	while ((current_time - start_eating) < philo->data->time_to_eat)
	{
		// Check if simulation should stop during eating
		pthread_mutex_lock(&philo->data->death_lock);
		stop = philo->data->simulation_stop;
		pthread_mutex_unlock(&philo->data->death_lock);
		
		if (stop)
		{
			// Release forks and exit if simulation stopped
			release_forks(philo);
			return (0);
		}
			
		// Sleep in small increments for better responsiveness
		usleep(500);
		current_time = get_time();
	}
	
	// Increment meal count after completing the full eating duration
	pthread_mutex_lock(&philo->meal_lock);
	philo->meals_eaten++;
	current_meals = philo->meals_eaten;
	pthread_mutex_unlock(&philo->meal_lock);
	
	// Debug message to track meal counts
	pthread_mutex_lock(&philo->data->print_lock);
	printf("%lld %d completed meal %d/%d\n", 
		get_time() - philo->data->start_time, 
		philo->id, current_meals, philo->data->meals_to_eat);
	pthread_mutex_unlock(&philo->data->print_lock);
	
	// Release forks
	release_forks(philo);
	
	// Check if we've reached the meal limit
	should_continue = 1;
	if (philo->data->meals_to_eat != -1 && 
		current_meals >= philo->data->meals_to_eat)
	{
		should_continue = 0;
		
		// Print that this philosopher has finished eating
		pthread_mutex_lock(&philo->data->print_lock);
		printf("%lld %d has finished all %d meals\n", 
			get_time() - philo->data->start_time, 
			philo->id, philo->data->meals_to_eat);
		pthread_mutex_unlock(&philo->data->print_lock);
	}
	
	return (should_continue);
}

void	*philosopher_routine(void *arg)
{
	t_philo	*philo;
	int		stop;
	int		current_meals;
	long    start_time;
	int     target_meals;
	int     time_to_sleep;

	philo = (t_philo *)arg;
	target_meals = philo->data->meals_to_eat;
	time_to_sleep = philo->data->time_to_sleep;
	
	// Initialize last meal time at start
	pthread_mutex_lock(&philo->meal_lock);
	philo->last_meal_time = get_time();
	pthread_mutex_unlock(&philo->meal_lock);
	
	// Stagger philosopher start times to reduce contention
	if (philo->id % 2)
		usleep(100);
	
	// Debug message about starting
	pthread_mutex_lock(&philo->data->print_lock);
	printf("Philosopher %d starting. Target: %d meals\n", 
		philo->id, target_meals);
	pthread_mutex_unlock(&philo->data->print_lock);
	
	stop = 0;
	while (!stop)
	{
		// Check if simulation should stop
		pthread_mutex_lock(&philo->data->death_lock);
		stop = philo->data->simulation_stop;
		pthread_mutex_unlock(&philo->data->death_lock);
		
		if (stop)
			break;
		
		// Get current meal count
		pthread_mutex_lock(&philo->meal_lock);
		current_meals = philo->meals_eaten;
		pthread_mutex_unlock(&philo->meal_lock);
		
		// Exit if we've eaten enough
		if (target_meals != -1 && current_meals >= target_meals)
		{
			pthread_mutex_lock(&philo->data->print_lock);
			printf("%lld %d has eaten enough (%d meals)\n", 
				get_time() - philo->data->start_time, philo->id, current_meals);
			pthread_mutex_unlock(&philo->data->print_lock);
			break;
		}
		
		// Think briefly
		print_state(philo, "is thinking");
		
		// Try to eat (returns 0 if we've eaten enough or should stop)
		if (!philosopher_eat(philo))
		{
			// Check if simulation has been stopped
			pthread_mutex_lock(&philo->data->death_lock);
			stop = philo->data->simulation_stop;
			pthread_mutex_unlock(&philo->data->death_lock);
			
			if (stop)
				break;
			
			// Get updated meal count to see if we've eaten enough
			pthread_mutex_lock(&philo->meal_lock);
			current_meals = philo->meals_eaten;
			pthread_mutex_unlock(&philo->meal_lock);
			
			// If we've reached our meal target, print that we've eaten enough
			if (target_meals != -1 && current_meals >= target_meals)
			{
				pthread_mutex_lock(&philo->data->print_lock);
				printf("%lld %d has eaten enough (%d meals)\n", 
					get_time() - philo->data->start_time, philo->id, current_meals);
				pthread_mutex_unlock(&philo->data->print_lock);
			}
			
			// Exit the philosopher routine
			break;
		}
		
		// Check if we should stop after eating
		pthread_mutex_lock(&philo->data->death_lock);
		stop = philo->data->simulation_stop;
		pthread_mutex_unlock(&philo->data->death_lock);
		
		if (stop)
			break;
		
		// Sleep after eating
		print_state(philo, "is sleeping");
		
		// Check if we should stop during sleeping with MORE FREQUENT checks
		start_time = get_time();
		while ((get_time() - start_time) < time_to_sleep)
		{
			// Check for simulation stop
			pthread_mutex_lock(&philo->data->death_lock);
			stop = philo->data->simulation_stop;
			pthread_mutex_unlock(&philo->data->death_lock);
			
			if (stop)
				break;
				
			// Sleep in very small increments for better responsiveness to deaths
			usleep(50);
		}
	}
	
	// Only print exit message if there's a meal count and simulation isn't stopped due to death
	pthread_mutex_lock(&philo->data->death_lock);
	stop = philo->data->simulation_stop; 
	pthread_mutex_unlock(&philo->data->death_lock);
	
	if (target_meals != -1 && !stop)
	{
		pthread_mutex_lock(&philo->meal_lock);
		current_meals = philo->meals_eaten;
		pthread_mutex_unlock(&philo->meal_lock);
		
		pthread_mutex_lock(&philo->data->print_lock);
		printf("Philosopher %d exiting with %d/%d meals eaten\n", 
			philo->id, current_meals, target_meals);
		pthread_mutex_unlock(&philo->data->print_lock);
	}
	
	return (NULL);
}
