/*
   CITS2230 Programming Project
   Name:		Eric (Jun) Tan
   Student number:	20925931
   Date:		23/10/2012
*/

#include "my_project.h"

/****************************************************************************
 *                            GLOBAL VARIABLES                              *
 ****************************************************************************/

//Simple initialisations of all global variables declared in my header file.
JOB *jobs = NULL;
SCHEDULE *schedules = NULL;
int nJobs = 0;
FRAME *cache = NULL;
FRAME *mMem = NULL;

/****************************************************************************
 *                             HELPER METHODS                               *
 ****************************************************************************/

/*
	Helper method to print a meaningful message to the user 
	and crash the program
*/
void printUsageAndCrash(char *progname)
{
	fprintf(stderr, "Usage: %s <-m d> [FCFS/RR d] in.file <out.file>\n", progname);
	exit(EXIT_FAILURE);
}

/*
	trimLine(line) removes any trailing new-line or carriage-return
	characters from the end of line. Copied from past projects.
*/
void trimLine(char line[])
{
	int i = 0;
	
	while (line[i] != '\0')
	{
		if (line[i] == '\n' || line[i] == '\r')
		{
			line[i] = '\0';
			break;
		}
		i++;
	}
}

/*
	dropExtension(line) removes any caracters in a string that appear
	after a full stop. Useful for printing names of files.
*/
void dropExtension(char line[])
{
	int i = 0;
	
	while (true)
	{
		if (line[i] == '.')
		{
			line[i] = '\0';
			break;
		}
		i++;
	}
}

/*
	Required method to compare two JOB's, to sort them in ascending 
	start time order, to be used by qsort.
 */
int compareTime(const void *p1, const void *p2)
{
	JOB *j1 = (JOB *)p1;
	int st1 = j1->startTime;
	JOB *j2 = (JOB *)p2;
	int st2 = j2->startTime;
	
	return st1 - st2;
}

/*
	Initialises the global variable for schedules and allocates memory for use.
 */
void setUpSchedules()
{
	//make memory to store new schedules details
	schedules = realloc(schedules, sizeof(SCHEDULE) * nJobs);
	//ensure the realloc was successful
	if (schedules == NULL)
	{
		perror("realloc failed");
		exit(EXIT_FAILURE);
	}
	
	//initialise variables in the schedules structure
	for (int i = 0; i < nJobs; i++)
	{
		schedules[i].times = NULL;
		schedules[i].length = 0;
	}
}

/*
	Prints the schedules to the terminal window in a meaningful way.
 */
void printSchedules(bool isFCFS)
{
	for (int a = 0; a < nJobs; a++)
	{
		printf("%-10s ", jobs[a].jobName);
		for (int i = 0; i < schedules[a].length; i++)
		{
			if (isFCFS)
			{
				printf("%-12d ", schedules[a].times[i]);
			}
			else if (i == schedules[a].length - 1)
			{
				printf("%d.", schedules[a].times[i]);
			}
			else
			{
				printf("%d, ", schedules[a].times[i]);
			}
		}
		printf("\n");
	}
}

/*
	Initialises the global variable for cache memory and main memory and allocates memory for use.
	Check that all allocations were successful.
 */
void setUpFrames()
{
	cache = malloc(sizeof(FRAME) * 2);
	if (cache == NULL)
	{
		perror("malloc failed");
		exit(EXIT_FAILURE);
	}
	
	mMem = malloc(sizeof(FRAME) * 8);
	if (mMem == NULL)
	{
		perror("malloc failed");
		exit(EXIT_FAILURE);
	}
	
	for (int i = 0; i < 2; i++)
	{
		cache[i].lines = malloc(sizeof(char *) * 2);
		if (cache[i].lines == NULL)
		{
			perror("malloc failed");
			exit(EXIT_FAILURE);
		}
		
		for (int j = 0; j < 2; j++)
		{
			cache[i].lines[j] = malloc(sizeof(char) * BUFSIZ);
			if (cache[i].lines[j] == NULL)
			{
				perror("malloc failed");
				exit(EXIT_FAILURE);
			}
		}
	}
	
	for (int k = 0; k < 8; k++)
	{
		mMem[k].lines = malloc(sizeof(char *) * 2);
		if (mMem[k].lines == NULL)
		{
			perror("malloc failed");
			exit(EXIT_FAILURE);
		}
		for (int l = 0; l < 2; l++)
		{
			mMem[k].lines[l] = malloc(sizeof(char) * BUFSIZ);
			if (mMem[k].lines[l] == NULL)
			{
				perror("malloc failed");
				exit(EXIT_FAILURE);
			}
		}
	}
}

/*
	Helper method to deal with the memory dump, outputting to the out file in a meaningful way.
 */
void dumpMemory(int d, char outfilename[])
{
	//declare the file pointer to the specified output file
	FILE *fp3;
	
	//ensure we can open the file if it exists, or create the file
	fp3 = fopen(outfilename, "w");
	if (fp3 == NULL)
	{
		fprintf(stderr, "Unable to process out-file: '%s'\n", outfilename);
		exit(EXIT_FAILURE);
	}
	
	fprintf(fp3, "1) Main Memory:\n\n");

	for (int i = 0; i < 8; i++)
	{
		fprintf(fp3, "Frame %d: %s\n%s\n%s\n\n", i + 1, mMem[i].jobName, mMem[i].lines[0], mMem[i].lines[1]);
	}
	
	fprintf(fp3, "2) Cache Memory:\n\n");
	for (int j = 0; j < 2; j++)
	{
		fprintf(fp3, "Frame %d: %s\n%s\n%s\n\n", j + 1, cache[j].jobName, cache[j].lines[0], cache[j].lines[1]);
	}
}

/*
	Helper method to increment the current time step, also checking if the time step
	passed is when a memory dump is occuring.
 */
void incCTS(int *CTS, int d, char outfilename[])
{
	(*CTS)++;
	if (*CTS == d)
	{
		dumpMemory(d, outfilename);
	}
}

/*
	Free's all the memory associated with this program before termination.
 */
void freeMemory()
{
	for (int i = 0; i < nJobs; i++)
	{
		for (int ii = 0; ii < jobs[i].jobLength; ii++)
		{
			free(jobs[i].disk[ii]);
		}
		free(jobs[i].disk);
		free(schedules[i].times);
	}
	free(jobs);
	free(schedules);
	
	for (int j = 0; j < 2; j++)
	{
		free(cache[j].lines[0]);
		free(cache[j].lines[1]);
		free(cache[j].lines);
	}
	free(cache);
	
	for (int k = 0; k < 8; k++)
	{
		free(mMem[k].lines[0]);
		free(mMem[k].lines[1]);
		free(mMem[k].lines);
	}
	free(mMem);
}

/****************************************************************************
 *                              MAIN METHODS                                *
 ****************************************************************************/

 /*
	Open a specific job file (whose name is in the input file) for reading,
	storing all the necessary information as well as the job itself to "disk".
*/
void processJob(char jobfilename[], int currentFile)
{
	//declare the file pointer and a storage string
	FILE *fp2;
	char line2[BUFSIZ];
	
	//ensure we can open the file for read access
	fp2 = fopen(jobfilename, "r");
	if (fp2 == NULL)
	{
		fprintf(stderr, "Unable to process job-file: '%s'\n", jobfilename);
		exit(EXIT_FAILURE);
	}
	
	//the first line of the job file is the start time (not a time step)
	int nLines = -1;
	int ifStatementLength = 0;
	//total if statement lengths
	int ISLT = 0;
	//current 'unfolded' job line
	int i = 0;
	
	//iterate through each line in the job file
	while (fgets(line2, sizeof line2, fp2) != NULL)
	{
		//remove trailing new line character
		trimLine(line2);
		nLines++;
		
		//if it is the first line, store the job start time
		if (nLines == 0)
		{
			jobs[currentFile].startTime = atoi(line2);
			continue;
		}
		
		//allocate memory for the string pointers
		jobs[currentFile].disk = realloc(jobs[currentFile].disk, sizeof(char *) * (i + 1));
		//allocate memory for the string
		jobs[currentFile].disk[i] = malloc(sizeof line2);
		
		//ensure the realloc was successful
		if (jobs[currentFile].disk == NULL || jobs[currentFile].disk[i] == NULL)
		{
			perror("alloc failed");
			exit(EXIT_FAILURE);
		}
		
		//copy the string into the newly allocated memory
		strcpy(jobs[currentFile].disk[i], line2);
		
		i++;
		
		int a, b;
		//if the line is an if statement
		if ((sscanf(line2, "if %*c < %d %*c = %*c + 1 goto %d", &a, &b)) == 2)
		{
			//calculate the time it will take to compute
			ifStatementLength = a * (nLines - b);
			int zz = i;
			i += ifStatementLength;
			
			//allocate memory to 'unfold' the job
			jobs[currentFile].disk = realloc(jobs[currentFile].disk, sizeof(char *) * i);
			
			//'unfold' the job and store the necessary lines
			for (int yy = 0; yy < a; yy++)
			{
				for (int ww = b + ISLT; ww < nLines + ISLT; ww++)
				{
					jobs[currentFile].disk[zz] = malloc(sizeof line2);
					strcpy(jobs[currentFile].disk[zz], jobs[currentFile].disk[ww - 1]);
					zz++;
				}
			}
			ISLT += ifStatementLength;
		}
	}
	
	//store the necessary job details
	dropExtension(jobfilename);
	strcpy(jobs[currentFile].jobName, jobfilename);
	jobs[currentFile].jobLength = i;
	jobs[currentFile].nLine = 0;
	jobs[currentFile].isCM = false;
}

/*
	Open the input file for reading and process each of the job files individually.
*/
void readInFile(char filename[])
{
	//declare the file pointer and a storage string
	FILE *fp;
	char line[BUFSIZ];
	
	//ensure we can open the file for read access
	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Unable to process in-file: '%s'\n", filename);
		exit(EXIT_FAILURE);
	}
	
	while (fgets(line, sizeof line, fp) != NULL)
	{
		trimLine(line);

		//make memory to store new job details
		jobs = realloc(jobs, sizeof(JOB) * (nJobs + 1));
		//ensure the realloc was successful
		if (jobs == NULL)
		{
			perror("realloc failed");
			exit(EXIT_FAILURE);
		}
		
		//read the job file
		processJob(line, nJobs);
		nJobs++;
	}
	
	//ensure the in-file was not empty (at least one job-file was found)
	if (nJobs == 0)
	{
		fprintf(stderr, "%s: the in-file is empty.\n", filename);
		exit(EXIT_FAILURE);
	}
	
	//sort the jobs array, in ascending start time order
	qsort(jobs, nJobs, sizeof(JOB), compareTime);
	
	setUpFrames();
	//initalise the schedules global variable
	setUpSchedules();
}

/*
	Process the job files in a FCFS schedule, with memory dump at time d to output file.
*/
void FCFS(int d, char outfilename[])
{
	//current time step
	int *CTS;
	*CTS = jobs[0].startTime;
	
	//the next frame to use in main memory
	int cFM = 0;
	
	//the next frame to use in cache
	int cFC = 0;
	
	for (int i = 0; i < nJobs; i++) {
		//if the (next) jobs start time is after the current time step, simply skip to the start time
		if (*CTS < jobs[i].startTime) 
		{
			*CTS = jobs[i].startTime;
		}
		
		schedules[i].length = 2;
		schedules[i].times = realloc(schedules[i].times, sizeof(int) * 2);
		
		//ensure the realloc was successful
		if (schedules[i].times == NULL)
		{
			perror("realloc failed");
			exit(EXIT_FAILURE);
		}
		
		schedules[i].times[0] = *CTS;
		//current job line
		int cJL = 0;
		
		while (jobs[i].jobLength > 0)
		{
			if (jobs[i].jobLength >= 4)
			{
				//load the next four lines from disk to main memory. no time cost.
				strcpy(mMem[cFM].jobName, jobs[i].jobName);
				strcpy(mMem[cFM].lines[0], jobs[i].disk[cJL]);
				strcpy(mMem[cFM].lines[1], jobs[i].disk[cJL + 1]);
				strcpy(mMem[(cFM + 1) % 8].jobName, jobs[i].jobName);
				strcpy(mMem[(cFM + 1) % 8].lines[0], jobs[i].disk[cJL + 2]);
				strcpy(mMem[(cFM + 1) % 8].lines[1], jobs[i].disk[cJL + 3]);
				//access the first line from main memory. bring 4 lines to cache. 
				strcpy(cache[cFC].jobName, mMem[cFM].jobName);
				strcpy(cache[cFC].lines[0], mMem[cFM].lines[0]);
				//2 time units.
				incCTS(CTS, d, outfilename);
				incCTS(CTS, d, outfilename);
				strcpy(cache[cFC].lines[1], mMem[cFM].lines[1]); 
				//1 time unit, reading cache
				incCTS(CTS, d, outfilename);
				strcpy(cache[(cFC + 1) % 2].jobName, mMem[(cFM + 1) % 8].jobName);
				strcpy(cache[(cFC + 1) % 2].lines[0], mMem[(cFM + 1) % 8].lines[0]); 
				//1 time unit, reading cache
				incCTS(CTS, d, outfilename);
				strcpy(cache[(cFC + 1) % 2].lines[1], mMem[(cFM + 1) % 8].lines[1]); 
				//1 time unit, reading cache
				incCTS(CTS, d, outfilename);

				//update current frames and job lines
				cFM = (cFM + 2) % 8;
				cFC = (cFC + 2) % 2;
				cJL += 4;
				
				//update job length remaining
				jobs[i].jobLength -= 4;
			}
			else if (jobs[i].jobLength == 3)
			{
				//load the last three lines from disk to main memory. no time cost.
				strcpy(mMem[cFM].jobName, jobs[i].jobName);
				strcpy(mMem[cFM].lines[0], jobs[i].disk[cJL]);
				strcpy(mMem[cFM].lines[1], jobs[i].disk[cJL + 1]);
				strcpy(mMem[(cFM + 1) % 8].jobName, jobs[i].jobName);
				strcpy(mMem[(cFM + 1) % 8].lines[0], jobs[i].disk[cJL + 2]);
				//access the first line from main memory. bring 3 lines to cache. 
				strcpy(cache[cFC].jobName, mMem[cFM].jobName);
				strcpy(cache[cFC].lines[0], mMem[cFM].lines[0]);
				//2 time units.
				incCTS(CTS, d, outfilename);
				incCTS(CTS, d, outfilename);
				strcpy(cache[cFC].lines[1], mMem[cFM].lines[1]); 
				//1 time unit, reading cache
				incCTS(CTS, d, outfilename);
				strcpy(cache[(cFC + 1) % 2].jobName, mMem[(cFM + 1) % 8].jobName);
				strcpy(cache[(cFC + 1) % 2].lines[0], mMem[(cFM + 1) % 8].lines[0]); 
				//1 time unit, reading cache
				incCTS(CTS, d, outfilename);

				//update current frames and job lines
				cFM = (cFM + 2) % 8;
				cFC = (cFC + 2) % 2;
				cJL += 3;
				
				//update job length remaining
				jobs[i].jobLength -= 3;
			}
			else if (jobs[i].jobLength == 2)
			{
				//load the last two lines from disk to main memory. no time cost.
				strcpy(mMem[cFM].jobName, jobs[i].jobName);
				strcpy(mMem[cFM].lines[0], jobs[i].disk[cJL]);
				strcpy(mMem[cFM].lines[1], jobs[i].disk[cJL + 1]);
				//access the first line from main memory. bring 2 lines to cache. 
				strcpy(cache[cFC].jobName, mMem[cFM].jobName);
				strcpy(cache[cFC].lines[0], mMem[cFM].lines[0]);
				//2 time units.
				incCTS(CTS, d, outfilename);
				incCTS(CTS, d, outfilename);
				strcpy(cache[cFC].lines[1], mMem[cFM].lines[1]); 
				//1 time unit, reading cache
				incCTS(CTS, d, outfilename);

				//update current frames and job lines
				cFM = (cFM + 1) % 8;
				cFC = (cFC + 1) % 2;
				cJL += 2;
				
				//update job length remaining
				jobs[i].jobLength -= 2;
			}
			else //job length remaining is 1
			{
				//load the last line from disk to main memory. no time cost.
				strcpy(mMem[cFM].jobName, jobs[i].jobName);
				strcpy(mMem[cFM].lines[0], jobs[i].disk[cJL]);
				//access the last line from main memory. bring to cache. 2 time units.
				strcpy(cache[cFC].jobName, mMem[cFM].jobName);
				strcpy(cache[cFC].lines[0], mMem[cFM].lines[0]);
				//2 time units.
				incCTS(CTS, d, outfilename);
				incCTS(CTS, d, outfilename);
				
				//update current frames and job lines
				cFM = (cFM + 1) % 8;
				cFC = (cFC + 1) % 2;
				cJL += 1;
				//update job length remaining
				jobs[i].jobLength -= 1;
			}
		}
		schedules[i].times[1] = *CTS - 1;
	}
	
	printf("<Job Name> <Start Time> <End Time>\n");
	printSchedules(true);
	
	//check that the memory dump was processed
	if (d > *CTS)
	{
		printf("\nYour specified time step for the memory dump occurs after the schedule has finished, Output file not made.\n");
	}
}

/*
	Process the job files in a RR schedule, with memory dump at time d to output file
*/
void RR(int tQ , int d, char outfilename[])
{
	//calculate the total time needed to complete all jobs, helpful for RR
	int totalJobTime = 0;
	for (int a = 0; a < nJobs; a++)
	{
		totalJobTime += jobs[a].jobLength;
	}
	
	//counter for the current job we are looking at
	int i = 0;
	//current time step, start at the first job
	int *CTS;
	*CTS = jobs[0].startTime;
	
	//the next frame to use in main memory
	int cFM = 0;
	
	//the next frame to use in cache
	int cFC = 0;
	
	while (totalJobTime > 0)
	{
		//if the least recently checked job hasn't started yet, or has finished, skip
		if (*CTS < jobs[i].startTime || jobs[i].jobLength == 0)
		{
			i = (i + 1) % nJobs;
			continue;
		}

		//the job initializes
		schedules[i].length++;
		schedules[i].times = realloc(schedules[i].times, sizeof(int) * schedules[i].length);
		
		//ensure the realloc was successful
		if (schedules[i].times == NULL)
		{
			perror("realloc failed");
			exit(EXIT_FAILURE);
		}
		
		//store the start time
		schedules[i].times[schedules[i].length - 1] = *CTS;
		
		//make a copy of the time quantum, for counting purposes
		int tQc = tQ;
		//number of lines to read from cache
		int nLFFR = 4;
		//the next page line number to read(0 or 1)
		int pLN = 0;
		
		//loop around the time quantum, until it times out or a page faults occurs
		while (tQc > 0 && jobs[i].jobLength > 0)
		{
			// if the current job is not found in the main memory or cache, load it
			if (!jobs[i].isCM)
			{
				if (jobs[i].jobLength >= 4)
				{
					if (tQc < 2)
					{
						break;
					}
					
					//load the next four lines from disk to main memory. no time cost.
					strcpy(mMem[cFM].jobName, jobs[i].jobName);
					strcpy(mMem[cFM].lines[0], jobs[i].disk[jobs[i].nLine]);
					strcpy(mMem[cFM].lines[1], jobs[i].disk[jobs[i].nLine + 1]);
					strcpy(mMem[(cFM + 1) % 8].jobName, jobs[i].jobName);
					strcpy(mMem[(cFM + 1) % 8].lines[0], jobs[i].disk[jobs[i].nLine + 2]);
					strcpy(mMem[(cFM + 1) % 8].lines[1], jobs[i].disk[jobs[i].nLine + 3]);
					//access the first line from main memory. bring 4 lines to cache. 
					strcpy(cache[cFC].jobName, mMem[cFM].jobName);
					strcpy(cache[cFC].lines[0], mMem[cFM].lines[0]);
					strcpy(cache[cFC].lines[1], mMem[cFM].lines[1]);
					strcpy(cache[(cFC + 1) % 2].jobName, mMem[(cFM + 1) % 8].jobName);
					strcpy(cache[(cFC + 1) % 2].lines[0], mMem[(cFM + 1) % 8].lines[0]);
					strcpy(cache[(cFC + 1) % 2].lines[1], mMem[(cFM + 1) % 8].lines[1]);
					//2 time units.
					incCTS(CTS, d, outfilename);
					incCTS(CTS, d, outfilename);
					tQc -= 2;
					
					int a, b;
					//if the line being read is an if statement
					if ((sscanf(cache[cFC].lines[0], "if %*c < %d %*c = %*c + 1 goto %d", &a, &b)) == 2)
					{
						//check if the 'goto' line will be out of main memory (see readme):
						if (b + 4 < jobs[i].nLine)
						{
							//a page fault occurs: job stops execution even if time quantum isn't over
							tQc = 0;
						}
					}
					
					//update current frames and job lines
					cFM = (cFM + 2) % 8;
					cFC = (cFC + 2) % 2;
					
					nLFFR = 3;
					pLN = 1;
				}
				else if (jobs[i].jobLength == 3)
				{
					if (tQc < 2)
					{
						break;
					}
				
					//load the last three lines from disk to main memory. no time cost.
					strcpy(mMem[cFM].jobName, jobs[i].jobName);
					strcpy(mMem[cFM].lines[0], jobs[i].disk[jobs[i].nLine]);
					strcpy(mMem[cFM].lines[1], jobs[i].disk[jobs[i].nLine + 1]);
					strcpy(mMem[(cFM + 1) % 8].jobName, jobs[i].jobName);
					strcpy(mMem[(cFM + 1) % 8].lines[0], jobs[i].disk[jobs[i].nLine + 2]);
					//access the first line from main memory. bring 3 lines to cache. 
					strcpy(cache[cFC].jobName, mMem[cFM].jobName);
					strcpy(cache[cFC].lines[0], mMem[cFM].lines[0]);
					strcpy(cache[cFC].lines[1], mMem[cFM].lines[1]);
					strcpy(cache[(cFC + 1) % 2].jobName, mMem[(cFM + 1) % 8].jobName);
					strcpy(cache[(cFC + 1) % 2].lines[0], mMem[(cFM + 1) % 8].lines[0]);
					//2 time units.
					incCTS(CTS, d, outfilename);
					incCTS(CTS, d, outfilename);
					tQc -= 2;
					
					int a, b;
					//if the line being read is an if statement
					if ((sscanf(cache[cFC].lines[0], "if %*c < %d %*c = %*c + 1 goto %d", &a, &b)) == 2)
					{
						//check if the 'goto' line will be out of main memory (see readme):
						if (b + 4 < jobs[i].nLine)
						{
							//a page fault occurs: job stops execution even if time quantum isn't over
							tQc = 0;
						}
					}
					
					//update current frames and job lines
					cFM = (cFM + 2) % 8;
					cFC = (cFC + 2) % 2;
					
					nLFFR = 2;
					pLN = 1;
				}
				else if (jobs[i].jobLength == 2)
				{
					if (tQc < 2)
					{
						break;
					}
				
					//load the last two lines from disk to main memory. no time cost.
					strcpy(mMem[cFM].jobName, jobs[i].jobName);
					strcpy(mMem[cFM].lines[0], jobs[i].disk[jobs[i].nLine]);
					strcpy(mMem[cFM].lines[1], jobs[i].disk[jobs[i].nLine + 1]);
					//access the first line from main memory. bring 2 lines to cache. 
					strcpy(cache[cFC].jobName, mMem[cFM].jobName);
					strcpy(cache[cFC].lines[0], mMem[cFM].lines[0]);
					strcpy(cache[cFC].lines[1], mMem[cFM].lines[1]);
					//2 time units.
					incCTS(CTS, d, outfilename);
					incCTS(CTS, d, outfilename);
					tQc -= 2;
					
					int a, b;
					//if the line being read is an if statement
					if ((sscanf(cache[cFC].lines[0], "if %*c < %d %*c = %*c + 1 goto %d", &a, &b)) == 2)
					{
						//check if the 'goto' line will be out of main memory (see readme):
						if (b + 4 < jobs[i].nLine)
						{
							//a page fault occurs: job stops execution even if time quantum isn't over
							tQc = 0;
						}
					}
					
					//update current frames and job lines
					cFM = (cFM + 1) % 8;
					cFC = (cFC + 1) % 2;
					
					nLFFR = 1;
					pLN = 1;
				}
				else
				{
					if (tQc < 2)
					{
						break;
					}
					
					//load the last line from disk to main memory. no time cost.
					strcpy(mMem[cFM].jobName, jobs[i].jobName);
					strcpy(mMem[cFM].lines[0], jobs[i].disk[jobs[i].nLine]);
					//access the line from main memory. bring line to cache.			
					strcpy(cache[cFC].jobName, mMem[cFM].jobName);
					strcpy(cache[cFC].lines[0], mMem[cFM].lines[0]);
					//2 time units.
					incCTS(CTS, d, outfilename);
					incCTS(CTS, d, outfilename);
					tQc -= 2;
					
					int a, b;
					//if the line being read is an if statement
					if ((sscanf(cache[cFC].lines[0], "if %*c < %d %*c = %*c + 1 goto %d", &a, &b)) == 2)
					{
						//check if the 'goto' line will be out of main memory (see readme):
						if (b + 4 < jobs[i].nLine)
						{
							//a page fault occurs: job stops execution even if time quantum isn't over
							tQc = 0;
						}
					}
					
					//update current frames and job lines
					cFM = (cFM + 1) % 8;
					cFC = (cFC + 1) % 2;
					
					nLFFR = 0;
					pLN = 1;
					jobs[i].isCM = false;
				}
				
				jobs[i].nLine++;
				
				//update job length remaining
				jobs[i].jobLength--;
				jobs[i].isCM = true;
				jobs[(i - 1 + nJobs) % nJobs].isCM = false;
				totalJobTime--;
				
			} 
			else //otherwise it is found in the main memory, read 1 line from cache
			{
				//printf("%s\n", cache[cFC].lines[pLN]);
				
				int a, b;
				//if the line being read is an if statement
				if ((sscanf(cache[cFC].lines[pLN], "if %*c < %d %*c = %*c + 1 goto %d", &a, &b)) == 2)
				{
					//check if the 'goto' line will be out of main memory (see readme):
					if (b + 3 < jobs[i].nLine)
					{
						//a page fault occurs: job stops execution even if time quantum isn't over
						tQc = 0;
					}
				}
				
				pLN = (pLN + 1) % 2;
				
				//if 2 lines of frame 1 of cache has been read, go to next frame:
				if (pLN == 0)
				{
					cFC = (cFC + 1) % 2;
				}
				
				nLFFR--;
				//if all 4 lines from cache have been read, set indicators
				if (nLFFR == 0)
				{
					nLFFR = 4;
					jobs[i].isCM = false;
				}
				
				tQc--;
				//1 time unit, reading cache
				incCTS(CTS, d, outfilename);
				
				jobs[i].nLine++;
				//update job length remaining
				jobs[i].jobLength--;
				totalJobTime--;
				
			}
		}
		i = (i + 1) % nJobs;
	}
	
	printf("<Job Name> <Start Time(s)>\n");
	printSchedules(false);
	
	//check that the memory dump was processed
	if (d > *CTS)
	{
		printf("\nYour specified time step for the memory dump occurs after the schedule has finished, Output file not made.\n");
	}
}

/*
	The main method. Processes command line arguments.
 */

int main(int argc, char *argv[])
{
	//ensure the command line arguments are proper before processing schedule.
	switch (argc)
	{
		case 3:
			if (strcmp(argv[1], "FCFS") == 0)
			{
				readInFile(argv[2]);
				//process in FCFS schedule, without a memory dump.
				FCFS(-1, "");
			}
			else
			{
				printUsageAndCrash(argv[0]);
			}
			break;
		case 4:
			if (strcmp(argv[1], "RR") == 0 && atoi(argv[2]) > 1)
			{
				readInFile(argv[3]);
				//process in RR schedule, without a memory dump.
				RR(atoi(argv[2]), -1, "");
			}
			else
			{
				printUsageAndCrash(argv[0]);
			}
			break;
		case 6:
			if (strcmp(argv[1], "-m") == 0 && atoi(argv[2]) > 1 && strcmp(argv[3], "FCFS") == 0)
			{
				readInFile(argv[4]);
				FCFS(atoi(argv[2]), argv[5]);
			}
			else
			{
				printUsageAndCrash(argv[0]);
			}
			break;
		case 7:
			if (strcmp(argv[1], "-m") == 0 && atoi(argv[2]) > 1 && strcmp(argv[3], "RR") == 0 && atoi(argv[4]) > 1)
			{
				readInFile(argv[5]);
				RR(atoi(argv[4]), atoi(argv[2]), argv[6]);
			}
			else
			{
				printUsageAndCrash(argv[0]);
			}
			break;
		default:
			printUsageAndCrash(argv[0]);
	}
	//free the memory associated with the global variables
	freeMemory();
	//terminate program successfully
	exit(EXIT_SUCCESS);
	return 0;
}
