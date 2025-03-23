/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/22 23:15:00 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/22 23:03:38 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

// Helper function to convert a number to a string
// Returns the length of the formatted number
int	format_number(char *buffer, long number)
{
	char	temp[32];
	int		length;
	int		i;
	
	if (number == 0)
	{
		buffer[0] = '0';
		return (1);
	}
	length = 0;
	while (number > 0)
	{
		temp[length++] = '0' + (number % 10);
		number /= 10;
	}
	i = 0;
	while (length > 0)
		buffer[i++] = temp[--length];
	return (i);
}

// Helper function to copy a string into a buffer
// Returns the length of the copied string
int	str_copy(char *buffer, const char *str)
{
	int	i;
	
	i = 0;
	while (str[i] != '\0')
	{
		buffer[i] = str[i];
		i++;
	}
	return (i);
} 