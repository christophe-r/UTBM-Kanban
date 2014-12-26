#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <pthread.h>

#include "structures.h"
#include "kanban.h"
#include "workstation.h"

#define OBSERVE_WORKSTATION		"Workstation 2"
#define OBSERVE_ALL_WORKSTATION	1

Workstation *create_workstation(char *name, unsigned short int processDelay){
	Workstation *workstation = calloc(1, sizeof(Workstation));

	//workstation->name = calloc(strlen(name)+1, sizeof(char*));
	workstation->name = name;
	workstation->processDelay = processDelay;
	
	// initilize kanban linked list
	workstation->todo = create_linkedlist();
	workstation->doing = NULL;
	workstation->done = create_linkedlist();

	// initilize container 
	workstation->containers0[0] = workstation->containers0[1] = NULL;
	workstation->containers1[0] = workstation->containers1[1] = NULL;


	// initilize mutex & condition 
	pthread_create(&(workstation->thread), 0, &workstation_thread, (void *) workstation);
	pthread_mutex_init(&(workstation->IDLE_mutex), 0);
	pthread_cond_init(&(workstation->IDLE_cond), 0);
	pthread_mutex_init(&(workstation->EmptyContainers_mutex), 0);
	pthread_cond_init(&(workstation->EmptyContainers_cond), 0);
	pthread_mutex_init(&(workstation->FullContainers_mutex), 0);
	pthread_cond_init(&(workstation->FullContainers_cond), 0);

	#ifdef DEBUG
		printf("Workstation \"%s\" created.\n", name);
		fflush(NULL);
	#endif
	
	return workstation;
}

void *workstation_thread(void *p_data){
	Workstation *thisWS = (Workstation *) p_data;
	consoleLog(thisWS, "Creation");
	fflush(NULL);

	int RemainigRessoucesInKanban = 0;

	while(1){// Main loop 

		pthread_mutex_lock(&(thisWS->IDLE_mutex)); // enter in a critical part
		if (RemainigRessoucesInKanban == 0 ) // if it's a new kanban or initialisation
		{			
			if (thisWS->doing != NULL) 
			{
				move_kanban_doing_to_done(thisWS); // move actual kaban to done
				consoleLog(thisWS, "Kanban finish");
			}
			if (thisWS->todo->nbKanban == 0 && thisWS->parents[0] != NULL && thisWS->parents[1] != NULL)
			{
				int nbEmptyContainers0 = 2 - count_full_container(thisWS->containers0);
				int nbEmptyContainers1 = 2 - count_full_container(thisWS->containers1);

				if (nbEmptyContainers0 != 0)
				{
					send_kanban(thisWS, thisWS->containers0, thisWS->parents[0],nbEmptyContainers0); // ask ressources to fill in his containers0
					pthread_cond_signal(&(thisWS->parents[0]->IDLE_cond));
					consoleLog(thisWS, "Send kanban to parent0 to fill in containers0");
				}
				if (nbEmptyContainers1 != 0)
				{
					send_kanban(thisWS, thisWS->containers1, thisWS->parents[1],nbEmptyContainers0); // ask ressources to fill in his containers1
					pthread_cond_signal(&(thisWS->parents[1]->IDLE_cond));
					consoleLog(thisWS, "Send kanban to parent1 to fill in containers1");
				}
			}

			while(thisWS->todo->nbKanban == 0){ // verifiy and wait a new kanban
				consoleLog(thisWS, "Wait until a new kanban arrive (IDLE_cond)");
				pthread_cond_wait(&(thisWS->IDLE_cond), &(thisWS->IDLE_mutex));
			}
			move_kanban_todo_to_doing(thisWS);
			consoleLog(thisWS, "Start a new kanban");

			if (thisWS->parents[0] != NULL && thisWS->parents[1] != NULL)
			{
				send_kanban(thisWS, thisWS->containers0, thisWS->parents[0],thisWS->doing->nbRessources); // transmit kanban to parents
				send_kanban(thisWS, thisWS->containers1, thisWS->parents[1],thisWS->doing->nbRessources);
				pthread_cond_signal(&(thisWS->parents[0]->IDLE_cond)); // wake up parents
				pthread_cond_signal(&(thisWS->parents[1]->IDLE_cond));
				
			}
			RemainigRessoucesInKanban = thisWS->doing->nbRessources;	
		}
		pthread_mutex_unlock(&(thisWS->IDLE_mutex));// end of the critical part

		pthread_mutex_lock(&(thisWS->EmptyContainers_mutex));// enter in a critical part

		Bool cantWork = TRUE;
		while(cantWork && thisWS->parents[0] != NULL && thisWS->parents[1] != NULL){
			if (count_full_container(thisWS->containers0) == 0 || count_full_container(thisWS->containers1) == 0)
			{
				consoleLog(thisWS, "Wait until containers are not empty (EmptyContainers_cond)");
				pthread_cond_wait(&(thisWS->EmptyContainers_cond), &(thisWS->EmptyContainers_mutex));
			}else{
				cantWork = FALSE;
			}
		}

		Ressource *ressource0 = take_ressource(thisWS->containers0, thisWS->parents[0]); // get current ressources
		Ressource *ressource1 = take_ressource(thisWS->containers1, thisWS->parents[1]);

		if (thisWS->parents[0] != NULL)
		{
			consoleLog(thisWS, "Send signal to wake up parent 0 (FullContainers_cond)");
			pthread_cond_signal(&(thisWS->parents[0]->FullContainers_cond)); // wake up parents
		}
		if (thisWS->parents[1] != NULL)
		{
			consoleLog(thisWS, "Send signal to wake up parent 1 (FullContainers_cond)");
			pthread_cond_signal(&(thisWS->parents[1]->FullContainers_cond)); // wake up parent
		}


		Ressource *ressource = calloc(1, sizeof(Ressource));
		ressource->elapsedTime = ressource0->elapsedTime + ressource1->elapsedTime;
		ressource->name = ""; // TODO
		consoleLog(thisWS, "Create new ressource");

		free(ressource0);//destroy old ressources
		free(ressource1);

		pthread_mutex_unlock(&(thisWS->EmptyContainers_mutex));// end of the critical part

		usleep(thisWS->processDelay * 100000);// simulate the time of production
		RemainigRessoucesInKanban--;

		pthread_mutex_lock(&(thisWS->FullContainers_mutex));// enter in a critical part

		if (count_full_container(thisWS->doing->fromContainerID) > 1)
		{
			consoleLog(thisWS, "Wait until containers are not full (FullContainers_cond)");
			pthread_cond_wait(&(thisWS->FullContainers_cond), &(thisWS->FullContainers_mutex));
		}

		place_ressource(thisWS->doing->fromContainerID, ressource); // place the ressource in the next container
		consoleLog(thisWS, "Ressource added to the kanban sender");
		pthread_mutex_unlock(&(thisWS->FullContainers_mutex));// end of the critical part
		
		consoleLog(thisWS, "Send signal to wake up the kanban sender (FullContainers_cond)");
		pthread_cond_signal(&(thisWS->doing->from->EmptyContainers_cond)); // wake up the WS who send the kanban

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
	Ressource *ressource;
	ressource = NULL;

	if (parent == NULL) // if parent doesn't exist create a kanban (low level Workstation)
	{
		ressource = calloc(1, sizeof(Ressource));
		ressource->elapsedTime = 0;
		ressource->name = "";// TODO
		return ressource;
	}

	if (containers != NULL)
	{
		ressource = containers[0];
		containers[0] = NULL;
		return ressource;

	}else if(containers[1] != NULL){
		ressource = containers[1];
		containers[1] = NULL;
		return ressource;
	}else{
		printf("Warning: The container is empty");
		return ressource;
	}
}

int place_ressource(Ressource *containers[], Ressource *ressource){
	if (containers[0] == NULL)
	{
		containers[0] = ressource;
		return 0;

	}else if(containers[1] == NULL){
		containers[1] = ressource;
		return 1;
	}else{
		return -1;
	}
}

int count_full_container(Ressource *containers[]){// calculate the amount of full container
	int nbFullContainer = 0;
	nbFullContainer = (containers[0] != NULL) ? nbFullContainer+1 : nbFullContainer; 
	nbFullContainer = (containers[1] != NULL) ? nbFullContainer+1 : nbFullContainer;
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

/**** Display function ****/

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
	if (strcmp(workstation->name, OBSERVE_WORKSTATION) == 0 || OBSERVE_ALL_WORKSTATION){
		printf("[%s] ", workstation->name);
		printf("%s\n", txt);
	}
}



/**** Destroy functions ****/

void destroy_workstation(Workstation *workstation){

	#ifdef DEBUG
		printf("Workstation \"%s\" freed.\n", workstation->name);
		fflush(NULL);
	#endif

	destroy_linkedlist(workstation->todo); // destroy kanban list
	free(workstation->doing);
	destroy_linkedlist(workstation->done);

	pthread_mutex_destroy(&workstation->IDLE_mutex); // destroy mutex and condition
	pthread_cond_destroy(&workstation->IDLE_cond);
	pthread_mutex_destroy(&workstation->EmptyContainers_mutex);
	pthread_cond_destroy(&workstation->EmptyContainers_cond);
	pthread_mutex_destroy(&workstation->FullContainers_mutex);
	pthread_cond_destroy(&workstation->FullContainers_cond);

	free(workstation->containers0[0]);// detroy ressources
	free(workstation->containers0[1]);
	free(workstation->containers1[0]);
	free(workstation->containers1[1]);

	free(workstation);
}




