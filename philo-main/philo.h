/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 17:02:00 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/31 20:54:12 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_H
# define PHILO_H

# include <limits.h>
# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/time.h>
# include <unistd.h>

/* Data structures */
typedef struct s_philo
{
	int				id;
	int				left;
	int				right;
	int				meals;
	long			last_meal;
	pthread_t		th;
	struct s_data	*d;
}					t_philo;

typedef struct s_data
{
	int				n_philo;
	int				t_die;
	int				t_eat;
	int				t_sleep;
	int				n_meals;
	int				stop;
	long			start;
	pthread_mutex_t	*f;
	pthread_mutex_t	m;
	t_philo			*p;
	pthread_t		mon;
}					t_data;

/* Function prototypes */
long				get_time(void);
int					get_sim_state(t_data *d);
void				set_sim_state(t_data *d, int v);
int					print_state(t_philo *p, char *s);
int					precise_sleep(long ms);
int					take_forks(t_philo *p);
void				release_forks(t_philo *p);
int					philo_eat(t_philo *p);
int					philo_sleep_think(t_philo *p);
void				*philo_routine(void *arg);

/* Fork strategy functions */
int					try_starving_strategy(t_philo *p, int first, int second);
int					handle_starvation_check(t_philo *p, long *time_since_meal);
int					try_take_second_fork(t_philo *p, int first, int second);
int					backoff_and_check(t_philo *p, long *time_since_meal);
void				get_fork_order(t_philo *p, int *first, int *second);

/* Death check functions */
int					check_death(t_data *d, int i, long time);

/* Meal monitoring functions */
int					check_meals_complete(t_data *d);
void				*monitor_routine(void *arg);

/* Initialization and cleanup */
int					init_mutexes(t_data *d);
int					init_philos(t_data *d);
int					init_simulation(t_data *d);
int					cleanup_simulation(t_data *d);

/* Utility functions */
int					ft_atoi(char const *str);
int					ft_isspace(char c);
void				try_unlock_mutex(pthread_mutex_t *mutex);
int					parse_args(int argc, char **argv, t_data *d);
int					run_simulation(t_data *d);
int					determine_fork_order(t_philo *p, int *first, int *second);

#endif