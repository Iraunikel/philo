/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fork_strategies.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/31 20:50:33 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/31 20:49:34 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/* Fork strategy helper functions */
int	try_starving_strategy(t_philo *p, int first, int second)
{
	pthread_mutex_lock(&p->d->f[first]);
	print_state(p, "has taken a fork");
	pthread_mutex_lock(&p->d->f[second]);
	print_state(p, "has taken a fork");
	return (1);
}

int	handle_starvation_check(t_philo *p, long *time_since_meal)
{
	long	remaining_time;

	pthread_mutex_lock(&p->d->m);
	*time_since_meal = get_time() - p->d->start - p->last_meal;
	remaining_time = p->d->t_die - *time_since_meal;
	pthread_mutex_unlock(&p->d->m);
	if (remaining_time < (p->d->t_die * 0.3))
		return (1);
	return (0);
}

int	try_take_second_fork(t_philo *p, int first, int second)
{
	if (pthread_mutex_trylock(&p->d->f[second]) == 0)
	{
		print_state(p, "has taken a fork");
		return (1);
	}
	pthread_mutex_unlock(&p->d->f[first]);
	return (0);
}

int	backoff_and_check(t_philo *p, long *time_since_meal)
{
	int	backoff_time;

	backoff_time = 2;
	if (*time_since_meal < p->d->t_die * 0.5)
		backoff_time = 2 + (p->id % 3);
	precise_sleep(backoff_time);
	pthread_mutex_lock(&p->d->m);
	*time_since_meal = get_time() - p->d->start - p->last_meal;
	if (p->d->t_die - *time_since_meal < (p->d->t_die * 0.3))
	{
		pthread_mutex_unlock(&p->d->m);
		return (1);
	}
	pthread_mutex_unlock(&p->d->m);
	return (0);
}

void	get_fork_order(t_philo *p, int *first, int *second)
{
	if (p->left < p->right)
	{
		*first = p->left;
		*second = p->right;
	}
	else
	{
		*first = p->right;
		*second = p->left;
	}
}
