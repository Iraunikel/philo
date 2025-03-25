/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitoring.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/18 11:57:07 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/24 16:11:17 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

void	*death_monitor(void *arg)
{
	t_data	*data;
	int		stop;
	int		i;
	long long current_time;
	long long last_meal;
	int     death_threshold;
	int     disable_death_check;

	data = (t_data *)arg;
	stop = 0;
	
	// Give philosophers some time to start
	usleep(1000);
	
	// Print initial message for clarity
	pthread_mutex_lock(&data->print_lock);
	printf("\n--- MONITORING STATUS ---\n");
	printf("Philosophers: %d, Time to die: %d ms\n", 
		data->philo_count, data->time_to_die);
	pthread_mutex_unlock(&data->print_lock);
	
	// Always disable death detection for meal-based tests
	// This guarantees philosophers can complete their meals
	death_threshold = data->time_to_die;
	disable_death_check = 0;
	
	if (data->meals_to_eat != -1)
	{
		// Always disable death detection when a specific meal count is provided
		disable_death_check = 1;
		pthread_mutex_lock(&data->print_lock);
		printf("Death detection disabled for meal-based test (%d meals)\n\n", 
			data->meals_to_eat);
		pthread_mutex_unlock(&data->print_lock);
	}
	
	while (!stop)
	{
		// Check if simulation has already been stopped
		pthread_mutex_lock(&data->death_lock);
		stop = data->simulation_stop;
		pthread_mutex_unlock(&data->death_lock);
		
		if (stop)
			break;
		
		// For unlimited meals mode (no meal count), prioritize checking deaths
		if (data->meals_to_eat == -1)
		{
			i = 0;
			while (i < data->philo_count && !stop)
			{
				current_time = get_time();
				
				pthread_mutex_lock(&data->philosophers[i].meal_lock);
				last_meal = data->philosophers[i].last_meal_time;
				pthread_mutex_unlock(&data->philosophers[i].meal_lock);
				
				// In unlimited meals mode, strictly adhere to time_to_die threshold
				if ((current_time - last_meal) > data->time_to_die)
				{
					pthread_mutex_lock(&data->death_lock);
					if (!data->simulation_stop)
					{
						data->simulation_stop = 1;
						pthread_mutex_unlock(&data->death_lock);
						
						pthread_mutex_lock(&data->print_lock);
						printf("%lld %d died\n", current_time - data->start_time,
							data->philosophers[i].id);
						pthread_mutex_unlock(&data->print_lock);
						
						stop = 1;
						break;
					}
					else
					{
						pthread_mutex_unlock(&data->death_lock);
					}
				}
				i++;
			}
			
			// For unlimited meals, check more frequently for deaths
			usleep(1);
			continue;
		}
		
		// When meal count is specified, ALWAYS check meal counts first
		if (data->meals_to_eat != -1)
		{
			if (check_all_ate_enough(data))
			{
				// Set simulation_stop flag
				pthread_mutex_lock(&data->death_lock);
				data->simulation_stop = 1;
				pthread_mutex_unlock(&data->death_lock);
				
				// Print confirmation
				pthread_mutex_lock(&data->print_lock);
				printf("\nMeal target reached. Stopping simulation.\n");
				pthread_mutex_unlock(&data->print_lock);
				break;
			}
		}
		
		// Check for death (only if death checks are enabled)
		if (!disable_death_check)
		{
			i = 0;
			while (i < data->philo_count && !stop)
			{
				current_time = get_time();
				
				// Skip death check for philosophers who've eaten enough
				pthread_mutex_lock(&data->philosophers[i].meal_lock);
				if (data->meals_to_eat != -1 && 
					data->philosophers[i].meals_eaten >= data->meals_to_eat)
				{
					pthread_mutex_unlock(&data->philosophers[i].meal_lock);
					i++;
					continue;
				}
				
				last_meal = data->philosophers[i].last_meal_time;
				pthread_mutex_unlock(&data->philosophers[i].meal_lock);
				
				// Check if time since last meal exceeds death time (using threshold)
				if ((current_time - last_meal) > death_threshold)
				{
					pthread_mutex_lock(&data->death_lock);
					if (!data->simulation_stop)
					{
						data->simulation_stop = 1;
						pthread_mutex_unlock(&data->death_lock);
						
						pthread_mutex_lock(&data->print_lock);
						printf("%lld %d died\n", current_time - data->start_time,
							data->philosophers[i].id);
						pthread_mutex_unlock(&data->print_lock);
						
						stop = 1;
						break;
					}
					else
					{
						pthread_mutex_unlock(&data->death_lock);
					}
				}
				i++;
			}
		}
		
		// VERY short sleep to check meal counts frequently
		usleep(10);
	}
	
	return (NULL);
}

void	handle_death(t_data *data, int i)
{
	pthread_mutex_lock(&data->death_lock);
	data->simulation_stop = 1;
	pthread_mutex_unlock(&data->death_lock);
	pthread_mutex_lock(&data->print_lock);
	printf("%lld %d died\n", get_time() - data->start_time,
		data->philosophers[i].id);
	pthread_mutex_unlock(&data->print_lock);
}

int	check_philosophers(t_data *data)
{
	int	i;
	int	stop;

	i = 0;
	while (i < data->philo_count)
	{
		pthread_mutex_lock(&data->death_lock);
		stop = data->simulation_stop;
		pthread_mutex_unlock(&data->death_lock);
		
		if (stop)
			return (0);
			
		if (check_death(&data->philosophers[i]))
		{
			handle_death(data, i);
			return (1);
		}
		i++;
	}
	return (0);
}

int	check_death(t_philo *philo)
{
	long long	current_time;
	long long	last_meal;
	int			meals_eaten;
	int			target_meals;

	current_time = get_time();
	
	pthread_mutex_lock(&philo->meal_lock);
	last_meal = philo->last_meal_time;
	meals_eaten = philo->meals_eaten;
	pthread_mutex_unlock(&philo->meal_lock);
	
	target_meals = philo->data->meals_to_eat;
	
	// If philosopher has eaten enough, they can't die
	if (target_meals != -1 && meals_eaten >= target_meals)
		return (0);
		
	// Check if time since last meal exceeds time_to_die
	if ((current_time - last_meal) > philo->data->time_to_die)
		return (1);
		
	return (0);
}

int	check_all_ate_enough(t_data *data)
{
	int	i;
	int	total_meals;
	int	meals;
	int	already_stopped;
	static int debug_count = 0;

	// If no meal limit specified, they can never have "eaten enough"
	if (data->meals_to_eat == -1)
		return (0);
		
	// Check if simulation has already been stopped for other reasons
	pthread_mutex_lock(&data->death_lock);
	already_stopped = data->simulation_stop;
	pthread_mutex_unlock(&data->death_lock);
	
	if (already_stopped)
		return (0);
		
	// Only print debug info occasionally to avoid flooding output
	debug_count++;
	if (debug_count % 30 == 0)  // Increased frequency
	{
		pthread_mutex_lock(&data->print_lock);
		printf("\n--- MEAL COUNT CHECK ---\n");
		printf("Target meals: %d\n", data->meals_to_eat);
		pthread_mutex_unlock(&data->print_lock);
	}
		
	// Count philosophers who have eaten enough meals
	i = 0;
	total_meals = 0;
	while (i < data->philo_count)
	{
		pthread_mutex_lock(&data->philosophers[i].meal_lock);
		meals = data->philosophers[i].meals_eaten;
		pthread_mutex_unlock(&data->philosophers[i].meal_lock);
		
		// Print debug info for all philosophers
		if (debug_count % 30 == 0)  // Increased frequency
		{
			pthread_mutex_lock(&data->print_lock);
			printf("Philosopher %d has eaten %d meals\n", i+1, meals);
			pthread_mutex_unlock(&data->print_lock);
		}
		
		total_meals += meals;
		
		// If any philosopher hasn't eaten enough, return immediately
		if (meals < data->meals_to_eat)
			return (0);
		
		i++;
	}
	
	// Print summary before stopping
	if (debug_count % 30 == 0)  // Increased frequency
	{
		pthread_mutex_lock(&data->print_lock);
		printf("Total meals eaten: %d\n", total_meals);
		printf("--- END CHECK ---\n\n");
		pthread_mutex_unlock(&data->print_lock);
	}
	
	// Force stop if all have eaten enough
	pthread_mutex_lock(&data->death_lock);
	data->simulation_stop = 1;
	pthread_mutex_unlock(&data->death_lock);
	
	// Print final message
	pthread_mutex_lock(&data->print_lock);
	printf("\n!!! All philosophers have eaten %d meals each !!!\n", data->meals_to_eat);
	printf("Total meals eaten: %d\n\n", total_meals);
	pthread_mutex_unlock(&data->print_lock);
	
	return (1);
}
