
#ifndef __MYCONFIG__
#define __MYCONFIG__



// View mode (true: all workstations / false: one workstation)
#define OBSERVE_ALL_WORKSTATIONS	true

// Used if OBSERVE_ALL_WORKSTATIONS is set to false
#define OBSERVE_WORKSTATION			"Aircraft testing"	// Workstation name : COPY & PASTE FROM simulation.c



// Other configurations
#define NB_WORKSTATIONS				7

#define TIMELINE_EVENTS_CAPACITY	50
#define FINAL_CONTAINERS_CAPACITY	100

// factor for each workstation work time (here 100ms)
#define WORKTIMEFACTOR	1000000


#endif
