/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   state_management.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 17:02:00 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/28 11:21:42 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/* State management functions */
int	get_sim_state(t_data *d)
{
	int	s;

	pthread_mutex_lock(&d->m);
	s = d->stop;
	pthread_mutex_unlock(&d->m);
	return (s);
}

void	set_sim_state(t_data *d, int v)
{
	pthread_mutex_lock(&d->m);
	d->stop = v;
	pthread_mutex_unlock(&d->m);
}

int	print_state(t_philo *p, char *s)
{
	long	time;
	int		stop;

	pthread_mutex_lock(&p->d->m);
	stop = p->d->stop;
	if (stop)
	{
		pthread_mutex_unlock(&p->d->m);
		return (1);
	}
	time = get_time() - p->d->start;
	printf("%ld %d %s\n", time, p->id, s);
	pthread_mutex_unlock(&p->d->m);
	return (0);
}
