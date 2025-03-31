/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   death_check.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/31 18:10:35 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/31 18:39:28 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/* Death checking functions */
static long	calculate_time_since_meal(long last, long current_time)
{
	if (last == 0)
		return (current_time);
	return (current_time - last);
}

static long	get_safety_margin(int single_philo)
{
	if (single_philo)
		return (0);
	return (2);
}

static int	handle_death(t_data *d, int i, long time)
{
	d->stop = 1;
	printf("%ld %d died\n", time, d->p[i].id);
	pthread_mutex_unlock(&d->m);
	return (1);
}

static int	is_philo_dead(t_data *d, int i, long since, long time_info[2])
{
	long	safety_margin;
	int		single_philo;

	single_philo = time_info[0];
	safety_margin = get_safety_margin(single_philo);
	if (since >= d->t_die + safety_margin && !d->stop)
		return (handle_death(d, i, time_info[1]));
	return (0);
}

int	check_death(t_data *d, int i, long time)
{
	int		single_philo;
	long	since;
	int		result;
	long	time_info[2];

	pthread_mutex_lock(&d->m);
	single_philo = (d->n_philo == 1);
	since = calculate_time_since_meal(d->p[i].last_meal, time);
	time_info[0] = single_philo;
	time_info[1] = time;
	result = is_philo_dead(d, i, since, time_info);
	if (result)
		return (1);
	pthread_mutex_unlock(&d->m);
	return (0);
}
