#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_STRING 100

typedef struct {
    char name[MAX_STRING];
    int num_cars, res;
    int pos_array;
} team_t;

typedef struct {
    int number, speed, reliability;
    double consuption;
    pthread_t thread;
    team_t * team;
} car_t;

typedef struct {
    int num_teams;
    int race_started;
    pthread_mutex_t mutex;
    pthread_cond_t new_command;
} shared_memory_t;


int shmid;
key_t shmkey;

#define MAX_TEAMS 3
#define MAX_CARS 2

shared_memory_t * shared;

team_t * get_teams(shared_memory_t * shared_memory) {
    return (team_t *) (shared_memory + 1);
}

car_t * get_cars(shared_memory_t * shared_memory) {
    return (car_t *) (get_teams(shared_memory) + MAX_TEAMS);
}

car_t * get_car(shared_memory_t * shared_memory, int team_pos, int car_pos){
    return (car_t *) (get_cars(shared_memory) + team_pos*MAX_CARS + car_pos);
}

void print_team(team_t * team) {// HelloThere
    printf("Team %s: %d %d %d\n", team->name, team->num_cars, team->res, team->pos_array);
}

void print_car (car_t * car){
    printf("Car %d Team %s %d %d %f\n", car->number, car->team->name, car->speed, car->reliability, car->consuption);
}

int main() {
    shmid = shmget(shmkey, sizeof(shared_memory_t) + sizeof(team_t) * MAX_TEAMS + sizeof(car_t) * MAX_CARS * MAX_TEAMS, IPC_CREAT|IPC_EXCL|0700);
    if(shmid < 1) {
        perror("Error with shmget: ");
        exit(1);
    }
    shared = (shared_memory_t *) shmat(shmid, NULL, 0);
    if(shared < (shared_memory_t *) 1) {
        perror("Error with shmat: ");
        exit(1);
    }

    /*if( (shmkey = ftok(".", getpid())) == (key_t) -1){
    perror("IPC error: ftok");
    exit(1);
    };
    
    if (shmid = shmget(shmkey, sizeof(shared_memory_t) + sizeof(team_t) * MAX_TEAMS + sizeof(car_t) * MAX_CARS * MAX_TEAMS, IPC_CREAT|IPC_EXCL|0700) < 1) {
		exit(1);
	}

	if ((shared = ((shared_memory_t *) shmat(shmid, NULL, 0)))< ((shared_memory_t *) 1)) {
        perror("Wee");
		exit(1);
	}*/
    team_t * teams = get_teams(shared);

    strcpy(teams[0].name, "Team A");
    strcpy(teams[1].name, "Team B");
    strcpy(teams[2].name, "Team C");
    teams[0].num_cars = 10;
    teams[1].num_cars = 11;
    teams[2].num_cars = 12;
    teams[0].pos_array = 0;
    teams[1].pos_array = 1;
    teams[2].pos_array = 2;
    teams[0].res = 20;
    teams[1].res = 21;
    teams[2].res = 22;
    
    

    car_t * car_a = get_car(shared, 0, 0);
    //car_t * car_a = get_cars(shared) + 0;
    car_a->number = 1;
    car_a->team = &teams[0];
    car_a->speed = 2;
    car_a->reliability = 3;
    car_a->consuption = 4;
    car_t * car_b = get_car(shared, 0, 1);
    //car_t * car_b = get_cars(shared) + 1;
    car_b->number = 5;
    car_b->team = &teams[0];
    car_b->speed = 6;
    car_b->reliability = 7;
    car_b->consuption = 8;
    car_t * car_c = get_car(shared, 1, 0);
    //car_t * car_c = get_cars(shared) + 2;
    car_c->number = 9;
    car_c->team = &teams[1];
    car_c->speed = 10;
    car_c->reliability = 11;
    car_c->consuption = 12;
    car_t * car_d = get_car(shared, 1, 1);
    //car_t * car_d = get_cars(shared) + 3;
    car_d->number = 13;
    car_d->team = &teams[1];
    car_d->speed = 14;
    car_d->reliability = 15;
    car_d->consuption = 16;
    car_t * car_e = get_car(shared,2, 0);
    //car_t * car_e = get_cars(shared) + 4;
    car_e->number = 17;
    car_e->team = &teams[2];
    car_e->speed = 18;
    car_e->reliability = 19;
    car_e->consuption = 20;
    car_t * car_f = get_car(shared,2, 1);
    //car_t * car_f = get_cars(shared) + 5;
    car_f->number = 21;
    car_f->team = &teams[2];
    car_f->speed = 22;
    car_f->reliability = 23;
    car_f->consuption = 24;

    for(int i = 0; i < 3; i++)
        print_team(&teams[i]);

    for(int i = 0; i < 6; i++)
        print_car(get_cars(shared) + i);
    
    shmdt(shared);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}