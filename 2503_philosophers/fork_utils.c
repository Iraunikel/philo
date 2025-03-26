/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fork_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 21:30:00 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/26 14:12:25 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** Handle the special case of a single philosopher
*/
int	handle_single_philosopher(t_philo *philo)
{
	// Take the only fork
	pthread_mutex_lock(&philo->data->forks[philo->left_fork]);
	print_state(philo, "has taken a fork");
	
	// Wait until simulation is stopped or time_to_die passes
	while (!is_simulation_stopped(philo->data))
	{
		// Since there's only one fork, philosopher will die after time_to_die
		if (get_sim_time(philo->data) >= philo->data->time_to_die)
			break;
		usleep(1000);
	}
	
	// Release fork
	pthread_mutex_unlock(&philo->data->forks[philo->left_fork]);
	return (0);
}

/*
** Take forks with deadlock prevention
*/
int	take_forks(t_philo *philo)
{
	int	first_fork;
	int	second_fork;

	// Special case for single philosopher
	if (philo->data->philo_count == 1)
		return (handle_single_philosopher(philo));
	
	// Check if simulation has stopped
	if (is_simulation_stopped(philo->data))
		return (0);
		
	// Always take forks in ascending order to prevent deadlocks
	// Even-numbered philosophers take left fork first, odd take right fork first
	if (philo->id % 2 == 0)
	{
		first_fork = philo->left_fork;
		second_fork = philo->right_fork;
	}
	else
	{
		first_fork = philo->right_fork;
		second_fork = philo->left_fork;
	}
	
	// Take first fork
	pthread_mutex_lock(&philo->data->forks[first_fork]);
	
	// Check if simulation was stopped while waiting for first fork
	if (is_simulation_stopped(philo->data))
	{
		pthread_mutex_unlock(&philo->data->forks[first_fork]);
		return (0);
	}
	
	// Print fork taken message
	print_state(philo, "has taken a fork");
	
	// Take second fork
	pthread_mutex_lock(&philo->data->forks[second_fork]);
	
	// Check if simulation was stopped while waiting for second fork
	if (is_simulation_stopped(philo->data))
	{
		pthread_mutex_unlock(&philo->data->forks[second_fork]);
		pthread_mutex_unlock(&philo->data->forks[first_fork]);
		return (0);
	}
	
	// Print fork taken message
	print_state(philo, "has taken a fork");
	
	return (1);
}

/*
** Release forks safely
*/
void	release_forks(t_philo *philo)
{
	int	first_fork;
	int	second_fork;

	// Nothing to do for a single philosopher (handled in take_forks)
	if (philo->data->philo_count == 1)
		return;
		
	// Release forks in reverse order of acquisition
	if (philo->id % 2 == 0)
	{
		first_fork = philo->left_fork;
		second_fork = philo->right_fork;
	}
	else
	{
		first_fork = philo->right_fork;
		second_fork = philo->left_fork;
	}
	
	pthread_mutex_unlock(&philo->data->forks[second_fork]);
	pthread_mutex_unlock(&philo->data->forks[first_fork]);
} 