#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

#include "structures.h"
#include "workstation.h"
#include "factory.h"
#include "kanban.h"
#include "simulation.h"

#define NB_WORKSTATIONS		7


// Creates the factory (array of workstations)
Workstation *factory[NB_WORKSTATIONS];
TimeLineNode *userTimeLine[50];

pthread_t timeLine;
bool simulationStart = false;


void sigintHandler(int sig_num){
	int i=0;

	if (simulationStart == false){
		exit(0);
	}
	printf("\n\nSig Int :\n");
	printf("\nDisplay final ressources :\n");
	displayFinalRessources();

	printf("\nDestroying time line ...\n");
	while(userTimeLine[i] != NULL){
		free(userTimeLine[i]);
		i++;
	}

	printf("Destroying workstations and exit...\n");
	for( i=0; i<NB_WORKSTATIONS; i++ ){
		pthread_cancel(factory[i]->thread);
	}
	
	// Joins workstation's threads
	join_threads_workstations(factory, NB_WORKSTATIONS);
	pthread_join(timeLine, NULL);

	// Destroys the workstations
	destroy_all_workstations(factory, NB_WORKSTATIONS);

	printf("Program exited.\n");
    exit(0);
}

void runSimulation() {

	printf("Launching Factory\n");
	simulationStart = true;

	// Creates each workstation  
	factory[0] = create_workstation("Workstation base   1.1", 5,false);
	factory[1] = create_workstation("Workstation base   1.2", 5,false);
	factory[2] = create_workstation("Workstation middle 1",   5,false);
	// factory[7] = create_workstation("Workstation base   2.1", 5,false);
	factory[3] = create_workstation("Workstation base   2.2", 5,false);
	factory[4] = create_workstation("Workstation middle 2",   5,false);
	factory[5] = create_workstation("Workstation up     0",   5,false);
	factory[6] = create_workstation("Customer", 5,true);

	// Linking the workstations each others
	link_workstations(factory[2], factory[0]); // Workstation middle 1 -> Workstation base   1.1
	link_workstations(factory[2], factory[1]); // Workstation middle 1 -> Workstation base   1.1
	// link_workstations(factory[4], factory[7]); // Workstation middle 2 -> Workstation base   2.1
	link_workstations(factory[4], factory[3]); // Workstation middle 2 -> Workstation base   2.2
	link_workstations(factory[5], factory[2]); // Workstation up     0 -> Workstation middle 1
	link_workstations(factory[5], factory[4]); // Workstation up     0 -> Workstation middle 2

	link_workstations(factory[6], factory[5]); // customer -> Workstation up     0 

	pthread_create(&timeLine, 0, &timeLine_thread, (void *) factory[6]);

	// Joins workstation's threads
	join_threads_workstations(factory, NB_WORKSTATIONS);
	pthread_join(timeLine, NULL);
 	
	// Destroys the workstations
	//destroy_all_workstations(factory, NB_WORKSTATIONS);

	return;
}

void buildTimeline(){
	int nborders= 0;
	int i = 0;

	userTimeLine[0] = NULL;

	printf("Initialisation of the time line \n");
	getUserEntry("Numbers of orders to send to the production line (0 for default time line) :", &nborders);
	if (nborders == 0 )
	{
		printf("Launching default time line\n");
		userTimeLine[0] = addNodeToTimeline(10, 2);
		userTimeLine[1] = addNodeToTimeline(3, 3);
		userTimeLine[2] = addNodeToTimeline(4, 1);
		userTimeLine[3] = addNodeToTimeline(6, 2);
		userTimeLine[4] = NULL;

		while(userTimeLine[i] != NULL){
			printf("%d sec pause --> ",userTimeLine[i]->WaitingTime);
			printf("send a order for %d items --> ",userTimeLine[i]->KanbanQuantities);
			i++;
		}
		printf("Finish \n");
		return;
	}	

	for (i = 0; i < nborders; ++i)
	{
		int waitingTime = 0;
		int kanbanQuantities = 0;
		printf("Waiting period before the order to order %d (sec): ",i+1);
		getUserEntry("", &waitingTime);
		printf("Quantity of product for the order %d : ",i+1);
		getUserEntry("", &kanbanQuantities);
		userTimeLine[i] = addNodeToTimeline(waitingTime, kanbanQuantities);
	}
	i++;
	userTimeLine[i] = NULL;
	i=0;
	while(userTimeLine[i] != NULL){//display timeline
		printf("%d sec pause --> ",userTimeLine[i]->WaitingTime);
		printf("send a order for %d items --> ",userTimeLine[i]->KanbanQuantities);
		i++;
	}
	printf("Finish \n");
}

void *timeLine_thread(void *p_data){
	int i = 0;
	Workstation *customer = (Workstation *) p_data;
	printf("Launching time line\n");
	while(userTimeLine[i] != NULL){
		usleep(userTimeLine[i]->WaitingTime * 1000000);
		send_kanban(customer, customer->containers0, customer, userTimeLine[i]->KanbanQuantities); //tramsmit kanban to customer thread
		pthread_cond_signal(&(customer->cond_EmptyContainers));// wake up customer thread
		i++;
	}
	printf("Time line finish\n");
	pthread_exit(NULL);
}

 
void getUserEntry(char *txt, int *value){ // get user keyboard input
	printf("%s", txt);
	scanf("%d", value);
}

TimeLineNode *addNodeToTimeline(int waitingTime, int kanbanQuantities){ // create node in time line
	TimeLineNode *node = calloc(1, sizeof(TimeLineNode));
	node->WaitingTime = waitingTime;
	node->KanbanQuantities = kanbanQuantities;
	return node;
}