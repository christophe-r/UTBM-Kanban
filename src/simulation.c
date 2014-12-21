#include <stdlib.h>
#include <stdio.h>

#include "simulation.h"
#include "structures.h"
#include "workstation.h"
#include "factory.h"
#include "kanban.h"

#define NB_WORKSTATIONS		3


void runSimulation() {

	// Creates the factory (array of workstations)
	Workstation *factory[NB_WORKSTATIONS];

	// Creates each workstation
	factory[0] = create_workstation("Workstation 0", 10);
	factory[1] = create_workstation("Workstation 1", 8);
	factory[2] = create_workstation("Workstation 2", 5);

	// Linking the workstations each others
	link_workstations(factory[2], factory[0]); // The parent of Workstation 2 is now Workstation 0
	link_workstations(factory[2], factory[1]); // The parent of Workstation 2 is now Workstation 0 & 1


	// Joins workstation's threads
	join_threads_workstations(factory, NB_WORKSTATIONS);
 	

	// Destroys the workstations
	destroy_all_workstations(factory, NB_WORKSTATIONS);


	return;
}

 
