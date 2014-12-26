#include <stdbool.h>

#ifndef __MYFACTORY__
#define __MYFACTORY__
#include "structures.h"


void join_threads_workstations(Workstation *factory[], int nb_workstations);
void destroy_all_workstations(Workstation *factory[], int nb_workstations);

#endif

