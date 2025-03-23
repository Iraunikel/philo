/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:08:54 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/23 13:50:38 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

void	precise_sleep(int ms)
{
	long	start;
	long	elapsed;

	start = get_time();
	
	while (1)
	{
		elapsed = get_time() - start;
		if (elapsed >= ms)
			break;
			
		// Adaptive sleep - shorter when close to target time
		usleep(elapsed > ms - 10 ? 500 : 1000);
	}
}

// Handle the single philosopher case
void	handle_one_philo(t_philo *philo)
{
	long	time_to_die;
	
	// Get time to die parameter
	pthread_mutex_lock(&philo->data->death_lock);
	time_to_die = philo->data->time_to_die;
	pthread_mutex_unlock(&philo->data->death_lock);
	
	// Take the only fork available
	pthread_mutex_lock(&philo->forks[philo->left_fork]);
	
	// Set initial meal time and print state atomically 
	pthread_mutex_lock(&philo->meal_lock);
	philo->last_meal_time = get_time();
	pthread_mutex_unlock(&philo->meal_lock);
	
	print_state(philo, "has taken a fork");
	
	// Wait until time to die - fork is held during sleep
	precise_sleep(time_to_die);
	
	// Release the fork before returning
	pthread_mutex_unlock(&philo->forks[philo->left_fork]);
}

int	take_fork(t_philo *philo, int fork_id)
{
	int	should_stop;

	// Check if we should stop
	pthread_mutex_lock(&philo->data->death_lock);
	should_stop = philo->data->simulation_stop;
	pthread_mutex_unlock(&philo->data->death_lock);

	if (should_stop)
		return (0);

	// Take the fork
	pthread_mutex_lock(&philo->forks[fork_id]);
	
	// Double-check if we should stop after acquiring the lock
	pthread_mutex_lock(&philo->data->death_lock);
	should_stop = philo->data->simulation_stop;
	pthread_mutex_unlock(&philo->data->death_lock);
	
	if (should_stop)
	{
		pthread_mutex_unlock(&philo->forks[fork_id]);
		return (0);
	}
	
	print_state(philo, "has taken a fork");
	return (1);
}

int	take_forks(t_philo *philo)
{
	int	should_stop;

	// Случай одного философа
	if (philo->data->philo_count == 1)
		return (0);

	// Проверка должны ли мы остановиться
	pthread_mutex_lock(&philo->data->death_lock);
	should_stop = philo->data->simulation_stop;
	pthread_mutex_unlock(&philo->data->death_lock);

	if (should_stop)
		return (0);

	// Блокируем вилки
	if (philo->id % 2 == 0)
	{
		pthread_mutex_lock(&philo->forks[philo->right_fork]);
		print_state(philo, "has taken a fork");
		pthread_mutex_lock(&philo->forks[philo->left_fork]);
		print_state(philo, "has taken a fork");
	}
	else
	{
		pthread_mutex_lock(&philo->forks[philo->left_fork]);
		print_state(philo, "has taken a fork");
		pthread_mutex_lock(&philo->forks[philo->right_fork]);
		print_state(philo, "has taken a fork");
	}
	
	return (1);
}

void	release_forks(t_philo *philo)
{
	// Разблокируем обе вилки в произвольном порядке
	pthread_mutex_unlock(&philo->forks[philo->left_fork]);
	pthread_mutex_unlock(&philo->forks[philo->right_fork]);
}

int	philosopher_eat(t_philo *philo)
{
	long	eat_time;

	// Try to take forks, return 0 if failed
	if (!take_forks(philo))
		return (0);

	// Get eat time
	pthread_mutex_lock(&philo->data->death_lock);
	eat_time = philo->data->time_to_eat;
	pthread_mutex_unlock(&philo->data->death_lock);

	// Now that we have both forks, update last meal time and increment meals counter
	pthread_mutex_lock(&philo->meal_lock);
	philo->last_meal_time = get_time();
	philo->meals_eaten++;
	pthread_mutex_unlock(&philo->meal_lock);

	// Eating state
	print_state(philo, "is eating");

	// Sleep for eat_time while holding the forks
	precise_sleep(eat_time);

	// Release forks directly
	pthread_mutex_unlock(&philo->forks[philo->left_fork]);
	pthread_mutex_unlock(&philo->forks[philo->right_fork]);

	return (1);
}

void	*philosopher_routine(void *arg)
{
	t_philo	*philo;
	long	sleep_time;
	int		meals_to_eat;

	philo = (t_philo *)arg;
	
	// Set initial last_meal_time
	pthread_mutex_lock(&philo->meal_lock);
	philo->last_meal_time = get_time();
	pthread_mutex_unlock(&philo->meal_lock);
	
	// Get sleep time and meals_to_eat parameters
	pthread_mutex_lock(&philo->data->death_lock);
	sleep_time = philo->data->time_to_sleep;
	meals_to_eat = philo->data->meals_to_eat;
	pthread_mutex_unlock(&philo->data->death_lock);
	
	// Handle special case for a single philosopher
	if (philo->data->philo_count == 1)
	{
		handle_one_philo(philo);
		return (NULL);
	}
	
	// Stagger philosopher start times based on ID
	if (philo->id % 2 == 0)
		precise_sleep(20);  // Even philosophers wait a bit
	else
		precise_sleep(10);  // Odd philosophers wait less
	
	// Main simulation loop
	while (1)
	{
		int should_stop;
		int meals_eaten;
		
		// Check if simulation should stop
		pthread_mutex_lock(&philo->data->death_lock);
		should_stop = philo->data->simulation_stop;
		pthread_mutex_unlock(&philo->data->death_lock);
		
		if (should_stop)
			break;
		
		// Check if we've eaten enough meals
		if (meals_to_eat != -1)
		{
			pthread_mutex_lock(&philo->meal_lock);
			meals_eaten = philo->meals_eaten;
			pthread_mutex_unlock(&philo->meal_lock);
			
			if (meals_eaten >= meals_to_eat)
				break;
		}
		
		// Try to eat
		if (!philosopher_eat(philo))
			break;
		
		// Sleeping state
		print_state(philo, "is sleeping");
		precise_sleep(sleep_time);
		
		// Thinking state
		print_state(philo, "is thinking");
		
		// Short pause while thinking to avoid CPU spinning
		usleep(500);
	}
	return (NULL);
}
