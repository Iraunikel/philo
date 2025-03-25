/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fork_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 21:30:00 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/25 21:56:13 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/*
** Handle the special case of a single philosopher
*/
int	handle_single_philosopher(t_philo *philo)
{
	pthread_mutex_lock(&philo->data->forks[philo->left_fork]);
	print_state(philo, "has taken a fork");
	
	// Stay in thinking state until simulation stops
	while (!is_simulation_stopped(philo->data))
		usleep(1000);
	
	pthread_mutex_unlock(&philo->data->forks[philo->left_fork]);
	return (0);
}

/*
** Take forks in a deterministic order to prevent deadlocks
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
		
	// Always lock forks in ascending order to prevent deadlocks
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
	
	// Take first fork with proper error checking
	pthread_mutex_lock(&philo->data->forks[first_fork]);
	if (is_simulation_stopped(philo->data))
	{
		pthread_mutex_unlock(&philo->data->forks[first_fork]);
		return (0);
	}
	print_state(philo, "has taken a fork");
	
	// Take second fork with proper error checking
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

/*
** Release forks in reverse order of acquisition
*/
void	release_forks(t_philo *philo)
{
	int	first_fork;
	int	second_fork;

	// Nothing to do for a single philosopher
	if (philo->data->philo_count == 1)
		return;
		
	// Determine fork order (same as in take_forks)
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
	
	// Release in reverse order of acquisition
	pthread_mutex_unlock(&philo->data->forks[second_fork]);
	pthread_mutex_unlock(&philo->data->forks[first_fork]);
} 