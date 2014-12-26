#include <stdlib.h>
#include <stdio.h>

#include "simulation.h"
#include "structures.h"
#include "workstation.h"
#include "factory.h"
#include "kanban.h"

#define NB_WORKSTATIONS		7



void runSimulation() {

	// Creates the factory (array of workstations)
	Workstation *factory[NB_WORKSTATIONS];

	// Creates each workstation  
	factory[0] = create_workstation("Workstation base   1.1", 8);
	factory[1] = create_workstation("Workstation base   1.2", 8);
	factory[2] = create_workstation("Workstation middle 1", 8);
	factory[3] = create_workstation("Workstation base   2.1", 8);
	factory[4] = create_workstation("Workstation base   2.2", 8);
	factory[5] = create_workstation("Workstation middle 2", 8);
	factory[6] = create_workstation("Workstation up     0", 8);

	// Linking the workstations each others
	link_workstations(factory[2], factory[0]); // The parent of Workstation 2 is now Workstation 0
	link_workstations(factory[2], factory[1]); // The parent of Workstation 2 is now Workstation 0 & 1
	link_workstations(factory[5], factory[3]); // The parent of Workstation 5 is now Workstation 3
	link_workstations(factory[5], factory[4]); // The parent of Workstation 5 is now Workstation 3 & 4
	link_workstations(factory[6], factory[5]); // The parent of Workstation 6 is now Workstation 2
	link_workstations(factory[6], factory[2]); // The parent of Workstation 6 is now Workstation 2 & 5


	// Ressource *arrivalContainer[2];
	// arrivalContainer[0] = arrivalContainer[1] = NULL
	// send_kanban(NULL, factory[2]->containers0, factory[1],1);
	
	// Joins workstation's threads
	join_threads_workstations(factory, NB_WORKSTATIONS);
 	

	// Destroys the workstations
	destroy_all_workstations(factory, NB_WORKSTATIONS);


	return;
}

 
