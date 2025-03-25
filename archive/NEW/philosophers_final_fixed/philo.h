/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:08:54 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/25 18:07:32 by iunikel          ###   ########.fr       */
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

typedef struct s_data
{
	int				philo_count;
	int				time_to_die;
	int				time_to_eat;
	int				time_to_sleep;
	int				meals_to_eat;
	long			start_time;
	bool			simulation_stop;
	pthread_mutex_t	*forks;
	pthread_mutex_t	print_lock;
	pthread_mutex_t	death_lock;
	pthread_mutex_t	meal_lock;
	pthread_mutex_t	stop_lock;
	t_philo			*philosophers;
	pthread_t		monitor_thread;
}	t_data;

typedef struct s_philo
{
	int				id;
	int				left_fork;
	int				right_fork;
	int				meals_eaten;
	long			last_meal_time;
	pthread_t		thread;
	t_data			*data;
}	t_philo;

/* philo.c */
int		main(int argc, char **argv);
int		parse_args(int argc, char **argv, t_data *data);
int		init_simulation(t_data *data);
int		create_threads(t_data *data);
int		wait_and_cleanup(t_data *data);

/* init.c */
int		init_data(t_data *data);
int		init_mutexes(t_data *data);
int		init_philosophers(t_data *data);
int		create_philosopher_threads(t_data *data);
int		cleanup_simulation(t_data *data);

/* routine.c */
void	*philosopher_routine(void *arg);
void	precise_sleep(int ms);

/* monitoring.c */
void	*death_monitor(void *arg);
int		handle_death(t_philo *philo, long current_time);
int		check_all_ate_enough(t_data *data);

/* utils.c */
long	get_time(void);
int		print_state(t_philo *philo, char *state);
int		ft_atoi(const char *str);
bool	is_simulation_stopped(t_data *data);
void	set_simulation_stop(t_data *data, bool value);
long	get_last_meal_time(t_philo *philo);
void	update_last_meal_time(t_philo *philo, long time);
int		get_meals_eaten(t_philo *philo);
void	increment_meals_eaten(t_philo *philo);

#endif
