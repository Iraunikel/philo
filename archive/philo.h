/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/18 12:25:22 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/23 13:50:32 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILO_H
# define PHILO_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/time.h>
# include <unistd.h>

/* State Definitions */
# define EATING 0
# define SLEEPING 1
# define THINKING 2
# define DIED 3

/* Error Messages */
# define ERR_ARGS "Error: Invalid number of arguments"
# define ERR_INPUT "Error: Invalid input values"
# define ERR_MALLOC "Error: Memory allocation failed"
# define ERR_MUTEX "Error: Mutex initialization failed"
# define ERR_THREAD "Error: Thread creation failed"
# define ERR_MAXPHILO "Error: Maximum number of philosophers is 200"
# define ERR_MINTIME "Error: Time values must be at least 60ms"

/* Structures */
typedef struct s_philo
{
	int				id;
	int				meals_eaten;
	long long		last_meal_time;
	int				left_fork;
	int				right_fork;
	int				state;
	pthread_t		thread;
	struct s_data	*data;
	pthread_mutex_t	*forks;
	pthread_mutex_t	meal_lock;
}					t_philo;

typedef struct s_data
{
	int				philo_count;
	int				time_to_die;
	int				time_to_eat;
	int				time_to_sleep;
	int				meals_to_eat;
	int				simulation_stop;
	long long		start_time;
	t_philo			*philosophers;
	pthread_mutex_t	*forks;
	pthread_mutex_t	print_lock;
	pthread_mutex_t	death_lock;
}					t_data;

/* Initialization Functions */
int					init_data(t_data *data, int argc, char **argv);
int					init_mutexes(t_data *data);
int					init_philosophers(t_data *data);
int					init_meals(t_data *data, int argc, char **argv);
int					allocate_resources(t_data *data);
int					create_threads(t_data *data, pthread_t *monitor);

/* Philosopher Routine Functions */
void				*philosopher_routine(void *arg);
void				print_state(t_philo *philo, char *message);
int					take_forks(t_philo *philo);
int					take_fork(t_philo *philo, int fork_id);
int					philosopher_eat(t_philo *philo);
void				handle_one_philo(t_philo *philo);
void				precise_sleep(int ms);

/* Monitoring Functions */
void				*death_monitor(void *arg);
int					check_death(t_philo *philo);
int					check_philosophers(t_data *data);
void				handle_death(t_philo *philo);
int					check_all_ate_enough(t_data *data);

/* Utility Functions */
long				get_time(void);
long				get_elapsed_time(t_data *data);
int					ft_atoi(const char *str);
void				cleanup_simulation(t_data *data);
int					format_number(char *buffer, long number);
int					str_copy(char *buffer, const char *str);

#endif