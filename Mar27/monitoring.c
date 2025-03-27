/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitoring.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 17:02:00 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/27 23:04:09 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/* Monitoring functions */
int	check_death(t_data *d, int i, long time)
{
	long	last;
	long	since;

	pthread_mutex_lock(&d->m);
	last = d->p[i].last_meal;
	pthread_mutex_unlock(&d->m);
	if (last == 0)
		since = time;
	else
		since = time - last;
	if (since >= d->t_die)
	{
		set_sim_state(d, 1);
		pthread_mutex_lock(&d->m);
		printf("%ld %d died\n", time, d->p[i].id);
		pthread_mutex_unlock(&d->m);
		return (1);
	}
	return (0);
}

int	check_meals_complete(t_data *d)
{
	int	i;
	int	meals;
	int	all;

	i = -1;
	all = 1;
	if (d->n_meals == -1)
		return (0);
	while (++i < d->n_philo && all)
	{
		pthread_mutex_lock(&d->m);
		meals = d->p[i].meals;
		pthread_mutex_unlock(&d->m);
		if (meals < d->n_meals)
			all = 0;
	}
	if (all)
		set_sim_state(d, 1);
	return (all);
}

void	*monitor_routine(void *arg)
{
	t_data	*d;
	int		i;
	long	time;

	d = (t_data *)arg;
	usleep(1000);
	while (!get_sim_state(d))
	{
		time = get_time() - d->start;
		i = -1;
		while (++i < d->n_philo)
			if (check_death(d, i, time))
				return (NULL);
		if (check_meals_complete(d))
			return (NULL);
		usleep(500);
	}
	return (NULL);
}
