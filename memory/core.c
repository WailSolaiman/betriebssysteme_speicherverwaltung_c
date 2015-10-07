/* Autoren der Erweiterung: Wail Soliaman, Alexander Luhmann */
/* Implementation of core functionality of the OS            */
/* this includes the main scheduling loop                    */
/* for comments on the functions see the associated .h-file  */
  
/* --------------------------------------------------------- */
/* Include required external definitions */
#include <math.h> 
#include <time.h> 
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include "bs_types.h" 
#include "globals.h" 
#include "core.h" 
#include "loader.h" 
  
/* ---------------------------------------------------------------- */
/* Declarations of global variables visible only in this file       */
  
PCB_t process;      // the only user process used for batch and FCFS 
PCB_t* pNewProcess; // pointer for new process read from batch 
BlockedListElement_t blockedOne ; // the only process that can be blocked 
FILE* processFile; 
struct feld { 
    unsigned pid;               //Aktuelle pid in Zelle 
    unsigned start;             //Prozess-Start 
    unsigned length;            //Prozess-Größe 
    Boolean occupiedByProcess;  //Ist Zelle besetzt ? 
    struct feld *next;          //Zeiger auf Nachfolge Zelle 
    struct feld *previous;      //Zeiger auf Vorgaenger Zelle 
}; 
struct feld *first = NULL;      //Zeiger auf das erste Element 
struct feld *last = NULL;       //Zeiger auf das letzte Element 
struct feld *pointer;           //Hilf-Zeiger 
struct feld *pointer1;          //Hilf-Zeiger 
BlockedListElement_t queue[MAX_PROCESSES];      // Blocked Queue 
unsigned front = 0;                             // first Blocked Process in Queue 
unsigned rear = 0;                              // last Blocked Process in Queue
  
/* ---------------------------------------------------------------- */
/*                Declarations of local helper functions            */
  
void firstFit(unsigned pid, unsigned size); 
void insertTheProcess(unsigned pid, unsigned size); 
void removeTheProcess(unsigned pid, unsigned size); 
void insertTheFirstEmptyFeld(); 
void mergeEmptyFields(struct feld *gap); 
void compaction(); 
Boolean isGabAvailable (unsigned pid, unsigned size); 
void processOutput(); 
void enQueue(BlockedListElement_t blockedProcess); 
void deQueue(); 
unsigned returnTotalGabSize(); 
  
/* ---------------------------------------------------------------- */
/*                Externally available functions                    */
/* ---------------------------------------------------------------- */
  
void initOS(void) 
{ 
  
    char filename[128] = PROCESS_FILENAME;  
    unsigned i;                 // iteration variable 
    systemTime=0;               // reset the system time to zero 
    // open the file with process definitions 
    processFile = openConfigFile (processFile, filename);    
    logGeneric("Process info file opened"); 
    srand( (unsigned)time( NULL ) );    // init the random number generator 
  
    /* init the status of the OS */
    // mark all process entries invalid 
    for (i=0; i<MAX_PROCESSES; i++) processTable[i].valid=FALSE; 
    process.pid=0;          // reset pid 
} 
  
void coreLoop(void) 
{ 
    PCB_t *candidateProcess=NULL;       // next process to start, already stored in process table 
    unsigned candiatePid;               // pid of new process candicate (waiting to be started) 
    PCB_t* nextReady=NULL;              // pointer to process that finishes next     
    Boolean fileComplete = FALSE;       // end of pending processes in the file indicator 
    unsigned runningCount=0;            // counter of currently running processes 
    unsigned i,pid;                     // used for accessing and counting processes  
    unsigned minRemaining, remaining;   // used for processing information on processes 
    unsigned delta=0;                   // time interval by which the systemTime is advanced 
  
    blockedOne.isBlocked = FALSE; 
  
  
    do {    // loop until no more process to run  
        // select and run a process 
        if (candidateProcess==NULL) // no candidate read from file yet 
        { 
            logGeneric("No candidate read, reading next process from file\n"); 
            // read the next process for the file and store in process table 
            candiatePid=getNextPid();                   // get next valid pid 
            processTable[candiatePid].pid=candiatePid;      // and store in PCB 
            // now really read the process information:  
            candidateProcess = readNextProcess (processFile, &processTable[candiatePid]); 
            if (candidateProcess!=NULL) 
            {   // there are still jobs listed in the file    
                candidateProcess->valid = TRUE;      // flag as valid entry 
                logPid(candidateProcess->pid, "Process loaded from file");  
            } 
            else    // no more processes to be started  
            { 
                logGeneric("No further process listed for execution.");  
                fileComplete=TRUE;  
            } 
        } 
        // if there are still processes to be started, the next candidate of these is now known  
        if (candidateProcess!=NULL)  
        {   // there is a process pending 
            if (candidateProcess->start<=systemTime)  // test if the process is ready to be started 
  
            {   // the process is ready to be started 
                // now search for a suitable piece of memory for the process 
                  
            // Prozess nur einlagern wenn eine Lücke dafür ausreichen ist 
            // sonst den Prozess in die Warteschlange einreihen 
                if (isGabAvailable(candidateProcess->pid, candidateProcess->size) == TRUE) {                   
  
                    // this simple if must be replaced with searching for a memory location: 
                    if (usedMemory+candidateProcess->size <= MEMORY_SIZE) 
                    {   // enough memory available, and location in memory found: start process 
                      
                        candidateProcess->status=running;    // all active processes are marked active 
                        runningCount++;                     // and add to number of running processes 
                        usedMemory = usedMemory+candidateProcess->size;  // update amount of used memory 
                        systemTime=systemTime + LOADING_DURATION;   // account for time used by OS 
  
                        logPidMem(candidateProcess->pid, "Process started and memory allocated"); 
                        firstFit(candidateProcess->pid, candidateProcess->size); 
                        processOutput(); 
                        candidateProcess=NULL;  // process is now a running process, not a candidate any more 
                    } 
                } 
                else {// Prozess ist zu Groß, muss also in die Warteschlange 
                    // Ausgabe zur Kontrolle 
                    logPidProcessTooLarge(candidateProcess->pid); 
                    //Prozess als Blocked bemerken 
                    blockedOne.pid = candidateProcess->pid;                                   
                    blockedOne.size = candidateProcess->size; 
                    blockedOne.isBlocked = TRUE; 
                      
                    if (rear == 0)                                  //Aller ersten blockierter Prozess in Warteschlange einfuegen                                                        
                        enQueue(blockedOne); 
                    else { 
                        unsigned k = 0; 
                        //Nur Prozesse einlagern die noch nicht in der Warteschlange stehen 
                        while (k <= rear) { 
                            // Prozess schon in der Warteschlange 
                            if (blockedOne.pid == queue[k].pid) 
                                break; 
                            // Prozess noch nicht in der Warteschlange 
                            else if ((blockedOne.pid != queue[k].pid) && (queue[k].pid == 0)) { 
                                enQueue(blockedOne); 
                                break; 
                            } 
                            k++; 
                        } 
                    }    
                } 
  
            } 
        } 
  
        else 
        { 
            //logPidMem(nextReady->pid, "Process read but not ready yet");  
            if (runningCount==0) 
            {   // no other process will advance the system time 
                systemTime=candidateProcess->start;  
                logGeneric("CPU was idle until now.");  
            } 
        } 
  
        // In case the candidate was started, check for another candiate before running the proceses 
        if ((!fileComplete) && (candidateProcess==NULL)) continue;  
        /* the use of "continue" is not optimal, but as "break" in this case tolerable for a first implementation 
        // The checks for starting a process are done. Now work on the running processes  
        */
  
  
        // get the process that will be completed next by searching the process table 
        minRemaining = UINT_MAX;        // init search value. UINT_MAX : Maximum value for a variable of type unsigned int.  Value : 4294967295 
        pid=1;                          // start search with lowest valid pid 
        i=0;                            // init counter for running processes  
        nextReady=NULL;                 // init result 
        while ((i<runningCount) && (pid<MAX_PROCESSES))  
        { 
            if ((processTable[pid].valid==TRUE) && (processTable[pid].status==running)) 
            { 
                remaining=processTable[pid].duration-processTable[pid].usedCPU; 
                if (remaining<minRemaining) 
                {   // found a process finishing earlier than last match 
                    nextReady=&processTable[pid];       // get pointer to that process 
                    minRemaining=remaining;             // update minimum 
                } 
                i++;    // one more running process used 
            } 
            pid++;      // next process entry 
        } 
        // the next process to end is found.  
        delta=minRemaining;     // all processes advance until first to quit is finished 
        // Compare with the waiting candidate (if any) 
        if (candidateProcess!=NULL)  
        { 
            if ((candidateProcess->start > systemTime)  
                && (candidateProcess->start-systemTime < minRemaining)) 
            {   // the waiting process can start before any running one is complete 
                delta=(candidateProcess->start-systemTime)/runningCount; // processes share available systemTime  
                nextReady=NULL;     // no process to terminate now 
            } 
        } 
  
  
        //Now update OS-stats and terminate that process (if needed) 
        pid=1;                          // start search with lowest valid pid and  
        i=0;                            // init counter for running processes  
        while ((i<runningCount) && (pid<MAX_PROCESSES))  
        { 
            if ((processTable[pid].valid==TRUE) && (processTable[pid].status==running)) 
            { 
                // update time already spent in process 
                processTable[pid].usedCPU=processTable[pid].usedCPU+delta; 
                systemTime=systemTime+delta;        // each process consumes CPU time 
                i++;    // one more running process updated 
            } 
            pid++;      // next process entry 
        } 
  
  
  
        // all running processes are updated, now quit the completed process 
        if (nextReady!=NULL)    // check of a process needs to be terminated 
        {                   // loop may be running even if no processes are active 
            usedMemory=usedMemory-nextReady->size;   // mark memory of the process free   
            removeTheProcess(nextReady->pid, nextReady->size);	// Prozess wird ausgelagert 
            deQueue();	// blockierte Prozess wird von Queue entfernet, falls möglich
			deleteProcess (nextReady);  // terminate process 
            runningCount--;             // one running process less 
        } 
    }  
    // loop until no running processes exist any more and no process is waiting t be started 
    while ((runningCount>0) || (fileComplete==FALSE)); 
    //mergeEmptyFields(); 
} 
  
unsigned getNextPid() 
{ 
    static unsigned pidCounter=1;  
    unsigned i=0;       // iteration variable; 
    // determine next available pid make sure not to search infinitely 
    while ((processTable[pidCounter].valid) && (i<MAX_PID)) 
    { 
        // determine next available pid  
        pidCounter=(pidCounter+1) % MAX_PID; 
        if (pidCounter==0) pidCounter++;    // pid=0 is invalid 
        i++;        // count the checked entries 
    } 
    if (i==MAX_PID) return 0;           // indicate error 
    else           return pidCounter; 
} 
  
  
/* ----------------------------------------------------------------- */
/*                       Local helper functions                      */
/* ----------------------------------------------------------------- */
  
/* Funktion firstFit sucht eine passende Luecke fuer den Prozess, 
    von Anfang der Liste, und fuegt diesen ein.*/
void firstFit(unsigned pid, unsigned size) { 
    struct feld *firstFitPointer = first;                   //Zeiger zum Suchen auf das erste Elementen                  
    struct feld *newEmptyFeld = NULL;                       //Zelle für eine mögliche neue Lücke deklaieren 
    unsigned newFeldStart = 0;								//Neuer Start der neuen Zelle
	unsigned newFeldLength = 0;								//Neue Größe der neuen Zelle
	// Lücke die unseren Freien Speicherdarstellt und  
    // den ersten Prozess in unsere Prozessverwaltung einfügen 
    if (firstFitPointer == NULL) { 
        insertTheFirstEmptyFeld(); 
        insertTheProcess(pid, size); 
    }   
    else { 
  
        while (firstFitPointer != NULL) {                           //Suche von (Anfang) der Liste nach einer passende Lücke   
            if (firstFitPointer->occupiedByProcess == FALSE) {  
                if ((firstFitPointer->length) == (size)) {            // Falls Lücke Größe == Process Größe 
                    pointer = firstFitPointer->previous;
					newFeldStart = pointer->start + pointer->length;
					
					pointer = firstFitPointer;           
                    pointer->pid = pid;                              //Neuen Prozess in Lücke einfügen 
                    pointer->start = firstFitPointer->start; 
                    pointer->length = size; 
                    pointer->occupiedByProcess = TRUE; 
                    logPidInList(pointer->pid, pointer->start, pointer->length, 
                        last->length, "MEMORY ALLOCATED [GAP USED]"); 
                    pointer = last->previous;                 
                    break;                                          //break, wenn passende Lücke gefunden, nicht weiter suchen 
                } 
                else if ((firstFitPointer->length) > (size)) {        // Falls Lücken Größe > Process Größe                
                    newFeldLength = (firstFitPointer->length) - (size);
					pointer = firstFitPointer->previous;
					newFeldStart = pointer->start + pointer->length;
  
                    pointer = firstFitPointer; 
                    pointer->pid = pid;                              //Neuen Prozess in Lücke einfuegen 
                    pointer->start = newFeldStart; 
                    pointer->length = size; 
                    pointer->occupiedByProcess = TRUE; 
					//Neue Lücke mit neuen Größe wird erzeugt 
                    newEmptyFeld = (struct feld*) malloc(sizeof(struct feld));         
                    newEmptyFeld->occupiedByProcess = FALSE; 
                    newEmptyFeld->start = (firstFitPointer->start) + (firstFitPointer->length); 
                    newEmptyFeld->length = newFeldLength;
  
                    logPidInList(pointer->pid, pointer->start, 
                        pointer->length, last->length, "MEMORY ALLOCATED [GAP USED]");  
					if (pointer == last) {							// newEmptyFeld wird mit der Liste verbunden				
						pointer->next = newEmptyFeld;
						newEmptyFeld->previous = pointer;
						newEmptyFeld->next = NULL;
						last = newEmptyFeld;
						break;							//break, wenn passende Lücke gefunden, nicht weiter suchen 
					}
					else {
						pointer = firstFitPointer->next; 
						newEmptyFeld->next = pointer; 
						newEmptyFeld->previous = firstFitPointer; 
						firstFitPointer->next = newEmptyFeld; 
						pointer->previous = newEmptyFeld;
					}
                    pointer = last->previous; 
                    break; 
                } 
            } 
            if ((firstFitPointer->next == last)){         
                insertTheProcess(pid, size);        // Prozess in die letzte Lücke eingefügt 
                break; 
            } 
            firstFitPointer = firstFitPointer->next; 
        }//while ende                
    } 
} 
  
/* Funktion insertTheFirstEmptyFeld erzeugt die erste Zelle mit Größe MEMORY_SIZE.*/
void insertTheFirstEmptyFeld(){ 
    if (first == NULL){                                             
        first = (struct feld*) malloc(sizeof(struct feld)); 
        first->start = 0; 
        first->length = MEMORY_SIZE; 
        first->occupiedByProcess = FALSE; 
        first->next = NULL;  
        last = first;			// Da nur eine Zelle existiert ist diese Zelle das Enden und der Anfang
        last->previous = NULL; 
    } 
} 
  
/* Funktion insertTheProcess fügt neuen Prozess, mit Hilfe von firstFit, in die Liste ein.*/
void insertTheProcess(unsigned pid, unsigned size){ 
    if (first == last) {					//Falls nur eine Zelle existiert, füge erstern Prozess ein
        logFirstMemorySize(last->length); 
  
        pointer = first; 
        pointer = pointer->previous; 
        pointer = (struct feld*) malloc(sizeof(struct feld)); 
        pointer->pid = pid; 
        pointer->start = 0; 
        pointer->length = size; 
        pointer->occupiedByProcess = TRUE; 
  
        last->length = last->length - size; 
        logPidInList(pointer->pid, pointer->start, 
            pointer->length, last->length, "MEMORY ALLOCATED");  
        first = pointer; 
        first->previous = NULL; 
        first->next = last; 
        last->previous = first;   
        last->next = NULL; 
    } 
    else {									//füge weitere Prozesse ein
        unsigned start_ = (pointer->start) + (pointer->length); 
  
        pointer1 = pointer; 
        pointer = pointer->next; 
        pointer = (struct feld*) malloc(sizeof(struct feld)); 
        pointer->pid = pid; 
        pointer->start = start_; 
        pointer->length = size; 
        pointer->occupiedByProcess = TRUE; 
  
        last->length = last->length - size; 
        logPidInList(pointer->pid, pointer->start, 
            pointer->length, last->length ,"MEMORY ALLOCATED");  
        pointer->next = pointer1->next; 
        pointer1->next->previous = pointer;    
        pointer->previous = pointer1; 
        pointer1->next = pointer;     
    } 
    if (last->length == 0) {          
            pointer->next = NULL; 
            free(last); 
            last = pointer;  
    } 
} 
  
/* Funktion removeTheProcess markiert ein Prozess als Lücke.*/
void removeTheProcess(unsigned pid, unsigned size){ 
    struct feld *pointerRemoveProcess = first;
	struct feld *gap_ = NULL;
  
    while (pointerRemoveProcess != NULL) {  
        if (pointerRemoveProcess->occupiedByProcess == TRUE){ 
            if ((pointerRemoveProcess->pid) == (pid)) { 
                unsigned freeMemory = last->length + size; 
                logPidFromList(pointerRemoveProcess->pid, pointerRemoveProcess->start, 
                    pointerRemoveProcess->length, freeMemory, "MEMORY FREED"); 
                pointerRemoveProcess->occupiedByProcess = FALSE;				//Zelle als Lücke bemerken
				gap_ = pointerRemoveProcess;                
                mergeEmptyFields(gap_);											//Verschmelzung nach Prozess Terminierung
                processOutput();
				break; 
            } 
        } 
        pointerRemoveProcess = pointerRemoveProcess->next; 
    } 
} 
  
/* Funktion mergeEmptyFields verschmeltzt zwei oder drei benachbarte Lücken.*/
void mergeEmptyFields(struct feld *gap) { 
    struct feld *pointer2 = NULL;		//Hilf-Pointer
	pointer = gap->previous;			//pointer repräsentiert Linker Nachbar
	pointer1 = gap->next;				//pointer1 repräsentiert Rechter Nachbar

	if (((pointer != NULL) && (pointer->occupiedByProcess == FALSE)) 
		&& ((pointer1 != NULL) && (pointer1->occupiedByProcess == FALSE))) {	//Falls 3 Lücken nebeneinander
		pointer1->length = pointer1->length + gap->length + pointer->length;	//Summe alle drei Lücken berechnen
		if (pointer == first) {		
			pointer1->previous = NULL;
			first = pointer1;
			free(gap);
			free(pointer);			
			pointer = pointer1;
		}
		else {	
			pointer2 = pointer->previous;
			pointer2->next = pointer1;
			pointer1->previous = pointer2;
			free(gap);
			free(pointer);
			pointer = pointer2;
		}
		logMerge(3, pointer1->length, TRUE);
	}
	//Falls 2 Lücken nebeneinander, verschmelzen nach Links
	else if ((pointer != NULL) && (pointer->occupiedByProcess == FALSE)) {		
		pointer->length = pointer->length + gap->length;	//Summe zwei Lücken berechnen
		if (pointer == first && gap == last) {	
			pointer->next = NULL;
			last = pointer;
			free(gap);
		}
		else {
			pointer->next = pointer1;
			pointer1->previous = pointer;
			free(gap);
		}		
		logMerge(2, pointer1->length, TRUE);
	}
	//Falls 2 Lücken nebeneinander, verschmelzen nach Rechts
	else if ((pointer1 != NULL) && (pointer1->occupiedByProcess == FALSE)) {	
		pointer1->length = pointer1->length + gap->length;		//Summe zwei Lücken berechnen
		pointer1->start = pointer1->start - gap->length;
		if (pointer1 == last && gap == first) {	
			pointer1->previous = NULL;
			first = pointer1;
			free(gap);
		}
		else {
			pointer->next = pointer1;
			pointer1->previous = pointer;	
			free(gap);
		}	
		logMerge(2, pointer1->length, FALSE);
	}
} 
  
/* Funktion processOutput gibt die gesamte Liste mit ihre Daten aus.*/
void processOutput(){ 
    unsigned zeilenumbruch = 0; 
    struct feld *ausgabe = first; 
    logHeadline("\n	MAIN MEMORY REPRESENTATION : \n");   
    while (ausgabe != NULL) {       
        if (ausgabe->occupiedByProcess == TRUE) {         
            logMainMemoryRepresentation(TRUE,ausgabe->pid); 
        } 
        else { 
            logMainMemoryRepresentation(FALSE,ausgabe->length); 
        } 
        zeilenumbruch++; 
        if (zeilenumbruch % 3 == 0) 
            logFormat("\n"); 
        ausgabe = ausgabe->next; 
    } 
    logFormat("\n\n"); 
} 
  
/* Funktion compaction schiebt die Prozesse und Lücken nebeneinander und verschmilzt alle Lücken.*/
void compaction(){  
    struct feld *compactionPointer = first; 
    struct feld *nextPointer = NULL;
	struct feld *gap_ = NULL;
  
    while (compactionPointer != NULL) {      
        if (compactionPointer->occupiedByProcess == FALSE) {     //Falls Lücke gefunden          
            nextPointer = compactionPointer->next;   
            if (nextPointer != NULL) {
			    // Falls der rechte Nachbar ein Prozess ist,  Prozess und Lücke tauschen           
                if (nextPointer->occupiedByProcess == TRUE) {        
                    unsigned pid_ = nextPointer->pid;
					unsigned size_ = nextPointer->length;  
                    unsigned pid_1 = compactionPointer->pid;
                    unsigned size_1 = compactionPointer->length;                      
                    compactionPointer->pid = pid_; 
                    compactionPointer->length = size_; 
                    compactionPointer->occupiedByProcess = TRUE;  
                    nextPointer->pid = pid_1; 
                    nextPointer->length = size_1; 
                    nextPointer->occupiedByProcess = FALSE;
                } 
                // Falls der rechte Nachbar eine Lücke ist verschmelzt
				else if (nextPointer->occupiedByProcess == FALSE){   
					gap_ = compactionPointer;
					mergeEmptyFields(gap_);
					compactionPointer = first;
                } 
            }    
        } 
        compactionPointer = compactionPointer->next; 
    }
	processOutput();
} 
  
/* Funktion isAvailable überprüft ob eine passende Lücke für den Prozess vorhanden ist. 
    Die Rückgabewert ist TRUE oder FALSE.*/
Boolean isGabAvailable (unsigned pid, unsigned size) {  
    Boolean gabAvailable = FALSE; 
    struct feld *gabPointer = first;  
    if (gabPointer == NULL) { 
        gabAvailable = TRUE; 
    }  
    while (gabPointer != NULL) {    
        if ((gabPointer->occupiedByProcess == FALSE)  
            && (gabPointer->length >= size)) { 
                gabAvailable = TRUE; 
                break; 
        } 
        gabPointer = gabPointer->next; 
    } 
    return gabAvailable; 
} 
  
/* Funktion enQueue fügt den blockierten Prozess in die Warteschlange ein.*/
void enQueue(BlockedListElement_t blockedProcess) { 
    if (rear != MAX_PROCESSES) { 
        queue[rear++] = blockedProcess; 
        logQueue(TRUE,blockedProcess.pid); 
    }    
} 
  
/* Funktion deQueue entfernet den ersten blockierten Prozess von der Warteschlange, und rüft die compaction-Funktion wenn nötig auf.*/
void deQueue() { 
	Boolean gabAvailableForBlockedOne = FALSE;
	if (front != rear) {   
        gabAvailableForBlockedOne =  
            isGabAvailable(queue[front].pid, queue[front].size);
        if (gabAvailableForBlockedOne == TRUE) { 
            if(rear != front) { 
				logQueue(FALSE,queue[front].pid); 
				front++;
			}  
        } 
        else {
			if ( returnTotalGabSize() >= queue[front].size) {
				logHeadline("  \n	COMPACTION  \n");
				compaction(); 
				gabAvailableForBlockedOne =  
					isGabAvailable(queue[front].pid, queue[front].size); 
				if (gabAvailableForBlockedOne == TRUE) { 
					if(rear != front) {  
						logQueue(FALSE,queue[front].pid);
						front++;
					} 
				} 
			}
		}
    }
} 
  
/* Funktion returnTotalGabSize berechnet die gesamte Größe aller Lücken 
    und gibt diese zurück.*/
unsigned returnTotalGabSize() {  
    struct feld *totalGabSizePointer = first; 
    unsigned sumGabSize = 0;   
    while (totalGabSizePointer != NULL) {       
        if (totalGabSizePointer->occupiedByProcess == FALSE) { 
            sumGabSize = sumGabSize + totalGabSizePointer->length; 
        } 
        totalGabSizePointer = totalGabSizePointer->next; 
    } 
    return sumGabSize; 
}