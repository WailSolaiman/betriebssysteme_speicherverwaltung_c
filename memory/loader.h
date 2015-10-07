/* Include-file defining functions needed to load process information from	*/
/* a file and to load a process into memory (including initilaising its PCB)*/
#ifndef __LOADER__
#define __LOADER__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bs_types.h"


FILE* openConfigFile (FILE *file, const char * filename);
/* opens the  process info file specified by <filename>						*/
/* file has to refer to a valid FILE handle in the calling instance			*/
/* returns the file handle (which is NULL on error)							*/
/* Data in the file must be read using the function readNextProcess()		*/



PCB_t* readNextProcess (FILE* f, PCB_t* pProcess);
/* the file handle must refer to an already opened process info file		*/
/* the pointer pProcess must point to a valid PCB_t variable, the function  */
/* does not allocate memory for this. 										*/
/* Only components stored in the fie are updated.							*/ 
/* returns NULL if no process block was read, either because the file		*/
/* handle was invalid (NULL) or EOF was reached								*/
/* returns the pointer pProcess on success 									*/

int deleteProcess (PCB_t* pProcess);
/* Voids the PCB handed over in pProcess, this includes setting the valid-	*/
/* flag to invalid and setting other values to invalid values.				*/
/* retuns 0 on error and 1 on success										*/



#endif /* __LOADER__ */