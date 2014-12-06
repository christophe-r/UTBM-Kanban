#include <stdlib.h>
#include <stdio.h>

#include "structures.h"
#include "factory.h"


void join_threads_workstations(Workstation *factory[], int nb_workstations){

	int i;
	for(i=0 ; i<nb_workstations ; i++ ){
		pthread_join(factory[i]->thread, NULL);
		#ifdef DEBUG
			printf("Workstation \"%s\" thread joined.\n", factory[i]->name);
			fflush(NULL);
		#endif
	}
	
}

