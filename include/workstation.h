#include <stdbool.h>

#ifndef __MYWORKSTATION__
#define __MYWORKSTATION__
#include "structures.h"


Workstation *create_workstation(char *name);
void *workstation_thread(void *param);

void link_workstations(Workstation *workstation, Workstation *parent);

void display_workstation(Workstation *workstation);

void destroy_workstation(Workstation *workstation);
void destroy_all_workstations(Workstation *factory[], int nb_workstations);

#endif

