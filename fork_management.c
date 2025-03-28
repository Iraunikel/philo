/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fork_management.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 17:02:00 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/27 23:03:08 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/* Fork handling functions */
int	determine_fork_order(t_philo *p, int *first, int *second)
{
	if (p->id % 2 == 0)
	{
		*first = p->left;
		*second = p->right;
	}
	else
	{
		*first = p->right;
		*second = p->left;
	}
	return (1);
}

int	take_forks(t_philo *p)
{
	int	first;
	int	second;

	if (p->d->n_philo == 1)
	{
		pthread_mutex_lock(&p->d->f[p->left]);
		print_state(p, "has taken a fork");
		precise_sleep(p->d->t_die);
		pthread_mutex_unlock(&p->d->f[p->left]);
		return (0);
	}
	if (get_sim_state(p->d))
		return (0);
	determine_fork_order(p, &first, &second);
	pthread_mutex_lock(&p->d->f[first]);
	print_state(p, "has taken a fork");
	pthread_mutex_lock(&p->d->f[second]);
	print_state(p, "has taken a fork");
	return (1);
}

void	release_forks(t_philo *p)
{
	if (p->d->n_philo == 1)
		return ;
	pthread_mutex_unlock(&p->d->f[p->right]);
	pthread_mutex_unlock(&p->d->f[p->left]);
}
