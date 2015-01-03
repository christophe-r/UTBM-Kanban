#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>

#include "structures.h"
#include "workstation.h"
#include "factory.h"
#include "kanban.h"
#include "simulation.h"

#include "config.h"


// Creates the factory (array of workstations)
Workstation *factory[NB_WORKSTATIONS];
// Creates the customer
Workstation *customer; // Special workstation
TimeLineEvent *userTimeline[TIMELINE_EVENTS_CAPACITY]; // Timeline

pthread_t timeline;
bool simulationStarted = false;

extern Resource *FinalContainers[]; // From workstation.c

void sigintHandler(int sig_num){
	int i = 0;

	if( simulationStarted == false ){
		exit(0);
	}

	printf("\n\nSig Int catched.\n");
	printf("Final items:\n");
	displayFinalItems();

	printf("Destroying final resources...");
	for( i=0; i<FINAL_CONTAINERS_CAPACITY; i++ ){
		free(FinalContainers[i]);
	}

	printf("\nDestroying timeline...\n");
	for( i=0; i<TIMELINE_EVENTS_CAPACITY; i++ ){
		free(userTimeline[i]);
	}

	printf("Destroying workstations and exit...\n");
	for( i=0; i<NB_WORKSTATIONS; i++ ){
		pthread_cancel(factory[i]->thread);
	}
	pthread_cancel(timeline);
	pthread_cancel(customer->thread);

	// Joins workstation's threads and timeline
	join_threads_workstations(factory, NB_WORKSTATIONS);
	pthread_join(timeline, NULL);
	printf("Timeline thread joined.\n");
	pthread_join(customer->thread, NULL);
	printf("Workstation \"%s\" thread joined.\n", customer->name);

	// Destroys the workstations
	destroy_all_workstations(factory, NB_WORKSTATIONS);
	destroy_workstation(customer);

	printf("Program exited.\n");
    exit(0);
}

void runSimulation() {

	printf("Building factory...\n");
	simulationStarted = true;

	/*
	// Creates each workstation  
	factory[0] = create_workstation("Workstation up     1.1", 5, false);
	factory[1] = create_workstation("Workstation up     1.2", 5, false);
	factory[2] = create_workstation("Workstation middle 1",   5, false);
	factory[3] = create_workstation("Workstation down   2.1", 5, false);
	factory[4] = create_workstation("Workstation up     2.2", 5, false);
	factory[5] = create_workstation("Workstation middle 2",   5, false);
	factory[6] = create_workstation("Workstation down   0",   5, false);

	// Creates customer (special workstation)
	customer = create_workstation("Customer", 0, true);

	// Linking the workstations each others
	link_workstations(factory[2], factory[0]); // Workstation middle 1 -> Workstation up   1.1
	link_workstations(factory[2], factory[1]); // Workstation middle 1 -> Workstation up   1.1
	link_workstations(factory[5], factory[3]); // Workstation middle 2 -> Workstation up   2.1
	link_workstations(factory[5], factory[4]); // Workstation middle 2 -> Workstation up   2.2
	link_workstations(factory[6], factory[2]); // Workstation down   0 -> Workstation middle 1
	link_workstations(factory[6], factory[5]); // Workstation down   0 -> Workstation middle 2

	// Linking the customer to the final workstation
	link_workstations(customer, factory[6]); // Customer -> Workstation down   0
	*/

	// Creates each workstation  
	factory[0] = create_workstation("Cabin parts", 5, false);
	factory[1] = create_workstation("Wings parts", 8, false);
	factory[2] = create_workstation("Gears parts", 6, false);
	factory[3] = create_workstation("Engines parts", 10, false);

	factory[4] = create_workstation("Cabin assembly", 4, false);
	factory[5] = create_workstation("Wings assembly", 6, false);
	factory[6] = create_workstation("Gears assembly", 4, false);
	factory[7] = create_workstation("Engines assembly", 9, false);

	factory[8] = create_workstation("C+W assembly", 4, false);
	factory[9] = create_workstation("(C+W)+G assembly", 3, false);
	factory[10] = create_workstation("(C+W+G)+E assembly", 4, false);

	factory[11] = create_workstation("Aircraft painting", 4, false);
	factory[12] = create_workstation("Aircraft testing", 5, false);

	// Creates customer (special workstation)
	customer = create_workstation("Customer", 0, true);

	// Linking the workstations each others
	link_workstations(factory[4], factory[0]);
	link_workstations(factory[5], factory[1]);
	link_workstations(factory[6], factory[2]);
	link_workstations(factory[7], factory[3]);

	link_workstations(factory[8], factory[4]);
	link_workstations(factory[8], factory[5]);

	link_workstations(factory[9], factory[8]);
	link_workstations(factory[9], factory[6]);

	link_workstations(factory[10], factory[9]);
	link_workstations(factory[10], factory[7]);

	link_workstations(factory[11], factory[10]);

	link_workstations(factory[12], factory[11]);

	// Linking the customer to the final workstation
	link_workstations(customer, factory[12]);

	// Creates the timeline
	pthread_create(&timeline, 0, &timeline_thread, (void *) customer);

	// Joins workstation's threads and timeline
	join_threads_workstations(factory, NB_WORKSTATIONS);
	pthread_join(timeline, NULL);
 	
	// Destroys the workstations
	//destroy_all_workstations(factory, NB_WORKSTATIONS);

	return;
}

void buildTimeline(){
	int nbOrders = 0;
	int i = 0;

	userTimeline[0] = NULL;

	printf("\e[4mTimeline initialization:\e[24m\n");

	getUserEntry("How many orders do you want to send? (\"-1\" for default timeline) ", &nbOrders);
	while( nbOrders > TIMELINE_EVENTS_CAPACITY || nbOrders < -1 ){
		getUserEntry("How many orders do you want to send? ", &nbOrders);
	}

	if( nbOrders == -1 ){
		printf("Launching the default timeline\n");
		userTimeline[0] = addEventToTimeline(20, 2);
		userTimeline[1] = addEventToTimeline(3, 3);
		userTimeline[2] = addEventToTimeline(4, 1);
		userTimeline[3] = addEventToTimeline(6, 2);
		userTimeline[4] = NULL;

	} else {

		for( i = 0; i < nbOrders; i++ ){
			int waitingTime = 0;
			int kanbanQuantity = 0;
			printf("\e[1m[Order %d]\e[21m Time to wait (seconds): ", i+1);
			getUserEntry("", &waitingTime);
			printf("\e[1m[Order %d]\e[21m Order quantity: ", i+1);
			getUserEntry("", &kanbanQuantity);
			userTimeline[i] = addEventToTimeline(waitingTime, kanbanQuantity);
		}

		userTimeline[i+1] = NULL;
	}

	printf("\n");	

	i = 0;
	while(userTimeline[i] != NULL){ // displays the timeline
		printf("Pause of %d sec. --> ", userTimeline[i]->WaitingTime);
		printf("Order of %d item(s) --> ", userTimeline[i]->KanbanQuantity);
		i++;
	}
	printf("Finish.\n");
	
}


void *timeline_thread(void *p_data){

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	int i = 0;
	Workstation *customer = (Workstation *) p_data;
	printf("\e[7mLaunching timeline...\e[27m\n");

	usleep(1000000);

	while( userTimeline[i] != NULL ){
		usleep(userTimeline[i]->WaitingTime * 1000000);
		send_kanban(customer, customer->containers0, customer, userTimeline[i]->KanbanQuantity); // tramsmits kanban to customer thread
		pthread_cond_signal(&(customer->cond_EmptyContainers)); // wakes up customer thread
		i++;
	}

	usleep(2000000);
	printf("\e[7mTimeline finished.\e[27m\n");
	pthread_exit(NULL);
}

 
void getUserEntry(char *txt, int *value){ // get user keyboard input
	printf("%s", txt);
	scanf("%d", value);
}

TimeLineEvent *addEventToTimeline(int waitingTime, int kanbanQuantity){ // creates event in timeline
	TimeLineEvent *event = calloc(1, sizeof(TimeLineEvent));
	event->WaitingTime = waitingTime;
	event->KanbanQuantity = kanbanQuantity;

	return event;
}
