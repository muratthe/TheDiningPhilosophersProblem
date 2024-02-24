#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define MAX_PHILOSOPHERS 27

typedef struct {
    int id;
    int num_times_dined;
    double total_hungry_duration;
    double *hungry_durations; // array to store individual hungry times
    int durations_recorded;   // count of durations
    struct timespec start_hungry_time;
} philosopher_t;

pthread_mutex_t chopsticks[MAX_PHILOSOPHERS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
philosopher_t philosophers[MAX_PHILOSOPHERS];
pthread_t threads[MAX_PHILOSOPHERS];
int num_philosophers;
int min_think, max_think, min_dine, max_dine;
char distribution[15];
int dining_count;

double generate_random_time(int min_time, int max_time) {
    if (strcmp(distribution, "uniform") == 0) {
        return min_time + rand() % (max_time - min_time + 1);
    } else {	// exponential distribution
        double mean = (min_time + max_time) / 2.0;
        double lambda = 1.0 / mean;
        double random_value;
        do {
            random_value = -log(1.0 - ((double)rand() / RAND_MAX)) / lambda;
        } while (random_value < min_time || random_value > max_time);
        return random_value;
    }
}

void pickup_chopsticks(int id) {
    int left = id;
    int right = (id + 1) % num_philosophers;
	// lock the start time for hunger
    pthread_mutex_lock(&mutex);
    clock_gettime(CLOCK_MONOTONIC, &philosophers[id].start_hungry_time);
    pthread_mutex_unlock(&mutex);
	// lock the chopsticks both sides
    pthread_mutex_lock(&chopsticks[left]);
    pthread_mutex_lock(&chopsticks[right]);
}
	// to put down chopsticks functiom
void return_chopsticks(int id) {
    int left = id;
    int right = (id + 1) % num_philosophers;

    pthread_mutex_unlock(&chopsticks[right]);
    pthread_mutex_unlock(&chopsticks[left]);
	// hungry duration
    struct timespec end_hungry_time;
    clock_gettime(CLOCK_MONOTONIC, &end_hungry_time);

    pthread_mutex_lock(&mutex);
    double hungry_duration = (end_hungry_time.tv_sec - philosophers[id].start_hungry_time.tv_sec) * 1000.0 +
                             (end_hungry_time.tv_nsec - philosophers[id].start_hungry_time.tv_nsec) / 1e6;
    philosophers[id].hungry_durations[philosophers[id].durations_recorded++] = hungry_duration;
    philosophers[id].total_hungry_duration += hungry_duration;
    printf("Philosopher %d was hungry for %.2f ms.\n", id, hungry_duration);
    pthread_mutex_unlock(&mutex);
}
	// to simulate thinking
void think(int id) {
    usleep(generate_random_time(min_think, max_think) * 1000);
}
	// to simulate dining
void dine(int id) {
    usleep(generate_random_time(min_dine, max_dine) * 1000);
    philosophers[id].num_times_dined++;
}

void *philosopher_routine(void *arg) {
    int id = *((int *)arg);
    for (int i = 0; i < dining_count; i++) {
        think(id);
        pickup_chopsticks(id);
        dine(id);
        return_chopsticks(id);
    }
    return NULL;
}
	// display statistics for each philosopher
void calculate_statistics() {

    for (int i = 0; i < num_philosophers; i++) {
    printf("Philosopher %d total hungry duration: %.2f ms\n", i, philosophers[i].total_hungry_duration);
        double average = philosophers[i].total_hungry_duration / philosophers[i].durations_recorded;
        double sum_sq_diff = 0;
        for (int j = 0; j < philosophers[i].durations_recorded; j++) {
            sum_sq_diff += pow(philosophers[i].hungry_durations[j] - average, 2);
        }
        double stddev = sqrt(sum_sq_diff / philosophers[i].durations_recorded);

        printf("Philosopher %d - Average Hungry Time: %f ms, Standard Deviation: %f ms\n", 
               philosophers[i].id, average, stddev);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 8) {
        fprintf(stderr, "Usage: %s <num_phsp> <min_think> <max_think> <min_dine> <max_dine> <dst> <num>\n", argv[0]);
        return 1;
    }

    num_philosophers = atoi(argv[1]);
    min_think = atoi(argv[2]);
    max_think = atoi(argv[3]);
    min_dine = atoi(argv[4]);
    max_dine = atoi(argv[5]);
    strcpy(distribution, argv[6]);
    dining_count = atoi(argv[7]);

    if (num_philosophers > MAX_PHILOSOPHERS || num_philosophers < 2) {
        fprintf(stderr, "Number of philosophers must be between 2 and %d.\n", MAX_PHILOSOPHERS);
        return 1;
    }

    srand(time(NULL));
	// initializing philosophers
    for (int i = 0; i < num_philosophers; i++) {
        pthread_mutex_init(&chopsticks[i], NULL);
        philosophers[i].id = i;
        philosophers[i].num_times_dined = 0;
        philosophers[i].total_hungry_duration = 0;
        philosophers[i].hungry_durations = malloc(dining_count * sizeof(double));
        philosophers[i].durations_recorded = 0;
    }
	// crateing philosopher threads
    for (int i = 0; i < num_philosophers; i++) {
        pthread_create(&threads[i], NULL, philosopher_routine, (void *)&philosophers[i].id);
    }

    for (int i = 0; i < num_philosophers; i++) {
        pthread_join(threads[i], NULL);
    }

    calculate_statistics();

    for (int i = 0; i < num_philosophers; i++) {
        free(philosophers[i].hungry_durations);
        pthread_mutex_destroy(&chopsticks[i]);
    }

    return 0;
}

