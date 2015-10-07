/* Autoren der Erweiterung: Wail Soliaman, Alexander Luhmann */
/* header-file defining the interface of the scheduler */
/* the scheduler contains all functions required to decide which process    */
/* to run next                                                              */
/* +++  this version only implements a batch-scheuling using FIFO without   */
/* +++  SPOOLING. It needs to be extended to support multiprogramming       */
  
#ifndef __LOG__
#define __LOG__
  
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bs_types.h"
#include "globals.h"
#include "loader.h"
  
void logGeneric(char* message);
/* print the given general string to stdout and/or a log file               */
/* The time stamp (systemTime) will be prepended automatically              */
/* the linefeed is automatically appended                                   */
  
void logPid(unsigned pid, char* message);
/* print the given  string to stdout and/or a log file                      */
/* The time stamp (systemTime) and the  PID are prepended automatically     */
/* the linefeed is automatically appended                                   */
  
void logPidCompleteness(unsigned pid, unsigned done, unsigned length, 
                        char * message);
/* print the given  string to stdout and/or a log file                      */
/* The time stamp (systemTime) and the  PID are prepended automatically     */
/* the fraction of used CPU over process duratiuon is also prepended        */
/* the linefeed is automatically appended                                   */
  
void logPidMem(unsigned pid, char * message);
/* print the given  string to stdout and/or a log file                      */
/* The time stamp (systemTime) and the  PID are prepended automatically     */
/* the currenty used amount of physical memory is also prepended            */
/* the linefeed is automatically appended                                   */
  
void logPidInList(unsigned pid, unsigned start, unsigned length, 
                        unsigned aktuelSizeMemory, char * message);
void logPidFromList(unsigned pid,unsigned start, unsigned length, 
                        unsigned aktuelSizeMemory, char * message);
void logPidProcessTooLarge(unsigned pid); 
void logMerge(unsigned count,unsigned length, Boolean site);
void logQueue(Boolean inOrOut, unsigned pid);
void logFormat(char * message);
void logHeadline(char * message);
void logMainMemoryRepresentation(Boolean PidOrLenght,unsigned value);
void logFirstMemorySize(unsigned size);
#endif /* __LOG__ */