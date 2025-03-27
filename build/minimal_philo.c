/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minimal_philo.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: iunikel <marvin@student.42.fr>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 17:02:00 by iunikel           #+#    #+#             */
/*   Updated: 2025/03/27 18:15:17 by iunikel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <limits.h>

/* Core macros */
#define LOCK(x) pthread_mutex_lock(x)
#define UNLOCK(x) pthread_mutex_unlock(x)
#define ELAPSED(d) (get_time() - (d)->start)
#define STOPPED(d) (get_sim_state(d))
#define PRINT(p, s) print_state(p, s)
#define SLEEP(t) precise_sleep(t)
#define MUTEX_OP(m, op) do { LOCK(m); op; UNLOCK(m); } while(0)
#define CHECK_STOP(d) if(STOPPED(d)) return NULL
#define SAFE_FREE(x) if(x) { free(x); x = NULL; }

/* Data structures */
typedef struct s_philo {
	int id;
	int left;
	int right;
	int meals;
	long last_meal;
	pthread_t th;
	struct s_data *d;
} t_philo;

typedef struct s_data {
	int n_philo;
	int t_die;
	int t_eat;
	int t_sleep;
	int n_meals;
	int stop;
	long start;
	pthread_mutex_t *f;
	pthread_mutex_t m;
	t_philo *p;
	pthread_t mon;
} t_data;

/* Function prototypes */
long get_time(void);
int get_sim_state(t_data *d);
void set_sim_state(t_data *d, int v);
int print_state(t_philo *p, char *s);
int precise_sleep(long ms);
int take_forks(t_philo *p);
void release_forks(t_philo *p);
int philo_eat(t_philo *p);
int philo_sleep_think(t_philo *p);
void *philo_routine(void *arg);
int check_death(t_data *d, int i, long time);
int check_meals_complete(t_data *d);
void *monitor_routine(void *arg);
int init_mutexes(t_data *d);
int init_philos(t_data *d);
int init_simulation(t_data *d);
int cleanup_simulation(t_data *d);
int ft_atoi(const char *s);
int parse_args(int argc, char **argv, t_data *d);
int run_simulation(t_data *d);

/* Time utility functions */
long get_time(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

int precise_sleep(long ms)
{
	long start = get_time();
	while (get_time() - start < ms)
		usleep(500);
	return 0;
}

/* State management functions */
int get_sim_state(t_data *d)
{
	int s;
	MUTEX_OP(&d->m, s = d->stop);
	return s;
}

void set_sim_state(t_data *d, int v)
{
	MUTEX_OP(&d->m, d->stop = v);
}

int print_state(t_philo *p, char *s)
{
	if (STOPPED(p->d))
		return 1;
	MUTEX_OP(&p->d->m, if (!p->d->stop) printf("%ld %d %s\n", 
		ELAPSED(p->d), p->id, s));
	return 0;
}

/* Fork handling functions */
int take_forks(t_philo *p)
{
	int first, second;
	
	if (p->d->n_philo == 1)
	{
		LOCK(&p->d->f[p->left]);
		PRINT(p, "has taken a fork");
		SLEEP(p->d->t_die);
		UNLOCK(&p->d->f[p->left]);
		return 0;
	}
	if (STOPPED(p->d))
		return 0;
	first = (p->id % 2) ? p->right : p->left;
	second = (p->id % 2) ? p->left : p->right;
	LOCK(&p->d->f[first]);
	PRINT(p, "has taken a fork");
	LOCK(&p->d->f[second]);
	PRINT(p, "has taken a fork");
	return 1;
}

void release_forks(t_philo *p)
{
	if (p->d->n_philo == 1)
		return;
	UNLOCK(&p->d->f[p->right]);
	UNLOCK(&p->d->f[p->left]);
}

/* Philosopher lifecycle functions */
int philo_eat(t_philo *p)
{
	if (STOPPED(p->d) || !take_forks(p))
		return 0;
	MUTEX_OP(&p->d->m, p->last_meal = ELAPSED(p->d); p->meals++);
	PRINT(p, "is eating");
	SLEEP(p->d->t_eat);
	release_forks(p);
	return 1;
}

int philo_sleep_think(t_philo *p)
{
	long think_time;
	
	if (STOPPED(p->d))
		return 0;
	PRINT(p, "is sleeping");
	SLEEP(p->d->t_sleep);
	if (STOPPED(p->d))
		return 0;
	PRINT(p, "is thinking");
	if (p->d->t_die > p->d->t_eat + p->d->t_sleep)
	{
		think_time = (p->d->t_die - p->d->t_eat - p->d->t_sleep) / 2;
		if (think_time > 0)
			SLEEP(think_time);
	}
	return 1;
}

void *philo_routine(void *arg)
{
	t_philo *p = (t_philo *)arg;
	
	MUTEX_OP(&p->d->m, p->last_meal = 0);
	PRINT(p, "is thinking");
	if (p->id % 2)
		usleep(1000);
	while (!STOPPED(p->d))
	{
		if (!philo_eat(p))
			break;
		if (!philo_sleep_think(p))
			break;
	}
	return NULL;
}

/* Monitoring functions */
int check_death(t_data *d, int i, long time)
{
	long last, since;
	
	MUTEX_OP(&d->m, last = d->p[i].last_meal);
	since = (last == 0) ? time : time - last;
	if (since >= d->t_die)
	{
		set_sim_state(d, 1);
		MUTEX_OP(&d->m, printf("%ld %d died\n", time, d->p[i].id));
		return 1;
	}
	return 0;
}

int check_meals_complete(t_data *d)
{
	int i = -1, meals, all = 1;
	
	if (d->n_meals == -1)
		return 0;
	while (++i < d->n_philo && all)
	{
		MUTEX_OP(&d->m, meals = d->p[i].meals);
		if (meals < d->n_meals)
			all = 0;
	}
	if (all)
		set_sim_state(d, 1);
	return all;
}

void *monitor_routine(void *arg)
{
	t_data *d = (t_data *)arg;
	int i;
	long time;
	
	usleep(1000);
	while (!STOPPED(d))
	{
		time = ELAPSED(d);
		i = -1;
		while (++i < d->n_philo)
			if (check_death(d, i, time))
				return NULL;
		if (check_meals_complete(d))
			return NULL;
		usleep(500);
	}
	return NULL;
}

/* Initialization functions */
int init_mutexes(t_data *d)
{
	int i = -1;
	
	if (pthread_mutex_init(&d->m, NULL) != 0)
		return 1;
	while (++i < d->n_philo)
		if (pthread_mutex_init(&d->f[i], NULL) != 0)
			return 1;
	return 0;
}

int init_philos(t_data *d)
{
	int i = -1;
	
	while (++i < d->n_philo)
	{
		d->p[i].id = i + 1;
		d->p[i].left = i;
		d->p[i].right = (i + 1) % d->n_philo;
		d->p[i].meals = 0;
		d->p[i].last_meal = 0;
		d->p[i].d = d;
	}
	return 0;
}

int init_simulation(t_data *d)
{
	d->stop = 0;
	d->p = malloc(sizeof(t_philo) * d->n_philo);
	if (!d->p)
		return 1;
	d->f = malloc(sizeof(pthread_mutex_t) * d->n_philo);
	if (!d->f)
	{
		free(d->p);
		return 1;
	}
	if (init_mutexes(d) != 0)
	{
		free(d->p);
		free(d->f);
		return 1;
	}
	init_philos(d);
	return 0;
}

int cleanup_simulation(t_data *d)
{
	int i = -1;
	
	if (d->f)
	{
		while (++i < d->n_philo)
			pthread_mutex_destroy(&d->f[i]);
		free(d->f);
	}
	pthread_mutex_destroy(&d->m);
	SAFE_FREE(d->p);
	return 0;
}

/* String to integer conversion */
int ft_atoi(const char *s)
{
	int i = 0, sign = 1, res = 0;
	
	while (s[i] == ' ' || (s[i] >= 9 && s[i] <= 13))
		i++;
	if (s[i] == '-' || s[i] == '+')
		sign = (s[i++] == '-') ? -1 : 1;
	while (s[i] >= '0' && s[i] <= '9')
	{
		if (res > INT_MAX / 10 || (res == INT_MAX / 10 && s[i] - '0' > INT_MAX % 10))
			return (sign == 1) ? INT_MAX : INT_MIN;
		res = res * 10 + (s[i++] - '0');
	}
	return res * sign;
}

/* Main program flow */
int parse_args(int argc, char **argv, t_data *d)
{
	if (argc < 5 || argc > 6)
		return 1;
	d->n_philo = ft_atoi(argv[1]);
	d->t_die = ft_atoi(argv[2]);
	d->t_eat = ft_atoi(argv[3]);
	d->t_sleep = ft_atoi(argv[4]);
	d->n_meals = (argc == 6) ? ft_atoi(argv[5]) : -1;
	if (d->n_philo <= 0 || d->n_philo > 200 || d->t_die <= 0 || 
		d->t_eat <= 0 || d->t_sleep <= 0 || (argc == 6 && d->n_meals <= 0))
		return 1;
	return 0;
}

int run_simulation(t_data *d)
{
	int i = -1;
	
	d->start = get_time();
	while (++i < d->n_philo)
		if (pthread_create(&d->p[i].th, NULL, philo_routine, &d->p[i]) != 0)
		{
			set_sim_state(d, 1);
			return 1;
		}
	if (pthread_create(&d->mon, NULL, monitor_routine, d) != 0)
	{
		set_sim_state(d, 1);
		return 1;
	}
	i = -1;
	while (++i < d->n_philo)
		pthread_join(d->p[i].th, NULL);
	pthread_join(d->mon, NULL);
	return 0;
}

int main   (int argc, char **argv)
{
	t_data d;
	
	if (parse_args(argc, argv, &d) != 0)
	{
		printf("Usage: %s number_of_philosophers time_to_die time_to_eat "
			"time_to_sleep [number_of_times_each_philosopher_must_eat]\n", argv[0]);
		return 1;
	}
	if (init_simulation(&d) != 0)
	{
		printf("Error: Failed to initialize simulation\n");
		return 1;
	}
	run_simulation(&d);
	cleanup_simulation(&d);
	return 0;
}
