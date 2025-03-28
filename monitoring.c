/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitoring.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 17:02:00 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/28 11:22:59 by iunikel          ###   ########.fr       */
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
	if (last == 0)
		since = time;
	else
		since = time - last;
	if (since >= d->t_die && !d->stop)
	{
		d->stop = 1;
		printf("%ld %d died\n", time, d->p[i].id);
		pthread_mutex_unlock(&d->m);
		return (1);
	}
	pthread_mutex_unlock(&d->m);
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
	pthread_mutex_lock(&d->m);
	while (++i < d->n_philo && all)
	{
		meals = d->p[i].meals;
		if (meals < d->n_meals)
			all = 0;
	}
	if (all && !d->stop)
		d->stop = 1;
	pthread_mutex_unlock(&d->m);
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
		usleep(100);
	}
	return (NULL);
}
