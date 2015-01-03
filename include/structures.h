#include <pthread.h>

#ifndef __MYSTRUCTURES__
#define __MYSTRUCTURES__


typedef struct Ressource Ressource;
typedef struct Kanban Kanban;
typedef struct LinkedList LinkedList;
typedef struct LinkedListNodeKanban LinkedListNodeKanban;
typedef struct Workstation Workstation;
typedef struct TimeLineEvent TimeLineEvent;

struct TimeLineEvent
{
	int WaitingTime;
	int KanbanQuantity;
};

struct Ressource
{
	int elapsedTime;
	char *name;
};


struct Kanban
{
	Workstation *from;
	Ressource **fromContainerID;
	Workstation *to;
	unsigned int nbRessources;
};


struct LinkedList
{
	int nbKanban;
	LinkedListNodeKanban *first;
};

struct LinkedListNodeKanban
{
	Kanban *kanban;
	LinkedListNodeKanban *next;
};


struct Workstation
{
	Workstation *parents[2];
	char *name;
	Ressource *containers0[2];
	Ressource *containers1[2];
	unsigned short int processDelay;	// Workstation working time (seconds)
	LinkedList *todo;     				// Linked List for to-do kanbans
	Kanban *doing;
	LinkedList *done;					// Linked List for done kanbans

	pthread_t thread;

	pthread_mutex_t mutex_IDLE;
	pthread_cond_t cond_IDLE;
	pthread_mutex_t mutex_EmptyContainers;
	pthread_cond_t cond_EmptyContainers;
	pthread_mutex_t mutex_FullContainers;
	pthread_cond_t cond_FullContainers;

};


#endif
