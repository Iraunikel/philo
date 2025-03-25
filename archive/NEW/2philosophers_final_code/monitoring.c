/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitoring.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:08:54 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/24 11:46:30 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static int	check_philosopher_death(t_philo *philo, long current_time)
{
	long	last_meal;
	int		time_to_die;
	int		already_stopped;

	pthread_mutex_lock(&philo->data->death_lock);
	already_stopped = philo->data->simulation_stop;
	pthread_mutex_unlock(&philo->data->death_lock);
	if (already_stopped)
		return (0);
	time_to_die = philo->data->time_to_die;
	pthread_mutex_lock(&philo->meal_lock);
	last_meal = philo->last_meal_time;
	pthread_mutex_unlock(&philo->meal_lock);
	if ((current_time - last_meal) > time_to_die)
		return (handle_death(philo, current_time));
	return (0);
}

int	handle_death(t_philo *philo, long current_time)
{
	pthread_mutex_lock(&philo->data->death_lock);
	if (!philo->data->simulation_stop)
	{
		philo->data->simulation_stop = 1;
		pthread_mutex_unlock(&philo->data->death_lock);
		pthread_mutex_lock(&philo->data->print_lock);
		printf("%lld %d died\n", current_time - philo->data->start_time, 
			philo->id);
		pthread_mutex_unlock(&philo->data->print_lock);
		return (1);
	}
	pthread_mutex_unlock(&philo->data->death_lock);
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
		pthread_mutex_lock(&data->philosophers[i].meal_lock);
		meals = data->philosophers[i].meals_eaten;
		pthread_mutex_unlock(&data->philosophers[i].meal_lock);
		printf("Philosopher %d: %d meals eaten\n", i + 1, meals);
		i++;
	}
	pthread_mutex_unlock(&data->print_lock);
}

static int	handle_all_ate_enough(t_data *data)
{
	pthread_mutex_lock(&data->death_lock);
	if (!data->simulation_stop)
	{
		data->simulation_stop = 1;
		pthread_mutex_unlock(&data->death_lock);
		pthread_mutex_lock(&data->print_lock);
		printf("\n--- ALL PHILOSOPHERS HAVE EATEN ENOUGH ---\n");
		pthread_mutex_unlock(&data->print_lock);
	}
	else
		pthread_mutex_unlock(&data->death_lock);
	return (1);
}

static int	monitor_philosophers(t_data *data)
{
	int		i;
	long	current_time;
	int		stop;

	pthread_mutex_lock(&data->death_lock);
	stop = data->simulation_stop;
	pthread_mutex_unlock(&data->death_lock);
	if (stop)
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
	while (!monitor_philosophers(data))
	{
		debug_count++;
		if (debug_count % 100 == 0)
			print_debug_info(data);
		usleep(1000);
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

static void	print_meal_completion(t_data *data, int total_meals, int debug_count)
{
	if (debug_count % 30 == 0)
	{
		pthread_mutex_lock(&data->print_lock);
		printf("All philosophers have eaten at least %d meals each!\n", 
			data->meals_to_eat);
		printf("Total meals eaten: %d\n", total_meals);
		pthread_mutex_unlock(&data->print_lock);
	}
}

int	check_all_ate_enough(t_data *data)
{
	int	i;
	int	total_meals;
	int	meals;
	int	already_stopped;
	static int debug_count = 0;

	if (data->meals_to_eat == -1)
		return (0);
	pthread_mutex_lock(&data->death_lock);
	already_stopped = data->simulation_stop;
	pthread_mutex_unlock(&data->death_lock);
	if (already_stopped)
		return (0);
	debug_count++;
	print_meal_check_header(data, debug_count);
	i = 0;
	total_meals = 0;
	while (i < data->philo_count)
	{
		pthread_mutex_lock(&data->philosophers[i].meal_lock);
		meals = data->philosophers[i].meals_eaten;
		pthread_mutex_unlock(&data->philosophers[i].meal_lock);
		print_meal_status(data, i, meals, debug_count);
		total_meals += meals;
		if (meals < data->meals_to_eat)
			return (0);
		i++;
	}
	print_meal_completion(data, total_meals, debug_count);
	return (1);
}
