/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 17:02:00 by iunikel           #+#    #+#             */
/*   Updated: 2025/04/01 09:04:58 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

/* String to integer conversion */
int	ft_isspace(char c)
{
	return (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v'
		|| c == '\f');
}

int	ft_atoi(char const *str)
{
	int	sign;
	int	nbr;

	if (str == 0)
		return (0);
	sign = 1;
	nbr = 0;
	while (ft_isspace(*str))
		str++;
	if (*str == '+')
		str++;
	else if (*str == '-')
	{
		sign = -1;
		str++;
	}
	while (*str >= '0' && *str <= '9')
		nbr = nbr * 10 + *str++ - '0';
	return (nbr * sign);
}

void	try_unlock_mutex(pthread_mutex_t *mutex)
{
	pthread_mutex_destroy(mutex);
}
