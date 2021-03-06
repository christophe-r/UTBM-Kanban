
#ifndef __MYCONFIG__
#define __MYCONFIG__


// View mode (true: all workstations / false: one workstation)
#define OBSERVE_ALL_WORKSTATIONS	true

// Used if OBSERVE_ALL_WORKSTATIONS is set to false
#define OBSERVE_WORKSTATION			"Cabin assembly"	// Workstation name : COPY & PASTE FROM simulation.c



// Which messages to log : 
// 1 : shows only very basic stuff
// 2 : shows basic stuff + monitors
// 3 : shows everything
#define LOGLEVEL					3


// Other configurations
#define NB_WORKSTATIONS				13

#define TIMELINE_EVENTS_CAPACITY	50
#define FINAL_CONTAINERS_CAPACITY	100


// Work time scale for workstations. (Used to debug)
// 1000000: 1sec=>1sec. 1000: 1sec=>0.1sec. etc.
#define WORKTIME_SCALE				1000000


#endif
