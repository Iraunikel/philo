/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/18 11:51:47 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/23 19:07:11 by iunikel          ###   ########.fr       */
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

void	print_state(t_philo *philo, char *message)
{
	int	stop;

	pthread_mutex_lock(&philo->data->death_lock);
	stop = philo->data->simulation_stop;
	pthread_mutex_unlock(&philo->data->death_lock);
	
	pthread_mutex_lock(&philo->data->print_lock);
	if (!stop)
		printf("%lld %d %s\n", get_time() - philo->data->start_time, philo->id,
			message);
	pthread_mutex_unlock(&philo->data->print_lock);
}

void	cleanup_simulation(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->philo_count)
	{
		pthread_join(data->philosophers[i].thread, NULL);
		i++;
	}
	
	i = 0;
	while (i < data->philo_count)
	{
		pthread_mutex_destroy(&data->forks[i]);
		pthread_mutex_destroy(&data->philosophers[i].meal_lock);
		i++;
	}
	
	pthread_mutex_destroy(&data->print_lock);
	pthread_mutex_destroy(&data->death_lock);
	free(data->philosophers);
	free(data->forks);
}
