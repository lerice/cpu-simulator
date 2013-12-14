/*
   CITS2230 Programming Project
   Name:		Eric (Jun) Tan
   Student number:	20925931
   Date:		23/10/2012
*/

/* Standard includes */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/****************************************************************************
 *                               STRUCTURES                                 *
 ****************************************************************************/

/* Structure to hold information about a process */
typedef struct
{
	/* The name of the file of the job, without its extension */
	char jobName[50];
	/* The start time of the job */
	int startTime;
	/* The calculated length of the job */
	int jobLength;
	/* The number of the line last read into main memory (used only in RR) */
	int nLine;
	/* A boolean indicator for if the job is currently in the main memory (used only in RR) */
	bool isCM;
	/* Storage for the lines of the job */
	char **disk;
} JOB;

typedef struct
{
	/* A pointer to the start of the integer array containing its schedule */
	int *times;
	/* The (current) length of times array, used for memory allocation */
	int length;
} SCHEDULE;

typedef struct
{
	/* An array of strings (2), to hold 1 page in a frame */
	char **lines;
	/* The name of the job that this frame currently holds */
	char jobName[50];
} FRAME;

/****************************************************************************
 *                            GLOBAL VARIABLES                              *
 ****************************************************************************/

/* The number of job files to be processed */
extern int nJobs; 
/* An array/pointer of process's */
extern JOB *jobs;
/* An array/pointer of each process's calculated schedules */
extern SCHEDULE *schedules;
/* The simulation of cache memory, as an array of FRAMES (strings) */
extern FRAME *cache;
/* The simulation of main memory, as an array of FRAMES (strings) */
extern FRAME *mMem;
