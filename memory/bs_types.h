/* Autoren der Erweiterung: Wail Soliaman, Alexander Luhmann */
/* Include-file defining elementary data types used by the */
/* operating system */
#ifndef __BS_TYPES__
#define __BS_TYPES__

typedef enum {FALSE=0, TRUE} Boolean;

/* data type for the possible types of processes */
/* the process type determines the IO-characteristic */
typedef enum 
	{
		os, interactive, batch, background, foreground
	} ProcessType_t; 

/* data type for the process-states used for scheduling and life-	*/
/* cycle manegament of the processes 								*/
typedef enum 
	{
		init, running, ready, blocked, ended
		
	} Status_t; 

/* data type for the different events that cause the scheduler to	*/
/* become active */
typedef enum 
	{ 
		completed, io, quantumOver

	} SchedulingEvent_t; 

/* data type for the Process Control Block */
/* like additional schedulers */
typedef struct 
	{
		Boolean valid; 		
		unsigned pid; 
		unsigned ppid; 
		unsigned ownerID;
		unsigned start; 
		unsigned duration; 
		unsigned size; 
		unsigned usedCPU; 
		ProcessType_t type; 
		Status_t status;
	} PCB_t; 

/*Datentype für blockierter Prozess*/
typedef struct
	{
		unsigned pid;
		unsigned size;
		Boolean isBlocked;
	} BlockedListElement_t;


#endif  /* __BS_TYPES__ */ 