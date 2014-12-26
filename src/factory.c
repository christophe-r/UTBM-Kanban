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

void destroy_all_workstations(Workstation *factory[], int nb_workstations){
	int i;
	for(i=0 ; i<nb_workstations ; i++ ){
		//printf("i=%d, addr: %d, addr: %d\n", i, (factory+i*sizeof(Workstation)), sizeof(Workstation) );
		//destroy_workstation(*(factory+i));
		destroy_workstation(factory[i]);
	}
}