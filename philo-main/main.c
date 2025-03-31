/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 17:02:00 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/31 13:47:11 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	main(int argc, char **argv)
{
	t_data	d;

	if (parse_args(argc, argv, &d))
	{
		printf("Error: invalid arguments\n");
		return (1);
	}
	if (init_simulation(&d))
	{
		printf("Error: failed to initialize simulation\n");
		return (1);
	}
	run_simulation(&d);
	cleanup_simulation(&d);
	return (0);
}
