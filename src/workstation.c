#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#include "structures.h"
#include "kanban.h"
#include "workstation.h"

#define OBSERVE_WORKSTATION			"Workstation up     0"
#define OBSERVE_ALL_WORKSTATIONS		0

Workstation *create_workstation(char *name, unsigned short int processDelay){
	Workstation *workstation = calloc(1, sizeof(Workstation));

	workstation->name = name;
	workstation->processDelay = processDelay;
	
	// Initializes linked-lists for kanbans
	workstation->todo = create_linkedlist();
	workstation->doing = NULL;
	workstation->done = create_linkedlist();

	// Initializes containers 
	workstation->containers0[0] = workstation->containers0[1] = NULL;
	workstation->containers1[0] = workstation->containers1[1] = NULL;

	// Initialize mutex & conditions 
	pthread_mutex_init(&(workstation->mutex_IDLE), 0);
	pthread_cond_init(&(workstation->cond_IDLE), 0);
	pthread_mutex_init(&(workstation->mutex_EmptyContainers), 0);
	pthread_cond_init(&(workstation->cond_EmptyContainers), 0);
	pthread_mutex_init(&(workstation->mutex_FullContainers), 0);
	pthread_cond_init(&(workstation->cond_FullContainers), 0);

	// Thread creation
	pthread_create(&(workstation->thread), 0, &workstation_thread, (void *) workstation);

	#ifdef DEBUG
		printf("Workstation \"%s\" created.\n", name);
		fflush(NULL);
	#endif
	
	return workstation;
}

void *workstation_thread(void *p_data){
	Workstation *thisWS = (Workstation *) p_data;
	consoleLog(thisWS, "Creation");

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	

	int RemainingRessourcesInKanban = 0;

	while(1){ // Main loop 

		pthread_mutex_lock(&(thisWS->mutex_IDLE)); // entering in a critical part
		if (RemainingRessourcesInKanban == 0 ){ // if it's a new kanban or initialization

			if( thisWS->doing != NULL ){
				move_kanban_doing_to_done(thisWS); // moves current kanban to done
				consoleLog(thisWS, "Kanban finished");
			}

			if( thisWS->todo->nbKanban == 0 && thisWS->parents[0] != NULL && thisWS->parents[1] != NULL ){
				int nbEmptyContainers0 = 2 - count_full_container(thisWS->containers0);
				int nbEmptyContainers1 = 2 - count_full_container(thisWS->containers1);

				if( nbEmptyContainers0 != 0 ){
					send_kanban(thisWS, thisWS->containers0, thisWS->parents[0], nbEmptyContainers0); // asks ressources to fill in his containers0
					pthread_cond_signal(&(thisWS->parents[0]->cond_IDLE));
					consoleLog(thisWS, "Send kanban to parent0 to fill in containers0");
				}

				if( nbEmptyContainers1 != 0 ){
					send_kanban(thisWS, thisWS->containers1, thisWS->parents[1], nbEmptyContainers0); // asks ressources to fill in his containers1
					pthread_cond_signal(&(thisWS->parents[1]->cond_IDLE));
					consoleLog(thisWS, "Send kanban to parent1 to fill in containers1");
				}
			}

			while( thisWS->todo->nbKanban == 0 ){ // verifies and waits for a new kanban
				consoleLog(thisWS, "Waiting until a new kanban arrives (cond_IDLE)");
				pthread_cond_wait(&(thisWS->cond_IDLE), &(thisWS->mutex_IDLE));
			}

			move_kanban_todo_to_doing(thisWS);
			consoleLog(thisWS, "Starting a new kanban");

			if( thisWS->parents[0] != NULL && thisWS->parents[1] != NULL ){
				send_kanban(thisWS, thisWS->containers0, thisWS->parents[0], thisWS->doing->nbRessources); // transmits kanban to parents
				send_kanban(thisWS, thisWS->containers1, thisWS->parents[1], thisWS->doing->nbRessources);
				pthread_cond_signal(&(thisWS->parents[0]->cond_IDLE)); // wakes up parents
				pthread_cond_signal(&(thisWS->parents[1]->cond_IDLE));
				
			}

			RemainingRessourcesInKanban = thisWS->doing->nbRessources;	
		}

		pthread_mutex_unlock(&(thisWS->mutex_IDLE)); // end of the critical part

		pthread_mutex_lock(&(thisWS->mutex_EmptyContainers)); // entering in a critical part

		bool cantWork = true;
		while( cantWork && thisWS->parents[0] != NULL && thisWS->parents[1] != NULL ){
			if (count_full_container(thisWS->containers0) == 0 || count_full_container(thisWS->containers1) == 0){
				consoleLog(thisWS, "Wait until containers are not empty (cond_EmptyContainers)");
				pthread_cond_wait(&(thisWS->cond_EmptyContainers), &(thisWS->mutex_EmptyContainers));
			} else {
				cantWork = false;
			}
		}

		Ressource *ressource0 = take_ressource(thisWS->containers0, thisWS->parents[0]); // gets current ressources
		Ressource *ressource1 = take_ressource(thisWS->containers1, thisWS->parents[1]);

		if( thisWS->parents[0] != NULL ){
			consoleLog(thisWS, "Send signal to wake up parent 0 (cond_FullContainers)");
			pthread_cond_signal(&(thisWS->parents[0]->cond_FullContainers)); // wakes up parents
		}

		if( thisWS->parents[1] != NULL ){
			consoleLog(thisWS, "Send signal to wake up parent 1 (cond_FullContainers)");
			pthread_cond_signal(&(thisWS->parents[1]->cond_FullContainers)); // wakes up parents
		}


		Ressource *ressource = calloc(1, sizeof(Ressource));
		ressource->elapsedTime = ressource0->elapsedTime + ressource1->elapsedTime;
		ressource->name = ""; // TODO
		consoleLog(thisWS, "Create new ressource");

		free(ressource0); //destroys old ressources
		free(ressource1);

		pthread_mutex_unlock(&(thisWS->mutex_EmptyContainers)); // end of the critical part

		usleep(thisWS->processDelay * 100000); // simulates the time of the production
		RemainingRessourcesInKanban--;

		pthread_mutex_lock(&(thisWS->mutex_FullContainers)); // entering in a critical part

		if( count_full_container(thisWS->doing->fromContainerID) > 1 ){
			consoleLog(thisWS, "Wait until containers are not full (cond_FullContainers)");
			pthread_cond_wait(&(thisWS->cond_FullContainers), &(thisWS->mutex_FullContainers));
		}

		put_ressource(thisWS->doing->fromContainerID, ressource); // puts the ressource in the next container
		consoleLog(thisWS, "Ressource added to the kanban sender");
		pthread_mutex_unlock(&(thisWS->mutex_FullContainers)); // end of the critical part
		
		consoleLog(thisWS, "Send signal to wake up the kanban sender (cond_FullContainers)");
		pthread_cond_signal(&(thisWS->doing->from->cond_EmptyContainers)); // wakes up the WS which have sent the kanban

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


Ressource *take_ressource(Ressource *containers[], Workstation *parent){
	Ressource *ressource = NULL;

	if( parent == NULL ){ // if parent doesn't exist, create a kanban (low level Workstation)
		ressource = calloc(1, sizeof(Ressource));
		ressource->elapsedTime = 0;
		ressource->name = ""; // TODO
		return ressource;
	}

	if( containers != NULL ){
		ressource = containers[0];
		containers[0] = NULL;
		return ressource;
	} else if( containers[1] != NULL ){
		ressource = containers[1];
		containers[1] = NULL;
		return ressource;
	} else {
		printf("Warning: The container is empty\n");
		return ressource;
	}
}


int put_ressource(Ressource *containers[], Ressource *ressource){
	if( containers[0] == NULL ){
		containers[0] = ressource;
		return 0;
	} else if( containers[1] == NULL ){
		containers[1] = ressource;
		return 1;
	} else {
		return -1;
	}
}

int count_full_container(Ressource *containers[]){ // Returns the amount of full containers
	int nbFullContainer = 0;
	if (containers[0] != NULL) nbFullContainer++;
	if (containers[1] != NULL) nbFullContainer++;

	return nbFullContainer;
}

void move_kanban_todo_to_doing(Workstation *workstation){
	
	if( workstation->doing == NULL ){
		workstation->doing = pop(workstation->todo);
	}
	#ifdef DEBUG
	else {
		printf("Warning: [%s] Can't move kanban from TODO to DOING because a kanban exists in DOING.\n", workstation->name);
	}
	#endif
	
}

void move_kanban_doing_to_done(Workstation *workstation){
	push(workstation->done, workstation->doing);	// Put the DOING kanban in the DONE list
	workstation->doing = NULL;						// and reset the DOING kanban
}


/**** Display functions ****/

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

void consoleLog(Workstation *workstation, char *txt){
	if( strcmp(workstation->name, OBSERVE_WORKSTATION) == 0 || OBSERVE_ALL_WORKSTATIONS ){
		printf("[%s] %s\n", workstation->name, txt);
		fflush(NULL);
	}
}



/**** Destroy functions ****/

void destroy_workstation(Workstation *workstation){

	#ifdef DEBUG
		printf("Workstation \"%s\" freed.\n", workstation->name);
		fflush(NULL);
	#endif

	destroy_linkedlist(workstation->todo); // destroys kanban todo list
	free(workstation->doing);
	destroy_linkedlist(workstation->done);

	// destroys mutex and conditions
	pthread_mutex_destroy(&workstation->mutex_IDLE);
	pthread_cond_destroy(&workstation->cond_IDLE);
	pthread_mutex_destroy(&workstation->mutex_EmptyContainers);
	pthread_cond_destroy(&workstation->cond_EmptyContainers);
	pthread_mutex_destroy(&workstation->mutex_FullContainers);
	pthread_cond_destroy(&workstation->cond_FullContainers);

	// Destroys ressources
	free(workstation->containers0[0]);
	free(workstation->containers0[1]);
	free(workstation->containers1[0]);
	free(workstation->containers1[1]);

	free(workstation);
}




