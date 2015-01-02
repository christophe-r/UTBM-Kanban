#include <stdbool.h>

#ifndef __MYWORKSTATION__
#define __MYWORKSTATION__
#include "structures.h"


Workstation *create_workstation(char *name, unsigned short int processDelay, bool launcher);
void *workstation_thread(void *p_data);
void *customer_thread(void *p_data);

void link_workstations(Workstation *workstation, Workstation *parent);
int count_full_container(Ressource *containers[]);
Ressource *take_ressource(Ressource *containers[], Workstation *attachedParent);
int put_ressource(Ressource *containers[], Ressource *ressource);

void move_kanban_todo_to_doing(Workstation *workstation);
void move_kanban_doing_to_done(Workstation *workstation);

void display_workstation(Workstation *workstation);
void consoleLog(Workstation *workstation, char *txt);
void consoleLogRoot(Workstation *workstation, char *txt);
void displayFinalRessources();

void destroy_workstation(Workstation *workstation);

void usleep(int time);

#endif

