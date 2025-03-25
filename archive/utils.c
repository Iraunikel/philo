/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/18 11:51:47 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/23 13:50:32 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	ft_atoi(char const *str)
{
	int	nbr;
	int	i;

	if (!str)
		return (0);
	nbr = 0;
	i = 0;
	while (str[i] >= '0' && str[i] <= '9')
	{
		nbr = nbr * 10 + (str[i] - '0');
		i++;
	}
	return (nbr);
}

long	get_time(void)
{
	struct timeval	tv;
	
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

long	get_elapsed_time(t_data *data)
{
	long current_time;
	long start_time;
	long elapsed;
	
	// Получаем текущее время до любых блокировок
	current_time = get_time();
	
	// Читаем shared data в thread-safe режиме
	pthread_mutex_lock(&data->death_lock);
	start_time = data->start_time;
	pthread_mutex_unlock(&data->death_lock);
	
	// Вычисляем время без блокировок
	elapsed = current_time - start_time;
	return (elapsed);
}

void	print_state(t_philo *philo, char *message)
{
	long time_since_start;
	
	// Проверяем, не остановлена ли симуляция
	pthread_mutex_lock(&philo->data->death_lock);
	if (philo->data->simulation_stop)
	{
		pthread_mutex_unlock(&philo->data->death_lock);
		return;
	}
	
	// Получаем время начала симуляции, пока есть блокировка
	time_since_start = get_time() - philo->data->start_time;
	pthread_mutex_unlock(&philo->data->death_lock);
	
	// Блокируем вывод для атомарной печати
	pthread_mutex_lock(&philo->data->print_lock);
	printf("%ld %d %s\n", time_since_start, philo->id, message);
	pthread_mutex_unlock(&philo->data->print_lock);
}

void	cleanup_simulation(t_data *data)
{
	int	i;

	// Ensure all philosopher threads have exited
	i = 0;
	while (i < data->philo_count)
	{
		pthread_join(data->philosophers[i].thread, NULL);
		i++;
	}
	
	// Now that all threads are done, we can safely destroy the mutexes
	i = 0;
	while (i < data->philo_count)
	{
		pthread_mutex_destroy(&data->forks[i]);
		pthread_mutex_destroy(&data->philosophers[i].meal_lock);
		i++;
	}
	
	pthread_mutex_destroy(&data->print_lock);
	pthread_mutex_destroy(&data->death_lock);
	
	// Free allocated memory
	free(data->philosophers);
	free(data->forks);
}

