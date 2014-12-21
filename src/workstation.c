#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <pthread.h>

#include "structures.h"
#include "kanban.h"
#include "workstation.h"


Workstation *create_workstation(char *name, unsigned short int processDelay){
	Workstation *workstation = calloc(1, sizeof(Workstation));

	//workstation->name = calloc(strlen(name)+1, sizeof(char*));
	workstation->name = name;
	workstation->processDelay = processDelay;
	
	workstation->todo = create_linkedlist();
	workstation->doing = NULL;
	workstation->done = create_linkedlist();


	pthread_create(&(workstation->thread), 0, &workstation_thread, (void *) workstation);
	pthread_mutex_init(&(workstation->mutex), 0);
	pthread_cond_init(&(workstation->cond), 0);

	#ifdef DEBUG
		printf("Workstation \"%s\" created.\n", name);
		fflush(NULL);
	#endif
	
	return workstation;
}

void *workstation_thread(void *p_data){
	Workstation *thisWorkstation = (Workstation *) p_data;

	printf("I'm a thread : \"%s\"\n", thisWorkstation->name);
	fflush(NULL);

	
	// TEST : Show test messages only for Workstation 2 (easier to understand what happens in terminal)
	if( strcmp(thisWorkstation->name, "Workstation 2") == 0 ){

		display_workstation(thisWorkstation);
		
		// Creates a kanban, from this WS, to parents[0] of this WS, with qty of 1
		Kanban *kanban1 = create_kanban(thisWorkstation, thisWorkstation->containers0, thisWorkstation->parents[0], 1);
		// Creates a kanban, from this WS, to parents[1] of this WS, with qty of 4
		Kanban *kanban2 = create_kanban(thisWorkstation, thisWorkstation->containers1, thisWorkstation->parents[1], 4);

		// Pushes kanban 1 & 2 in TODO list
		push(thisWorkstation->todo, kanban1);
		push(thisWorkstation->todo, kanban2);



		printf("[%s] TODO list : %d kanbans\n", thisWorkstation->name, thisWorkstation->todo->nbKanban);
		//print_list(thisWorkstation->todo);
		printf("[%s] DOING kanban :\n", thisWorkstation->name);
		print_kanban(thisWorkstation->doing);
		printf("[%s] DONE list : %d kanbans\n", thisWorkstation->name, thisWorkstation->done->nbKanban);
		//print_list(thisWorkstation->done);
		printf("**************************************\n");


		// Moves first kanban in TODO to DOING by removing the first TODO kanban and put it in doing
		move_kanban_todo_to_doing(thisWorkstation); 

		
		printf("[%s] TODO list : %d kanbans\n", thisWorkstation->name, thisWorkstation->todo->nbKanban);
		//print_list(thisWorkstation->todo);
		printf("[%s] DOING kanban :\n", thisWorkstation->name);
		print_kanban(thisWorkstation->doing);
		printf("[%s] DONE list : %d kanbans\n", thisWorkstation->name, thisWorkstation->done->nbKanban);
		//print_list(thisWorkstation->done);
		printf("**************************************\n");


		// Moves the DOING kanban in the DONE list
		move_kanban_doing_to_done(thisWorkstation);


		printf("[%s] TODO list : %d kanbans\n", thisWorkstation->name, thisWorkstation->todo->nbKanban);
		//print_list(thisWorkstation->todo);
		printf("[%s] DOING kanban :\n", thisWorkstation->name);
		print_kanban(thisWorkstation->doing);
		printf("[%s] DONE list : %d kanbans\n", thisWorkstation->name, thisWorkstation->done->nbKanban);
		//print_list(thisWorkstation->done);
		printf("**************************************\n");


		// Sends a kanban from this WS, to parents[0] of this WS, with a containerID=0, and qty of 20
		send_kanban(thisWorkstation, thisWorkstation->containers0, thisWorkstation->parents[0], 20);



	}

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

	destroy_linkedlist(workstation->todo);
	free(workstation->doing);
	destroy_linkedlist(workstation->done);



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


