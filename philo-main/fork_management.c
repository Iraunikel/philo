/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fork_management.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 17:02:00 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/31 20:49:19 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/* Fork handling functions */
static int	handle_single_fork(t_philo *p)
{
	pthread_mutex_lock(&p->d->f[p->left]);
	print_state(p, "has taken a fork");
	pthread_mutex_lock(&p->d->m);
	p->last_meal = get_time() - p->d->start;
	pthread_mutex_unlock(&p->d->m);
	precise_sleep(p->d->t_die / 2);
	if (!get_sim_state(p->d))
		precise_sleep(p->d->t_die / 2);
	pthread_mutex_unlock(&p->d->f[p->left]);
	return (0);
}

static int	try_normal_strategy(t_philo *p, int first, int second,
		long time_since_meal)
{
	int	retry_count;

	retry_count = 0;
	while (retry_count < 3 && !get_sim_state(p->d))
	{
		pthread_mutex_lock(&p->d->f[first]);
		print_state(p, "has taken a fork");
		if (try_take_second_fork(p, first, second))
			return (1);
		retry_count++;
		if (backoff_and_check(p, &time_since_meal))
			return (2);
	}
	return (0);
}

static int	final_attempt(t_philo *p, int first, int second)
{
	pthread_mutex_lock(&p->d->f[first]);
	print_state(p, "has taken a fork");
	pthread_mutex_lock(&p->d->f[second]);
	print_state(p, "has taken a fork");
	return (1);
}

int	take_forks(t_philo *p)
{
	int		first;
	int		second;
	long	time_since_meal;
	int		result;

	if (p->d->n_philo == 1)
		return (handle_single_fork(p));
	if (get_sim_state(p->d))
		return (0);
	get_fork_order(p, &first, &second);
	if (handle_starvation_check(p, &time_since_meal))
		return (try_starving_strategy(p, first, second));
	result = try_normal_strategy(p, first, second, time_since_meal);
	if (result == 1)
		return (1);
	if (result == 2)
		return (try_starving_strategy(p, first, second));
	if (!get_sim_state(p->d))
		return (final_attempt(p, first, second));
	return (0);
}

void	release_forks(t_philo *p)
{
	int	first;
	int	second;

	if (p->d->n_philo == 1)
		return ;
	get_fork_order(p, &first, &second);
	pthread_mutex_unlock(&p->d->f[second]);
	pthread_mutex_unlock(&p->d->f[first]);
}
