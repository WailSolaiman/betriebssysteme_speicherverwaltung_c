/* Autoren der Erweiterung: Wail Soliaman, Alexander Luhmann */
/* Implementation of the log functions */
/* for comments on the functions see the associated .h-file */

/* ---------------------------------------------------------------- */
/* Include required external definitions */
#include <math.h>
#include "bs_types.h"
#include "globals.h"
#include "log.h"


/* ---------------------------------------------------------------- */
/*                Declarations of local helper functions            */

/* ---------------------------------------------------------------- */
/* Declarations of global variables visible only in this file 		*/
// array with strings associated to scheduling events for log outputs
char eventString[3][12] = {"completed", "io", "quantumOver"};

/* ---------------------------------------------------------------- */
/*                Externally available functions					*/
/* ---------------------------------------------------------------- */

void logGeneric(char* message)
{
	printf("%6u : %s\n", systemTime, message); 
}
	
void logPid(unsigned pid, char * message)
{
	printf("%6u : PID %3u : %s\n", systemTime, pid, message); 
}
		
void logPidCompleteness(unsigned pid, unsigned done, unsigned length, 
						char * message)
{
	printf("%6u : PID %3u : completeness: %u/%u | %s\n", systemTime, 
			pid, done, length, message); 
}
		
void logPidMem(unsigned pid, char * message)
{
	printf("%6u : PID %3u : %s\n", systemTime, 
			pid, message); 
}
	
		

/* ----------------------------------------------------------------- */
/*                       Local helper functions                      */
/* ----------------------------------------------------------------- */
/*Ausgabe wenn ein Prozess groß ist.*/
void logPidProcessTooLarge(unsigned pid){
	printf("	\n\n	PROCESS [%d] too large, not started \n\n", pid);
}
/*Ausgabe für ein Prozess, wenn er eingelagert wird.*/
void logPidInList(unsigned pid, unsigned start, unsigned length, unsigned currentMemorySize,
						char * message)
{
	printf("	----------------------------------------------------------------------\n");
	printf("  %4d :", systemTime);
	printf("PROCESS PID : %u ADDED TO THE LIST OF RUNNING PROCESS \n", pid);
	printf("	[RUNNING]   START : %u  SIZE : %u | %s	\n", start, length, message);
	printf("	CURRENT MEMORY SIZE : %u  \n", currentMemorySize);
	printf("	----------------------------------------------------------------------\n");

}
/*Ausgabe für ein Prozess, wenn er ausgelagert wird.*/
void logPidFromList(unsigned pid, unsigned start, unsigned length, unsigned currentMemorySize,
						char * message)
{
	printf("	----------------------------------------------------------------------\n");
	printf("  %4d :", systemTime);
	printf("PROCESS PID : %u [TERMINATED] \n", pid);
	printf("	[FREE]      START : %u  SIZE : %u | %s\n", start, length, message);
	printf("	CURRENT MEMORY SIZE : %u  \n", currentMemorySize);
	printf("	----------------------------------------------------------------------\n");

}
/*	Ausgabe Wohin wie viele Lücken verschmolzen werden 
	wenn 3 Lücken verschmolzen wrden, dann count = 3
	werden nur 2 Lücken verschmolzen muss die Richtung durch einen BOOLEAN angegeben werden
	(TRUE == LEFT, FALSE == RIGHT)
*/
void logMerge(unsigned count, unsigned length, Boolean site){
	if(count == 3) {
		printf("	\n	MERGE THREE GAPS, NEW GAP SIZE : %d \n\n", length);
	}
	else {
		if(site == TRUE)
			printf("	\n	MERGE TWO GAPS to the LEFT, NEW GAP SIZE : %d \n\n", length);
		if(site == FALSE)
			printf("	\n	MERGE TWO GAPS to the RIGHT, NEW GAP SIZE : %d \n\n", length);
	}
}
/*Ausgabe der blockierter Prozess, wenn sie in die Warteschlange eingefügt bzw. entfernt werden.*/
void logQueue(Boolean inOrOut, unsigned pid) {
	if(inOrOut == TRUE)
		printf("	EN-QUEUE : ADD PROCESS [%d] IN BLOCKED-LIST \n\n", pid);
	else
		printf("	DE-QUEUE : REMOVE PROCESS [%d] FROM BLOCKED-LIST \n\n", pid);
}
/*Ausgabe für Zeilenbrüche und ander Formatierungs-Zeichen.*/
void logFormat(char * message){
	printf("%s", message);
}
/*Ausgabe einer Überschrift.*/
void logHeadline(char * message){
	printf("%s", message);
}
/*Ausgabe für die gesamte Liste mit ihre Daten.*/
void logMainMemoryRepresentation(Boolean PidOrLenght,unsigned value){
	if (PidOrLenght)
		printf("	PROCESS PID [%3d]  |",value);
	else
		printf("	FREE SIZE   (%3d)  |",value);
}
/*Ausgabe für die aktuelle Speichergröße.*/
void logFirstMemorySize(unsigned size){
	printf("\n\n	CURRENT MEMORY SIZE : %d\n\n", size);
}