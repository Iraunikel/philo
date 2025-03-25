/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/17 17:08:54 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/25 21:49:16 by iunikel          ###   ########.fr       */
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

# define MIN_SLEEP_TIME 60
# define SAFETY_MARGIN 10
# define MAX_PHILOSOPHERS 200
# define SYNC_INTERVAL 5

typedef enum e_philo_state
{
    STATE_THINKING,
    STATE_HUNGRY,
    STATE_EATING,
    STATE_SLEEPING,
    STATE_DONE
} t_philo_state;

typedef enum e_action_result
{
    ACTION_SUCCESS,
    ACTION_FAILURE,
    ACTION_SIMULATION_STOPPED
} t_action_result;

typedef struct s_philo t_philo;
typedef struct s_scheduler t_scheduler;

typedef struct s_data
{
    int                 philo_count;
    int                 time_to_die;
    int                 time_to_eat;
    int                 time_to_sleep;
    int                 meals_to_eat;
    long                start_time;
    bool                simulation_stop;
    bool                all_philos_ready;
    
    pthread_mutex_t     *forks;
    pthread_mutex_t     global_lock;
    pthread_mutex_t     print_lock;
    
    t_philo            *philosophers;
    t_scheduler        *scheduler;
    
    pthread_t           monitor_thread;
    pthread_t           scheduler_thread;
} t_data;

typedef struct s_philo
{
    int                 id;
    int                 left_fork;
    int                 right_fork;
    int                 meals_eaten;
    long                last_meal_time;
    t_philo_state       state;
    bool                has_left_fork;
    bool                has_right_fork;
    pthread_t           thread;
    t_data              *data;
} t_philo;

typedef struct s_scheduler
{
    pthread_mutex_t     lock;
    pthread_cond_t      cond;
    int                 active_philos;
    int                 ready_philos;
    bool                cycle_complete;
    long                last_check_time;
} t_scheduler;

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
int             init_scheduler(t_data *data);
int             cleanup_simulation(t_data *data);

/* scheduler.c */
void            *scheduler_routine(void *arg);
bool            is_simulation_stopped(t_data *data);

/* routine.c */
void            *philosopher_routine(void *arg);
t_action_result philosopher_think(t_philo *philo);
t_action_result philosopher_eat(t_philo *philo);
t_action_result philosopher_sleep(t_philo *philo);
void            precise_sleep(int ms);

/* actions.c */
t_action_result take_forks(t_philo *philo);
void            release_forks(t_philo *philo);
t_action_result update_meal_time(t_philo *philo);
t_action_result increment_meals_eaten(t_philo *philo);
t_action_result check_meal_completion(t_data *data);

/* monitoring.c */
void            *death_monitor(void *arg);
int             check_philosopher_death(t_philo *philo, long current_time);
int             handle_death(t_philo *philo, long current_time);
int             check_all_ate_enough(t_data *data);
void            print_final_meal_counts(t_data *data);

/* utils.c */
long            get_time(void);
int             print_state(t_philo *philo, char *state);
int             ft_atoi(const char *str);
void            set_simulation_stop(t_data *data, bool value);
void            log_action(t_philo *philo, const char *action);

/* testing.c */
void            run_test_suite(void);
void            test_case_one_philosopher(void);
void            test_case_five_philosophers_no_death(void);
void            test_case_five_philosophers_with_meals(void);
void            test_case_four_philosophers_no_death(void);

#endif
