#include <pthread.h>

#ifndef __MYSTRUCTURES__
#define __MYSTRUCTURES__


typedef struct Ressource Ressource;
struct Ressource
{
	int elapsedTime;
	char name[20];
};


typedef struct Workstation Workstation;
struct Workstation
{
	Workstation *parents[2];
	char *name;
	Ressource *containers1[2];
	Ressource *containers2[2];
	pthread_t thread;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
};

#endif
