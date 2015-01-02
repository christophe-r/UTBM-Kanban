#ifndef __SIMULATION__
#define __SIMULATION__
#include "structures.h"

void sigintHandler(int sig_num);
void runSimulation();
void buildTimeline();
void *timeLine_thread(void *p_data);
void getUserEntry(char *txt, int *value);
TimeLineNode *addNodeToTimeline(int waitingTime, int kanbanQuantities);

#endif
