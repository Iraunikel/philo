/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 17:02:00 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/30 22:22:38 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/* Main program flow */
int	parse_args(int argc, char **argv, t_data *d)
{
	if (argc < 5 || argc > 6)
		return (1);
	d->n_philo = ft_atoi(argv[1]);
	d->t_die = ft_atoi(argv[2]);
	d->t_eat = ft_atoi(argv[3]);
	d->t_sleep = ft_atoi(argv[4]);
	if (argc == 6)
		d->n_meals = ft_atoi(argv[5]);
	else
		d->n_meals = -1;
	if (d->n_philo <= 0 || d->n_philo > 200 || d->t_die <= 0 || d->t_eat <= 0
		|| d->t_sleep <= 0 || (argc == 6 && d->n_meals <= 0))
		return (1);
	return (0);
}

int	run_simulation(t_data *d)
{
	int	i;

	i = -1;
	d->start = get_time();
	while (++i < d->n_philo)
	{
		if (pthread_create(&d->p[i].th, NULL, philo_routine, &d->p[i]) != 0)
		{
			set_sim_state(d, 1);
			return (1);
		}
	}
	if (pthread_create(&d->mon, NULL, monitor_routine, d) != 0)
	{
		set_sim_state(d, 1);
		return (1);
	}
	i = -1;
	while (++i < d->n_philo)
		pthread_join(d->p[i].th, NULL);
	pthread_join(d->mon, NULL);
	return (0);
}
