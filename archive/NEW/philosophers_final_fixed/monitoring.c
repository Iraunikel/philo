/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitoring.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:08:54 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/25 18:24:34 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static int	check_philosopher_death(t_philo *philo, long current_time)
{
	long	last_meal;
	int		time_to_die;
	long	time_since_last_meal;

	if (is_simulation_stopped(philo->data))
		return (0);
		
	time_to_die = philo->data->time_to_die;
	last_meal = get_last_meal_time(philo);
	
	if (last_meal == 0 && (current_time - philo->data->start_time) < time_to_die) 
		return (0);
	
	time_since_last_meal = current_time - last_meal;
	if (time_since_last_meal > time_to_die)
	{
		usleep(1);
		current_time = get_time();
		time_since_last_meal = current_time - last_meal;
		if (time_since_last_meal > time_to_die)
		{
			pthread_mutex_lock(&philo->data->print_lock);
			printf("DEBUG: Philosopher %d death check - Last meal: %ld, Current: %ld, Time since last meal: %ld\n",
				philo->id, last_meal, current_time, time_since_last_meal);
			pthread_mutex_unlock(&philo->data->print_lock);
			return (handle_death(philo, current_time));
		}
	}
	return (0);
}

int	handle_death(t_philo *philo, long current_time)
{
	bool	was_stopped;

	was_stopped = is_simulation_stopped(philo->data);
	if (!was_stopped)
	{
		set_simulation_stop(philo->data, true);
		pthread_mutex_lock(&philo->data->print_lock);
		printf("%ld %d died\n", current_time - philo->data->start_time, 
			philo->id);
		pthread_mutex_unlock(&philo->data->print_lock);
		return (1);
	}
	return (0);
}

static void	print_debug_info(t_data *data)
{
	int	i;
	int	meals;

	pthread_mutex_lock(&data->print_lock);
	printf("\n--- MONITORING STATUS ---\n");
	printf("Philosophers: %d, Time to die: %d ms\n", 
		data->philo_count, data->time_to_die);
	i = 0;
	while (i < data->philo_count)
	{
		meals = get_meals_eaten(&data->philosophers[i]);
		printf("Philosopher %d: %d meals eaten\n", i + 1, meals);
		i++;
	}
	pthread_mutex_unlock(&data->print_lock);
}

static int	handle_all_ate_enough(t_data *data)
{
	bool	was_stopped;

	was_stopped = is_simulation_stopped(data);
	if (!was_stopped)
	{
		set_simulation_stop(data, true);
		pthread_mutex_lock(&data->print_lock);
		printf("\n--- ALL PHILOSOPHERS HAVE EATEN ENOUGH ---\n");
		printf("All philosophers have eaten at least %d meals!\n", data->meals_to_eat);
		printf("Simulation stopping now.\n");
		pthread_mutex_unlock(&data->print_lock);
		return (1);
	}
	return (0);
}

static int	monitor_philosophers(t_data *data)
{
	int		i;
	long	current_time;

	if (is_simulation_stopped(data))
		return (1);
		
	current_time = get_time();
	i = 0;
	while (i < data->philo_count)
	{
		if (check_philosopher_death(&data->philosophers[i], current_time))
			return (1);
		i++;
	}
	
	if (check_all_ate_enough(data))
		return (handle_all_ate_enough(data));
	return (0);
}

void	*death_monitor(void *arg)
{
	t_data	*data;
	static int	debug_count = 0;

	data = (t_data *)arg;
	while (!is_simulation_stopped(data))
	{
		debug_count++;
		if (debug_count % 100 == 0)
			print_debug_info(data);
		if (monitor_philosophers(data))
			break;
		usleep(100);
	}
	
	pthread_mutex_lock(&data->print_lock);
	printf("\n--- MONITORING ENDING ---\n");
	pthread_mutex_unlock(&data->print_lock);
	return (NULL);
}

static void	print_meal_check_header(t_data *data, int debug_count)
{
	if (debug_count % 30 == 0)
	{
		pthread_mutex_lock(&data->print_lock);
		printf("\n--- MEAL COUNT CHECK ---\n");
		printf("Target meals: %d\n", data->meals_to_eat);
		pthread_mutex_unlock(&data->print_lock);
	}
}

static void	print_meal_status(t_data *data, int i, int meals, int debug_count)
{
	if (debug_count % 30 == 0)
	{
		pthread_mutex_lock(&data->print_lock);
		printf("Philosopher %d has eaten %d meals\n", i+1, meals);
		pthread_mutex_unlock(&data->print_lock);
	}
}

static void	print_meal_completion(t_data *data, int total_meals)
{
	pthread_mutex_lock(&data->print_lock);
	printf("All philosophers have eaten at least %d meals each!\n", 
		data->meals_to_eat);
	printf("Total meals eaten: %d\n", total_meals);
	pthread_mutex_unlock(&data->print_lock);
}

int	check_all_ate_enough(t_data *data)
{
	int		i;
	int		total_meals;
	int		meals;
	static int debug_count = 0;

	if (data->meals_to_eat == -1)
		return (0);
		
	if (is_simulation_stopped(data))
		return (0);
		
	debug_count++;
	print_meal_check_header(data, debug_count);
	
	// First pass to check if all philosophers have eaten enough
	i = 0;
	while (i < data->philo_count)
	{
		meals = get_meals_eaten(&data->philosophers[i]);
		if (meals < data->meals_to_eat)
			return (0);
		i++;
	}
	
	// Small delay to ensure no philosopher is in the middle of eating
	usleep(100);
	
	// Second pass to get accurate counts
	i = 0;
	total_meals = 0;
	while (i < data->philo_count)
	{
		meals = get_meals_eaten(&data->philosophers[i]);
		print_meal_status(data, i, meals, debug_count);
		total_meals += meals;
		i++;
	}
	
	print_meal_completion(data, total_meals);
	return (1);
}
