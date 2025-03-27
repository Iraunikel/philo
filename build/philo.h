/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 16:34:00 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/27 16:34:00 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_H
# define PHILO_H

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <pthread.h>
# include <sys/time.h>
# include <stdbool.h>
# include <limits.h>

typedef struct s_philo	t_philo;
typedef struct s_data	t_data;

typedef struct s_data
{
	int				philo_count;
	int				time_to_die;
	int				time_to_eat;
	int				time_to_sleep;
	int				meals_to_eat;
	bool			simulation_stop;
	long			start_time;
	pthread_mutex_t	*forks;
	pthread_mutex_t	print_lock;
	pthread_mutex_t	data_lock;
	t_philo			*philosophers;
	pthread_t		monitor_thread;
}	t_data;

typedef struct s_philo
{
	int			id;
	int			left_fork;
	int			right_fork;
	int			meals_eaten;
	long		last_meal_time;
	pthread_t	thread;
	t_data		*data;
}	t_philo;

/* philo.c */
int		main(int argc, char **argv);
int		parse_args(int argc, char **argv, t_data *data);
int		init_simulation(t_data *data);
int		create_threads(t_data *data);
int		wait_and_cleanup(t_data *data);

/* philo_core.c */
void	*philosopher_routine(void *arg);
void	*death_monitor(void *arg);
int		philosopher_eat(t_philo *philo);
int		philosopher_sleep(t_philo *philo);
int		philosopher_think(t_philo *philo);
int		check_all_ate_enough(t_data *data);
int		check_philosopher_death(t_philo *philo, long current_time);

/* utils.c */
long	get_time(void);
long	get_elapsed_time(t_data *data);
int		ft_atoi(const char *str);
int		print_state(t_philo *philo, char *state);
bool	is_simulation_stopped(t_data *data);
void	set_simulation_stop(t_data *data, bool value);
int		take_forks(t_philo *philo);
void	release_forks(t_philo *philo);
int		init_data(t_data *data);
int		init_mutexes(t_data *data);
int		init_philosophers(t_data *data);
int		cleanup_simulation(t_data *data);
int		precise_sleep(long ms);

#endif
