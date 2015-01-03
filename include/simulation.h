#ifndef __SIMULATION__
#define __SIMULATION__
#include "structures.h"

void sigintHandler(int sig_num);
void runSimulation();

void buildTimeline();
void *timeline_thread(void *p_data);
void getUserEntry(char *txt, int *value);
TimeLineEvent *addEventToTimeline(int waitingTime, int kanbanQuantity);

#endif
