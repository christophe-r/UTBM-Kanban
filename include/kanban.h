#include <stdbool.h>

#ifndef __MYKANBAN__
#define __MYKANBAN__
#include "structures.h"


LinkedList *create_linkedlist();
Kanban *create_kanban(Workstation *from, Ressource *fromContainerID[], Workstation *to, int nbRessources);

void send_kanban(Workstation *from, Ressource *fromContainerID[], Workstation *to, int nbRessources);
void move_kanban_todo_to_doing(Workstation *workstation);
void move_kanban_doing_to_done(Workstation *workstation);

void print_kanban(Kanban *kanban);


void print_list(LinkedList *linkedlist);
Kanban *pop(LinkedList *linkedlist); // Remove the first
//int remove_last(LinkedListKanban * head);
void push(LinkedList *linkedlist, Kanban *kanban); // Adding to the end of the list
void destroy_linkedlist(LinkedList *linkedlist);

#endif

