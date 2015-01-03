#include <stdbool.h>

#ifndef __MYKANBAN__
#define __MYKANBAN__
#include "structures.h"


LinkedList *create_linkedlist();
Kanban *create_kanban(Workstation *from, Resource *fromContainerID[], Workstation *to, int nbResources);

void send_kanban(Workstation *from, Resource *fromContainerID[], Workstation *to, int nbResources);

void print_kanban(Kanban *kanban);


void print_list(LinkedList *linkedlist);
Kanban *pop(LinkedList *linkedlist); // Remove the first item and returns it
void push(LinkedList *linkedlist, Kanban *kanban); // Adds to the end of the list
void destroy_linkedlist(LinkedList *linkedlist);

#endif

