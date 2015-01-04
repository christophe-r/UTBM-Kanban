#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#include "structures.h"
#include "kanban.h"
#include "workstation.h"

#include "config.h"


Resource *FinalContainers[FINAL_CONTAINERS_CAPACITY]; // save finished items
bool factoryState = false;

pthread_mutex_t mutex_factoryState;



void start_factory(){
	pthread_mutex_lock(&mutex_factoryState);
	factoryState = true;
	pthread_mutex_unlock(&mutex_factoryState);
}


Workstation *create_workstation(char *name, unsigned short int processDelay, bool launcher){
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

	// Initializes parents
	workstation->parents[0] = workstation->parents[1] = NULL;

	// Initialize mutex & conditions 
	pthread_mutex_init(&(workstation->mutex_IDLE), 0);
	pthread_cond_init(&(workstation->cond_IDLE), 0);
	pthread_mutex_init(&(workstation->mutex_EmptyContainers), 0);
	pthread_cond_init(&(workstation->cond_EmptyContainers), 0);
	pthread_mutex_init(&(workstation->mutex_FullContainers), 0);
	pthread_cond_init(&(workstation->cond_FullContainers), 0);

	// Thread creation
	if( launcher == true ){
		pthread_create(&(workstation->thread), 0, &customer_thread, (void *) workstation);
	} else {
		pthread_create(&(workstation->thread), 0, &workstation_thread, (void *) workstation);
	}
	
	#ifdef DEBUG
		printf("Workstation \"%s\" created.\n", name);
		fflush(NULL);
	#endif
	
	return workstation;
}

void *customer_thread(void *p_data){

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	int index = 0;

	Workstation *thisWS = (Workstation *) p_data;
	consoleLogRoot(thisWS, "Creating and enable customer...");

	FinalContainers[0] = NULL;

	while(1){
		pthread_mutex_lock(&(thisWS->mutex_EmptyContainers)); // entering in a critical part
		pthread_cond_wait(&(thisWS->cond_EmptyContainers), &(thisWS->mutex_EmptyContainers));

		if( thisWS->todo->nbKanban != 0 ){
			move_kanban_todo_to_doing(thisWS);

			if( thisWS->parents[0] != NULL ){
				consoleLogRoot(thisWS, "Order transmited to the final workstation");
				send_kanban(thisWS, thisWS->containers0, thisWS->parents[0], thisWS->doing->nbResources); // transmits kanban to parent
				pthread_cond_signal(&(thisWS->parents[0]->cond_IDLE)); // wakes up parent
			}

			// normally never use
			if( thisWS->parents[1] != NULL ){
				consoleLogRoot(thisWS, "Kanban transmited (1)");
				send_kanban(thisWS, thisWS->containers1, thisWS->parents[1], thisWS->doing->nbResources); // transmits kanban to parent
				pthread_cond_signal(&(thisWS->parents[1]->cond_IDLE)); // wakes up parent
			}

			move_kanban_doing_to_done(thisWS);
		}

		while( count_full_container(thisWS->containers0) != 0 ){ // save resource
			consoleLogRoot(thisWS, "Resource stored to the final containers.");
			FinalContainers[index] = take_resource(thisWS->containers0, thisWS);
			index++;
			FinalContainers[index] = NULL;
		}

		// normally never use
		while( count_full_container(thisWS->containers1) != 0 ){ // save resource
			consoleLogRoot(thisWS, "Resource stored to the final containers.");
			FinalContainers[index] = take_resource(thisWS->containers1, thisWS);
			index++;
			FinalContainers[index] = NULL;
		}

		pthread_mutex_unlock(&(thisWS->mutex_EmptyContainers)); // end of the critical part
	}

	pthread_exit(NULL);
}


void *workstation_thread(void *p_data){


	Workstation *thisWS = (Workstation *) p_data;
	consoleLog(thisWS, "Creating workstation...");

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	
	int RemainingResourcesInKanban = 0;

	pthread_mutex_lock(&mutex_factoryState);
	while( factoryState == false ){
		pthread_mutex_unlock(&mutex_factoryState);
		usleep(100000);
	}
	consoleLog(thisWS, "Workstation enabled...");

	// optimize workstation (allways full)
	int nbEmptyContainers0 = 2 - count_full_container(thisWS->containers0);
	int nbEmptyContainers1 = 2 - count_full_container(thisWS->containers1);

	if( nbEmptyContainers0 != 0 && thisWS->parents[0] != NULL){
		send_kanban(thisWS, thisWS->containers0, thisWS->parents[0], nbEmptyContainers0); // asks resources to fill in his containers0
		pthread_cond_signal(&(thisWS->parents[0]->cond_IDLE));
		consoleLog(thisWS, "Send kanban to parent0 to fill in containers0");
	}

	if( nbEmptyContainers1 != 0 && thisWS->parents[1] != NULL ){
		send_kanban(thisWS, thisWS->containers1, thisWS->parents[1], nbEmptyContainers0); // asks resources to fill in his containers1
		pthread_cond_signal(&(thisWS->parents[1]->cond_IDLE));
		consoleLog(thisWS, "Send kanban to parent1 to fill in containers1");
	}

	while(1){ // Main loop 

		// consoleLog(thisWS, "lock mutex_IDLE");
		pthread_mutex_lock(&(thisWS->mutex_IDLE)); // entering in a critical part
		if (RemainingResourcesInKanban == 0 ){ // if it's a new kanban or initialization

			if( thisWS->doing != NULL ){
				move_kanban_doing_to_done(thisWS); // moves current kanban to done
				consoleLog(thisWS, "Kanban finished");
			}

			while( thisWS->todo->nbKanban == 0 ){ // verifies and waits for a new kanban
				consoleLog(thisWS, "Waiting until a new kanban arrives (cond_IDLE)");
				pthread_cond_wait(&(thisWS->cond_IDLE), &(thisWS->mutex_IDLE));
			}

			move_kanban_todo_to_doing(thisWS);
			consoleLog(thisWS, "Starting a new kanban");

			if( thisWS->parents[0] != NULL ){
				send_kanban(thisWS, thisWS->containers0, thisWS->parents[0], thisWS->doing->nbResources); // transmits kanban to parent
				pthread_cond_signal(&(thisWS->parents[0]->cond_IDLE)); // wakes up parent
			}
			if( thisWS->parents[1] != NULL ){
				send_kanban(thisWS, thisWS->containers1, thisWS->parents[1], thisWS->doing->nbResources); // transmits kanban to parent
				pthread_cond_signal(&(thisWS->parents[1]->cond_IDLE));// wake up parent
			}

			RemainingResourcesInKanban = thisWS->doing->nbResources;	
		}

		pthread_mutex_unlock(&(thisWS->mutex_IDLE)); // end of the critical part

		// consoleLog(thisWS, "lock mutex_EmptyContainers");
		pthread_mutex_lock(&(thisWS->mutex_EmptyContainers)); // entering in a critical part

		bool cantWork = true;
		while( cantWork ){
			if (count_full_container(thisWS->containers0) == 0 && thisWS->parents[0] != NULL){
				consoleLog(thisWS, "Waiting until containers 0 are not empty (cond_EmptyContainers)");
				pthread_cond_wait(&(thisWS->cond_EmptyContainers), &(thisWS->mutex_EmptyContainers));
			} else if (count_full_container(thisWS->containers1) == 0  && thisWS->parents[1] != NULL ){
				consoleLog(thisWS, "Waiting until containers 1 are not empty (cond_EmptyContainers)");
				pthread_cond_wait(&(thisWS->cond_EmptyContainers), &(thisWS->mutex_EmptyContainers));
			} else {
				cantWork = false;
			}
		}

		Resource *resource0 = take_resource(thisWS->containers0, thisWS->parents[0]); // gets current resources
		Resource *resource1 = take_resource(thisWS->containers1, thisWS->parents[1]);

		if( thisWS->parents[0] != NULL ){
			consoleLog(thisWS, "Send signal to wake up parent 0 (cond_FullContainers)");
			pthread_cond_signal(&(thisWS->parents[0]->cond_FullContainers)); // wakes up parent
		}

		if( thisWS->parents[1] != NULL ){
			consoleLog(thisWS, "Send signal to wake up parent 1 (cond_FullContainers)");
			pthread_cond_signal(&(thisWS->parents[1]->cond_FullContainers)); // wakes up parent
		}

		Resource *resource = calloc(1, sizeof(Resource));
		if (resource == NULL || resource0 == NULL || resource1 == NULL ){
			consoleLog(thisWS, "Error: Resource not initialized\n");
			printf("resource : %p\n", resource);
			printf("resource0 : %p\n", resource0);
			printf("resource1 : %p\n", resource1);
		}

		resource->elapsedTime = resource0->elapsedTime + resource1->elapsedTime + thisWS->processDelay;
		resource->name = ""; // TODO
		consoleLog(thisWS, "Creating new resource");

		free(resource0); //destroys old resources
		free(resource1);

		pthread_mutex_unlock(&(thisWS->mutex_EmptyContainers)); // end of the critical part

		consoleLog(thisWS, "\e[91mWorking...\e[39m");
		usleep(thisWS->processDelay * WORKTIME_SCALE); // simulates the time of the production
		RemainingResourcesInKanban--;

		// consoleLog(thisWS, "lock mutex_FullContainers");
		pthread_mutex_lock(&(thisWS->mutex_FullContainers)); // entering in a critical part

		if( count_full_container(thisWS->doing->fromContainerID) > 1 ){
			consoleLog(thisWS, "Waiting until containers are not full (cond_FullContainers)");
			pthread_cond_wait(&(thisWS->cond_FullContainers), &(thisWS->mutex_FullContainers));
		}

		put_resource(thisWS->doing->fromContainerID, resource); // puts the resource in the next container
		consoleLog(thisWS, "Resource added to the kanban sender");
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


Resource *take_resource(Resource *containers[], Workstation *attachedParent){
	Resource *resource = NULL;

	if( attachedParent == NULL ){ // if parent doesn't exist, create a kanban (low level Workstation)
		resource = calloc(1, sizeof(Resource));
		resource->elapsedTime = 0;
		resource->name = ""; // TODO
		// printf("Warning: Creating new resource\n");
		return resource;
	}

	if( containers[0] != NULL ){
		resource = containers[0];
		containers[0] = NULL;
		// printf("returning container 0 : %p\n", resource);
		return resource;
	} else if( containers[1] != NULL ){
		resource = containers[1];
		containers[1] = NULL;
		// printf("returning container 1 : %p\n", resource);
		return resource;
	} else {
		printf("Warning: The container is empty\n");
		return resource;
	}
}


int put_resource(Resource *containers[], Resource *resource){
	if( containers[0] == NULL ){
		containers[0] = resource;
		return 0;
	} else if( containers[1] == NULL ){
		containers[1] = resource;
		return 1;
	} else {
		return -1;
	}
}

int count_full_container(Resource *containers[]){ // Returns the amount of full containers
	int nbFullContainer = 0;
	if (containers[0] != NULL) nbFullContainer++;
	if (containers[1] != NULL) nbFullContainer++;
	// printf("nb full container %d.\n", nbFullContainer);
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

	printf("Resources: TODO\n");
	printf("-------------------------------------------\n");

}

void consoleLog(Workstation *workstation, char *txt){
	//usleep(10000); // 10 ms
	if( strcmp(workstation->name, OBSERVE_WORKSTATION) == 0 || OBSERVE_ALL_WORKSTATIONS ){
		printf("\e[1m[%s]\e[21m %s\n", workstation->name, txt);
		fflush(NULL);
	}
}
void consoleLogRoot(Workstation *workstation, char *txt){
		printf("\e[7m[%s] %s\e[27m\n", workstation->name, txt);
		fflush(NULL);

}

void displayFinalItems(){
	int k = 0;
	while(FinalContainers[k] != NULL){
		printf("	[Item %d] Elapsed time : %d\n", k+1, FinalContainers[k]->elapsedTime);
		k++;
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

	// Destroys resources
	free(workstation->containers0[0]);
	free(workstation->containers0[1]);
	free(workstation->containers1[0]);
	free(workstation->containers1[1]);

	free(workstation);
}




