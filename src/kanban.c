#include <stdio.h>
#include <stdlib.h>

#include "kanban.h"

/***** Kanban ******/
Kanban *create_kanban(Workstation *from, Resource *fromContainerID[], Workstation *to, int nbResources){

	Kanban *kanban = calloc(1, sizeof(Kanban));
	kanban->from = from;
	kanban->fromContainerID = fromContainerID;
	kanban->to = to;
	kanban->nbResources = nbResources;
	#ifdef DEBUG
		// printf("Kanban created (qty: %d) from \"%s\" to \"%s\".\n", nbResources, from->name, to->name);
		fflush(NULL);
	#endif
	
	return kanban;
}

void send_kanban(Workstation *from, Resource *fromContainerID[], Workstation *to, int nbResources){

	if( to != NULL ){
		push(to->todo, create_kanban(from, fromContainerID, to, nbResources));
		return;
	}else{
		#ifdef DEBUG
			printf("Warning: No \"to\" workstation to push a TODO kanban in LinkedList.\n");
		#endif
	}

	
}


void print_kanban(Kanban *kanban){

	if( kanban != NULL ){
		printf("-----------Kanban-----------\n");
		printf("From: %s\n", kanban->from->name);
		if( kanban->to != NULL ){
			printf("To:   %s\n", kanban->to->name);
		} else {
			printf("To:   -\n");
		}
		printf("Nb Resources: %d\n", kanban->nbResources);
		printf("----------------------------\n");
	} else {
		printf("No Kanban to print.\n");
	}
}



/***** Linked List ******/


LinkedList *create_linkedlist(){

	LinkedList *linkedlist = malloc(sizeof(LinkedList));

	linkedlist->nbKanban = 0;
	linkedlist->first = NULL;

	return linkedlist;
}

void print_list(LinkedList *linkedlist) {

    LinkedListNodeKanban *current = linkedlist->first;

    while (current != NULL) {
        print_kanban(current->kanban);
        current = current->next;
    }
}

// Remove the first item from the list and return it
Kanban *pop(LinkedList *linkedlist) {

    LinkedListNodeKanban *item = linkedlist->first;
	Kanban *kanban = NULL;


	if( item != NULL ){
		kanban = item->kanban;
		linkedlist->first = linkedlist->first->next;
		
		free(item);

		linkedlist->nbKanban--;
	}


    return kanban;
}

// Push an item at the end of the list
void push(LinkedList *linkedlist, Kanban *kanban) {

	if( kanban == NULL ){
		#ifdef DEBUG
			printf("Warning: No kanban to push in the LinkedList.\n");
		#endif
		return;
	}

    LinkedListNodeKanban *current = linkedlist->first;

	if( current == NULL ){ // First item
		linkedlist->first = malloc(sizeof(LinkedListNodeKanban));
		linkedlist->first->kanban = kanban;
    	linkedlist->first->next = NULL;

	} else {
		while (current->next != NULL) {
		    current = current->next;
		}

    	current->next = malloc(sizeof(LinkedListNodeKanban));
    	current->next->kanban = kanban;
    	current->next->next = NULL;

	}

	linkedlist->nbKanban++;
}


void destroy_linkedlist(LinkedList *linkedlist){

    LinkedListNodeKanban *current = linkedlist->first;
	LinkedListNodeKanban *current_temp = NULL;

    while (current != NULL) {
        free(current->kanban);
		current_temp = current->next;
		free(current);
		current = current_temp;
    }

	free(linkedlist);
	
}


