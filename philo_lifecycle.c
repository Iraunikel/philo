/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_lifecycle.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 17:02:00 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/28 11:23:23 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/* Philosopher lifecycle functions */

int	philo_eat(t_philo *p)
{
	if (get_sim_state(p->d) || !take_forks(p))
		return (0);
	print_state(p, "is eating");
	pthread_mutex_lock(&p->d->m);
	p->last_meal = get_time() - p->d->start;
	p->meals++;
	pthread_mutex_unlock(&p->d->m);
	precise_sleep(p->d->t_eat);
	release_forks(p);
	return (1);
}

int	philo_sleep_think(t_philo *p)
{
	long	think_time;

	if (get_sim_state(p->d))
		return (0);
	print_state(p, "is sleeping");
	precise_sleep(p->d->t_sleep);
	if (get_sim_state(p->d))
		return (0);
	print_state(p, "is thinking");
	if (p->d->t_die > p->d->t_eat + p->d->t_sleep)
	{
		think_time = (p->d->t_die - p->d->t_eat - p->d->t_sleep) / 2;
		if (think_time > 0)
			precise_sleep(think_time);
	}
	return (1);
}

void	*philo_routine(void *arg)
{
	t_philo	*p;
	int		stop;

	p = (t_philo *)arg;
	pthread_mutex_lock(&p->d->m);
	p->last_meal = 0;
	pthread_mutex_unlock(&p->d->m);
	print_state(p, "is thinking");
	if (p->id % 2)
		usleep(1000);
	while (1)
	{
		pthread_mutex_lock(&p->d->m);
		stop = p->d->stop;
		pthread_mutex_unlock(&p->d->m);
		if (stop)
			break ;
		if (!philo_eat(p))
			break ;
		if (!philo_sleep_think(p))
			break ;
	}
	return (NULL);
}
