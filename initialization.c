/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   initialization.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 17:02:00 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/27 23:01:34 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/* Initialization functions */
int	init_mutexes(t_data *d)
{
	int	i;

	i = -1;
	if (pthread_mutex_init(&d->m, NULL) != 0)
		return (1);
	while (++i < d->n_philo)
		if (pthread_mutex_init(&d->f[i], NULL) != 0)
			return (1);
	return (0);
}

int	init_philos(t_data *d)
{
	int	i;

	i = -1;
	while (++i < d->n_philo)
	{
		d->p[i].id = i + 1;
		d->p[i].left = i;
		d->p[i].right = (i + 1) % d->n_philo;
		d->p[i].meals = 0;
		d->p[i].last_meal = 0;
		d->p[i].d = d;
	}
	return (0);
}

int	init_simulation(t_data *d)
{
	d->stop = 0;
	d->p = malloc(sizeof(t_philo) * d->n_philo);
	if (!d->p)
		return (1);
	d->f = malloc(sizeof(pthread_mutex_t) * d->n_philo);
	if (!d->f)
	{
		free(d->p);
		return (1);
	}
	if (init_mutexes(d) != 0)
	{
		free(d->p);
		free(d->f);
		return (1);
	}
	init_philos(d);
	return (0);
}

int	cleanup_simulation(t_data *d)
{
	int	i;

	i = -1;
	if (d->f)
	{
		while (++i < d->n_philo)
			pthread_mutex_destroy(&d->f[i]);
		free(d->f);
	}
	pthread_mutex_destroy(&d->m);
	if (d->p)
	{
		free(d->p);
		d->p = NULL;
	}
	return (0);
}
