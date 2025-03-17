#ifndef PHILO_H
# define PHILO_H

/* Required Libraries */
# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/time.h>

/* State Definitions */
# define EATING    0
# define SLEEPING  1
# define THINKING  2
# define DIED      3

/* Error Messages */
# define ERR_ARGS    "Error: Invalid number of arguments"
# define ERR_INPUT   "Error: Invalid input values"
# define ERR_MALLOC  "Error: Memory allocation failed"
# define ERR_MUTEX   "Error: Mutex initialization failed"
# define ERR_THREAD  "Error: Thread creation failed"
# define ERR_MAXPHILO "Error: Maximum number of philosophers is 200"
# define ERR_MINTIME  "Error: Time values must be at least 60ms"

/* Structures */
// philosopher structure
typedef struct s_philo
{
    int             id; //philosopher id
    int             meals_eaten; //number of meals eaten
    long long       last_meal_time; //last meal time
    int             left_fork; //left fork id
    int             right_fork; //right fork id
    int             state; //current state of the philosopher
    pthread_t       thread; //thread id
    struct s_data   *data; //pointer to the data structure
    pthread_mutex_t *forks; //array of forks
    pthread_mutex_t print_lock; //mutex for printing
    pthread_mutex_t meal_lock; //mutex for meal count
}   t_philo;

// data structure
typedef struct s_data
{
    int             philo_count; //number of philosophers
    int             time_to_die; //time to die
    int             time_to_eat; //time to eat
    int             time_to_sleep; //time to sleep
    int             meals_to_eat; //number of meals to eat
    int             simulation_stop; //simulation stop flag
    long long       start_time; //start time
    t_philo         *philosophers; //array of philosophers              
    pthread_mutex_t *forks; //array of forks
    pthread_mutex_t print_lock; //mutex for printing
    pthread_mutex_t death_lock; //mutex for death
}   t_data;

/* Public Function Declarations */
void    *philosopher_routine(void *arg); //philosopher routine  
long    get_time(void); //get current time
void    precise_sleep(int ms); //precise sleep
int     ft_atoi(const char *str); //convert string to integer
int     check_death(t_philo *philo); //check if the philosopher is dead
void    *death_monitor(void *arg); //death monitor
int     check_all_ate_enough(t_data *data); //check if all philosophers have eaten enough
void	cleanup_simulation(t_data *data); //cleanup simulation
int     init_mutexes(t_data *data); //initialize mutexes
int     init_philosophers(t_data *data); //initialize philosophers
void	print_state(t_philo *philo, char *message); //print state
int     take_forks(t_philo *philo); //take forks
void	release_forks(t_philo *philo); //release forks
int     init_meals(t_data *data, int argc, char **argv); //initialize meals
void	*death_monitor(void *arg); //death monitor
void	handle_death(t_data *data, int i); //handle death
int     check_philosophers(t_data *data); //check philosophers
int     init_data(t_data *data, int argc, char **argv); //initialize data
int     allocate_resources(t_data *data); //allocate resources
int     create_threads(t_data *data, pthread_t *monitor); //create threads

#endif