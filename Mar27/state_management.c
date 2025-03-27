/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   state_management.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 17:02:00 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/27 23:02:56 by iunikel          ###   ########.fr       */
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
	if (get_sim_state(p->d))
		return (1);
	pthread_mutex_lock(&p->d->m);
	if (!p->d->stop)
		printf("%ld %d %s\n", get_time() - p->d->start, p->id, s);
	pthread_mutex_unlock(&p->d->m);
	return (0);
}
