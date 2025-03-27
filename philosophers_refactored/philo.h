/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:08:54 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/27 12:21:56 by iunikel          ###   ########.fr       */
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

# define TIME_PRECISION 1000
# define STEP_SIZE 1
# define MAX_SAFETY_MARGIN 2

typedef struct s_philo  t_philo;
typedef struct s_timekeeper t_timekeeper;
typedef struct s_data t_data;

typedef enum e_action_result
{
        ACTION_SUCCESS = 0,
        ACTION_SIMULATION_STOPPED = 1,
        ACTION_ERROR = 2
} t_action_result;

typedef enum e_philo_state
{
        STATE_THINKING = 0,
        STATE_HUNGRY = 1,
        STATE_EATING = 2,
        STATE_SLEEPING = 3,
        STATE_DONE = 4
} t_philo_state;

typedef struct s_timekeeper
{
        pthread_mutex_t lock;
        pthread_cond_t  cond;
        long                    current_time;
        long                    start_time;
        bool                    running;
        pthread_t               thread;
} t_timekeeper;

typedef struct s_data
{
        int                             philo_count;
        int                             time_to_die;
        int                             time_to_eat;
        int                             time_to_sleep;
        int                             meals_to_eat;
        bool                    simulation_stop;
        pthread_mutex_t *forks;
        pthread_mutex_t print_lock;
        pthread_mutex_t meal_lock;
        pthread_mutex_t stop_lock;
        t_philo                 *philosophers;
        pthread_t               monitor_thread;
        t_timekeeper    *timekeeper;
} t_data;

typedef struct s_philo
{
        int                             id;
        int                             left_fork;
        int                             right_fork;
        int                             meals_eaten;
        long                    last_meal_time;
        t_philo_state   state;
        pthread_t               thread;
        t_data                  *data;
} t_philo;

/* philo.c */
int             main(int argc, char **argv);
int             parse_args(int argc, char **argv, t_data *data);
int             init_simulation(t_data *data);
int             create_threads(t_data *data);
int             wait_and_cleanup(t_data *data);

/* init.c */
int             init_data(t_data *data);
int             init_mutexes(t_data *data);
int             init_philosophers(t_data *data);
int             create_philosopher_threads(t_data *data);
int             cleanup_simulation(t_data *data);

/* routine.c */
void	*philosopher_routine(void *arg);
int             precise_sleep_until(t_philo *philo, long target_time);
int             philosopher_eat(t_philo *philo);
int             philosopher_sleep(t_philo *philo);
int             philosopher_think(t_philo *philo);
void    init_philosopher(t_philo *philo);
int             check_meals_and_update_state(t_philo *philo);
int             execute_philosopher_actions(t_philo *philo);
int             update_meal_time(t_philo *philo, long current_time);
int             handle_eating_state(t_philo *philo, long current_time);
int             calculate_think_time(t_philo *philo);

/* monitoring.c */
void    *death_monitor(void *arg);
int             handle_death(t_philo *philo, long current_time);
int             check_all_ate_enough(t_data *data);
int             check_philosopher_death(t_philo *philo, long current_time);
void    setup_monitor_thread(struct timespec *ts);
int             check_philosophers_meals(t_data *data);
int             handle_all_ate_enough(t_data *data);

/* utils.c */
long    get_time(void);
int             print_state(t_philo *philo, char *state);
int             ft_atoi(const char *str);
bool    is_simulation_stopped(t_data *data);
void    set_simulation_stop(t_data *data, bool value);
long    get_last_meal_time(t_philo *philo);
void    update_last_meal_time(t_philo *philo, long time);
int             get_meals_eaten(t_philo *philo);
void    increment_meals_eaten(t_philo *philo);
int             handle_whitespace_and_sign(const char *str, int *i, int *sign);
int             convert_to_int(const char *str, int i, int sign);

/* timekeeper.c */
void    *timekeeper_routine(void *arg);
int             init_timekeeper(t_data *data);
long    get_sim_time(t_data *data);
void    wait_until(t_philo *philo, long target_time);
void    update_timekeeper_time(t_data *data, long elapsed);
void    setup_timekeeper_sleep(struct timespec *ts);

/* fork_utils.c */
int             take_forks(t_philo *philo);
void    release_forks(t_philo *philo);
int             handle_single_philosopher(t_philo *philo);

#endif
