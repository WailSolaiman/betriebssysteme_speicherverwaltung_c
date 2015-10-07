/* Implementation of the loader. Responsible for reading batch,		*/
/* generating and destroying of processes  */
/* for comments on the functions see the associated .h-file */

#define	_CRT_SECURE_NO_WARNINGS		// suppress legacy warnings 

/* ---------------------------------------------------------------- */
/* Include required external definitions */
#include <math.h>
#include "loader.h"
#include "bs_types.h"
#include "globals.h"
#include "log.h"

/* ---------------------------------------------------------------- */
/*                Declarations of local helper functions            */
// none

/* ---------------------------------------------------------------- */
/*                Externally available functions                    */
/* ---------------------------------------------------------------- */

FILE* openConfigFile (FILE *file, const char * filename)
{

	char linebuffer[129]="x"; // read buffer for file-input
	char charbuffer='x';		// read buffer for file-input
	unsigned ownerID=0xFF, start=0xFF, duration=0xFF; // local buffer for reading proccess properties
	char ownerIDStr[21]="", startStr[21]=""; // buffers for reading process type-string
	char durationStr[21]="", processTypeStr [21]=""; 	// buffers for reading process type-string

	// try to open file for read
	file = fopen (filename, "r");
	// test for success and error handling
	if (file == NULL)
		{
		logGeneric("Fehler beim Oeffnen der Datei mit Prozessinformationen.");
		return NULL;
		}
	// skip first line, only a comment
	if (!feof(file))
		fgets(linebuffer, 128, file);
		printf("Read from File: %s ", linebuffer);
	return file;
}


PCB_t* readNextProcess (FILE* f, PCB_t* pProcess)
{
	char linebuffer[129]="";		// read buffer for file-input
	char processTypeStr [21]=""; 	// buffers for reading process type-string
	if (f==NULL) return NULL;		// error: file handle not initialised
	if (feof(f))
		{
		fclose (f);			// close the file on reaching EOF
		return NULL;		// error occured (EOF reached)
		}
	else 
	{	
		fgets(linebuffer, 128, f);
		if (strcmp(linebuffer, "")==0)
			return NULL;			// error occured: line is empty
		else
		{
			sscanf (linebuffer, "%u %u %u %u %s", &pProcess->ownerID, &pProcess->start, 
				&pProcess->duration, &pProcess->size, processTypeStr);

			if (strcmp(processTypeStr, "os")==0)
				pProcess->type=os;
			else if (strcmp(processTypeStr, "interactive")==0)
				pProcess->type=interactive;
			else if (strcmp(processTypeStr, "batch")==0)
				pProcess->type=batch;
			else if (strcmp(processTypeStr, "background")==0)
				pProcess->type=background;
			else if (strcmp(processTypeStr, "foreground")==0)
				pProcess->type=foreground;
			else pProcess->type=os;
		}
		// pProcess->pid=getNextPid();		-> moved to core loop
		printf( "For Process %u read from File: %u %u %u %u %s\n", pProcess->pid, pProcess->ownerID, 
			pProcess->start, pProcess->duration, pProcess->size, processTypeStr);

	}
	return pProcess; 
}



int deleteProcess (PCB_t* pProcess)
/* Voids the PCB handed over in pProcess, this includes setting the valid-	*/
/* flag to invalid and setting other values to invalid values.				*/
/* retuns 0 on error and 1 on success										*/
{
	if (pProcess==NULL)
		return 0; 
	else {	/* PCB correctly passed, now delete it */
		pProcess->valid=FALSE; 
		pProcess->pid=0; 
		pProcess->ppid=0;
		pProcess->ownerID=0;
		pProcess->start=0;
		pProcess->duration=0;
		pProcess->size=0;
		pProcess->usedCPU=0;
		pProcess->type=os;
		pProcess->status=ended;
		return 1; 
	}
}