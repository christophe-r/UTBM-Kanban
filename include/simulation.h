#ifndef __SIMULATION__
#define __SIMULATION__


void sigintHandler(int sig_num);
void runSimulation();
void *customer_thread(void *p_data);

#endif
