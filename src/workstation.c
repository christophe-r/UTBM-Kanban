#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <pthread.h>

#include "structures.h"
#include "workstation.h"


Workstation *create_workstation(char *name){
	Workstation *workstation = calloc(1, sizeof(Workstation));

	//workstation->name = calloc(strlen(name)+1, sizeof(char*));
	workstation->name = name;

	pthread_create(&(workstation->thread), 0, &workstation_thread, NULL);
	pthread_mutex_init(&(workstation->mutex), 0);
	pthread_cond_init(&(workstation->cond), 0);

	#ifdef DEBUG
		printf("Workstation \"%s\" created.\n", name);
		fflush(NULL);
	#endif
	
	return workstation;
}

void *workstation_thread(void *param){
	printf("I'm a thread\n");
	fflush(NULL);
	pthread_exit(NULL);

	return (void*)NULL;
}


void link_workstations(Workstation *workstation, Workstation *parent){

	if( workstation->parents[0] == NULL ){
		workstation->parents[0] = parent;
	} else if ( workstation->parents[1] == NULL ){
		workstation->parents[1] = parent;
	}
	#ifdef DEBUG
	else {
		printf("Warning: This workstation already reached the maximum number of parents.\n");
		fflush(NULL);
	}
	#endif
}

void display_workstation(Workstation *workstation){

	printf("-------------------------------------------\n");
	printf("Name      : %s\n", workstation->name);

	if( workstation->parents[0] != NULL ){
		printf("Parent 1  : %s\n", workstation->parents[0]->name);
	}

	if( workstation->parents[1] != NULL ){
		printf("Parent 2  : %s\n", workstation->parents[1]->name);
	}

	printf("Ressources: TODO\n");
	printf("-------------------------------------------\n");

}


/**** Destroy functions ****/

void destroy_workstation(Workstation *workstation){

	#ifdef DEBUG
		printf("Workstation \"%s\" freed.\n", workstation->name);
		fflush(NULL);
	#endif

	pthread_mutex_destroy(&workstation->mutex);
	pthread_cond_destroy(&workstation->cond);
	free(workstation);
}

void destroy_all_workstations(Workstation *factory[], int nb_workstations){
	int i;
	for(i=0 ; i<nb_workstations ; i++ ){
		//printf("i=%d, addr: %d, addr: %d\n", i, (factory+i*sizeof(Workstation)), sizeof(Workstation) );
		//destroy_workstation(*(factory+i));
		destroy_workstation(factory[i]);
	}
}


