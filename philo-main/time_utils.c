/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 17:02:00 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/31 20:34:39 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

long	get_time(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

static long	calculate_sleep_interval(long ms, long elapsed)
{
	long	sleep_interval;

	sleep_interval = ms - elapsed;
	if (sleep_interval > 20)
		sleep_interval = 20;
	else if (sleep_interval > 0)
		sleep_interval = 1;
	return (sleep_interval);
}

int	precise_sleep(long ms)
{
	long	start;
	long	elapsed;
	long	sleep_interval;

	start = get_time();
	while (1)
	{
		elapsed = get_time() - start;
		if (elapsed >= ms)
			break ;
		sleep_interval = calculate_sleep_interval(ms, elapsed);
		usleep(sleep_interval * 100);
	}
	return (0);
}
