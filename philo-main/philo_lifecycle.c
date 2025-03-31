/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_lifecycle.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 17:02:00 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/31 18:37:49 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

static int	update_meal_info(t_philo *p)
{
	pthread_mutex_lock(&p->d->m);
	p->last_meal = get_time() - p->d->start;
	p->meals++;
	pthread_mutex_unlock(&p->d->m);
	return (1);
}

int	philo_eat(t_philo *p)
{
	if (get_sim_state(p->d))
		return (0);
	if (!take_forks(p))
		return (0);
	update_meal_info(p);
	print_state(p, "is eating");
	precise_sleep(p->d->t_eat);
	release_forks(p);
	return (1);
}

static int	should_skip_thinking(t_philo *p)
{
	long	current_time;
	long	time_since_last_meal;
	long	time_remaining;

	pthread_mutex_lock(&p->d->m);
	current_time = get_time() - p->d->start;
	time_since_last_meal = current_time - p->last_meal;
	time_remaining = p->d->t_die - time_since_last_meal;
	pthread_mutex_unlock(&p->d->m);
	if (time_remaining < (p->d->t_die * 0.6))
		return (1);
	if (p->d->t_eat + p->d->t_sleep > p->d->t_die * 0.8)
		return (1);
	return (0);
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
	if (should_skip_thinking(p))
		return (1);
	think_time = 3;
	if (p->id % 2 == 0)
		think_time = 5;
	precise_sleep(think_time);
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
	if (p->id % 2 == 0)
		precise_sleep(5);
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
