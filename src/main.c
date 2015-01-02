#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "simulation.h"

int main(void) {

	signal(SIGINT, sigintHandler);

	buildTimeline();
	runSimulation();

	return EXIT_SUCCESS;
}
