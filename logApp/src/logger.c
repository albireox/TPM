/*==============================================================================
//
//  L O G G E R . C
//
//  This module gets loaded into the VxWorks node on the mountain. It spools
//	out data to a specified file.
//
//	$Id: logger.c,v 1.34 2000/06/30 20:10:27 peregrin Exp $
==============================================================================*/
#include <vxWorks.h>
#include <errnoLib.h>
#include <nfsDrv.h>
#include <nfsLib.h>
#include <usrLib.h>
#include <tyLib.h>
#include <ledLib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include "errMdef.h"
#include "timer.h"
#include "lf.h"
#include "data_collection.h"
#include "dbAccess.h"
#include "tsDefs.h"

#define TPMERR_BADMOUNT		1
#define	TPMERR_CREATEREG	2
#define	TPMERR_CREATE		3
#define	TPMERR_TASK		4
#define	TPMERR_SEMCREATE	5
#define	TPMERR_LINEEDIT		6

#define	TPMWRN_OPENED		1

#define DEF_SAMPLE_RATE		200
#define MAX_REGISTERS 		300
#define	REG_DISABLE		0x00
#define	REG_ENABLE		0x02

#define OP_READ		0
#define	OP_WRITE	0

typedef enum {
	statInterrupts, statDataWritten, statRereadVersion,
	statTotal
} STATS;

struct RegDef {
	char name[REG_NAME_LEN + 1];
	unsigned short rate;
        int flags;
	RegType type;
	struct dbAddr addr; 
        int vflags;
        HLOGREG reg;
        unsigned short vrate;
};

struct CmdInfo {
	char* name;
	void (*func)(void);
	char* brief;
	char* verbose;
};

typedef int (*ApplyOp)(int, struct RegDef *, void const*);

/*
 *		Local function prototypes...
 */
static int checkVersion(void);
static STATUS createNewTask(void);
static STATUS createNfsConnection(void);
static STATUS configDatabase(char *);
static STATUS configRegisters(char *);
static STATUS destroyNfsConnection(void);
static void disable(void);
static int disableOp(int, struct RegDef *, void const*);
static void dump(void);
static int dumpOp(int, struct RegDef *, void const*);
static void enable(void);
static int enableOp(int, struct RegDef *, void const*);
static void foreach(ApplyOp, void *, int);
static void list(void);
static int listOp(int, struct RegDef *, void const*);
static void printHeader(void);
static void printHelp(void);
static void printStatus(void);
static void resetStats(void);
static void sample(void);
static void sampleData(void);
static int sampleOp(int, struct RegDef *, void const*);
static STATUS setupLogFile(char *, char *);
static void startLog(void);
static void stopLog(void);
static int stricmp(char const*, char const*);
static void taskMain(void);

extern void slaCldj(int, int, int, double*, int*);

/*
 *	Static data definitions...
 */

static HLOGFILE lf = 0;
static SEM_ID semNotify = 0;
static unsigned short sampleRate = DEF_SAMPLE_RATE;
static unsigned long stats[statTotal] = { 0 };

/*
 *	Constant local data...
 */

static struct CmdInfo const cmdInfo[] = {
	{
		"disable", disable,
		"Specifies which registers should not get logged.",
		"disable reglist\n"
		"\n"
		"The registers specified in the list are disabled."
	},
	{
		"dump", dump,
		"Display current values.",
		"dump\n"
		"\n"
		"Dumps the values of the current sampled registers. This command\n"
		"only works when a log file is being written."
	},
	{
		"enable", enable,
		"Specifies which registers can be logged.",
		"enable reglist\n"
		"\n"
		"The registers specified in the list are enabled."
	},
	{
		"help", printHelp,
		"Displays help for a specific command. Try 'help help'.",
		"help [command]\n"
		"\n"
		"If no arguments are given, 'help' displays a list of commands\n"
		"that are available. If a command is specified, more detailed\n"
		"help is displayed."
	},
	{
		"list", list,
		"Lists the status of the registers.",
		"list\n"
		"\n"
		"The current settings are displayed for the specified registers.\n"
		"If no registers are specified, all of them are displayed."
	},
	{
		"quit", NULL,
		"Returns to VxWorks.",
		"quit\n"
		"\n"
		"Returns to the VxWorks shell prompt."
	},
	{
		"sample", sample,
		"Alters the sample rate of a register.",
		"sample { default | { rate [reglist] } }\n"
		"\n"
		"By specifying 'default', all the default sample rates are\n"
		"reloaded. A register's sample rate can be changed by specifying\n"
		"its name followed by the new rate (in Hertz). If no registers are\n"
		"specified, the log file's rate is updated.\n\n"
		"The sampling rates cannot be modified while a log file is active.\n"
	},
	{
		"start", startLog,
		"Starts logging telescope data to a file.",
		"start filename {config_file}\n"
		"\n"
		"This command starts logging telescope data to a file. If the\n"
		"file already exists, it is overwritten. Only one file can be\n"
		"open at a time. If a configuration file is specified, then the\n"
		"registers and scan/log rates can be changed.\n"
	},
	{
		"status", printStatus,
		"Prints status information for the logger.",
		"status\n"
		"\n"
		"This command prints status information. This is a \"feel-good\"\n"
		"command to show that the TPM is still running."
	},
	{
		"stop", stopLog,
		"Stops logging telescope data.",
		"stop\n"
		"\n"
		"Stops logging data to a file and closes the file."
	}
};

static char const sep[] = " \t";

/* This variable holds the default path to where the log files will be
   held. There is code that expects this string to contain the
   trailing '/' */

static char const thePath[] = "/mcptpm/";
static char const configPath[] = "/mcptpm/config/";

/* This array holds all the information pertaining to register
   handles. We make it 'const' for debugging purposes. */

/* This pointer gets us to the shared data structure. */

#define data ((struct SDSS_FRAME volatile*) 0x02800002)

static struct RegDef regList[MAX_REGISTERS];
static int N_REG = 0;

/*------------------------------------------------------------------------------
  checkVersion

  Returns true or false, depending whether we're in sync with the MCP's
  shared data.
------------------------------------------------------------------------------*/
static int checkVersion(void)
{
	unsigned char const vers = data->vers;

	if (SDSS_FRAME_VERSION != vers)
		if (SDSS_FRAME_VERSION != vers) {
			epicsPrintf("**** Looking for v%d format, found v%d. Need to "
					"recompile.\n", SDSS_FRAME_VERSION, vers);
			return 0;
		} else
			++stats[statRereadVersion];
	return 1;
}

/*------------------------------------------------------------------------------
  CloseLog

  Closes a currently opened log file.
------------------------------------------------------------------------------*/
int CloseLog(void)
{
	/* Check to see if the log file is opened. If the the handle is
	   not null, we assume that a file is opened. */

	if (lf) {
		int ii;

		TimerStop();

		semDelete(semNotify);
		semNotify = 0;
		taskDelay(60);

		/* Close the file and zero out the handle to indicate there is
		   no active log file. */
		logClose(&lf);

		/* Clear out the register handles since they are going to
           refer to illegal memory locations. */

		for (ii = 0; ii < N_REG; ++ii)
			regList[ii].reg = 0;

		/* Free up our NFS resource. */

		destroyNfsConnection();
	}
	return 0;
}

/*------------------------------------------------------------------------------
  cmpName

  Compares the name of a device with a mask. The mask can contain
  wildcards. For instance:

		.		matches any character
		*		matches zero or more "any" characters
------------------------------------------------------------------------------*/
static int cmpName(char const* name, char const* mask)
{
	assert(name);
	assert(mask);

	/* Loop until all characters are used up. */

	while (*name) {

		/* If the wildcard character is an asterisk, then we loop the
           name until we find the next character. */

		if ('*' == *mask) {

			/* Make sure we eat up any following asterisks. */

			do

				/* If we find an embedded '.', we have to process
                   it. */

				if ('.' == *++mask)
					if (!*name)
						while ('*' == *++mask)
							;
					else {
						++mask;
						++name;
					}
			while ('*' == *mask);

			/* Move the name up to the next matching character. */

			while (*name && tolower(*name) != tolower(*mask))
				++name;
		}

		/* If we find a '.', then we match one character. Otherwise,
           if the characters match, we can continue. */

		else if ('.' == *mask || tolower(*name) == tolower(*mask)) {
			++mask;
			++name;
		} else
			break;
	}
	return tolower(*name) == tolower(*mask);
}

/*------------------------------------------------------------------------------
  createNewTask
------------------------------------------------------------------------------*/
static STATUS createNewTask(void)
{
	static char const tskName[] = "tTpmTask";

	return ERROR != taskSpawn((char*) tskName, 200, VX_FP_TASK, 16384,
							  (FUNCPTR) taskMain, 0, 0, 0, 0, 0, 0, 0, 0, 0,
							  0) ? OK : ERROR;
}

/*------------------------------------------------------------------------------
  createNfsConnection

  This function sets up the NFS connection to 'sdsshost'. It also
  changes the directory.
------------------------------------------------------------------------------*/
static STATUS createNfsConnection(void)
{
	/* Attempt to mount the remote drive and set the default
       directory. */

	if (OK == nfsMount("sdsshost", (char*) thePath, 0))
		return OK;
	else {
		epicsPrintf("Couldn't mount NFS drive from sdsshost.\n");
		return ERROR;
	}
}

/*------------------------------------------------------------------------------
 configDatabase 
------------------------------------------------------------------------------*/
static STATUS configDatabase(char *cfg)
{
	int ii = 0;
	char pvname[40];
	char str[81];
	struct RegDef * ptr;
   	FILE * fp;
	char name[40];
	float period;
        int flag;
	int disa;
	struct dbAddr addr;
	int status;
	char scanstr[40];
	char default_cfg[]="default.cfg";
	char cfgfull[40];

        if (!cfg) cfg = default_cfg;	
	sprintf(cfgfull, "%s%s", configPath, cfg);

 	if (!(fp = fopen(cfgfull, "r"))) {
	    epicsPrintf("Error opening config file: %s\n", cfgfull);
	    return ERROR;
	}

	/* Loop through each register definition and register it. */
        while (fgets(str, 80, fp)) {
 	    if (str[0] == '#') continue;

	    if (sscanf(str, "%s %f %d", &name, &period, &flag) != 3) {
		epicsPrintf("Error parsing: %s", str);
		continue;
	    } 


	    sprintf(pvname, "tpm_%s.VAL", name);
	    status = dbNameToAddr(pvname, &addr);	
	    if (status != 0L) {
	         epicsPrintf("%s: not found!\n", pvname);
	         continue;
	    } 

	    if (flag == 2) {
	        ptr = regList + ii++;
		ptr->addr = addr;
	    }

/*
 * Set the disable flag (DISV = 1):
 *	0 - record is disabled. => DISA = 1 
 *	1 - scan record only. => DISA = 0
 *	2 - scan and log record. => DISA = 0
*/
	    disa = (flag == 0) ? 1 : 0;
	    sprintf(pvname, "tpm_%s.DISA", name);
	    status = dbNameToAddr(pvname, &addr);
	    status = dbPutField(&addr, DBR_SHORT, &disa, 1);
	    if (status != 0) { epicsPrintf("dbPutField failed: %s\n", pvname); continue; }

/*
 * Set the record scan rate.
*/
	    sprintf(pvname, "tpm_%s.SCAN", name);
            status = dbNameToAddr(pvname, &addr);
	    if (flag == 0) {
		sprintf(scanstr, " Passive");
	    } else if (period >= 1.0) {
		sprintf(scanstr, " %.0f second", period);
	    } else if (period >= 0.1) {
		sprintf(scanstr, "%.1f second", period);
	    } else if (period >= 0.01) {
		sprintf(scanstr, "%.2f second", period);
	    } else {
		sprintf(scanstr, "%.3f second", period);
	    } 
	    status = dbPutField(&addr, DBR_STRING,  &scanstr[1], 1); /* drop first char */
	    if (status != 0) { epicsPrintf("dbPutField failed: %s\n", pvname); continue; }
	}
	N_REG = ii;
	return OK;
}

/*------------------------------------------------------------------------------
  configRegisters
------------------------------------------------------------------------------*/
static STATUS configRegisters(char *cfg)
{
	int ii = 0;
	char pvname[40];
	char str[81];
	struct RegDef * ptr;
   	FILE * fp;
	char name[40];
	float period;
        int flag;
	int disa;
	int status;
	char scanstr[40];
	char default_cfg[]="default.cfg";
	char cfgfull[40];

        if (!cfg) cfg = default_cfg;	
	sprintf(cfgfull, "%s%s", configPath, cfg);

 	if (!(fp = fopen(cfgfull, "r"))) {
	    epicsPrintf("Error opening config file: %s\n", cfgfull);
	    return ERROR;
	}

	/* Loop through each register definition and register it. */
        while (fgets(str, 80, fp)) {
 	    if (str[0] == '#') continue;

	    if (sscanf(str, "%s %f %d", &name, &period, &flag) != 3) {
		epicsPrintf("Error parsing: %s", str);
		continue;
	    } 

	    ptr = regList + ii;
	    ii++;

/*
 * Now that the record processing is configured, we
 * set up the TPM logger.
*/ 
	    strcpy(ptr->name, name);
	    ptr->vrate = floor(DEF_SAMPLE_RATE * period + 0.5);
            ptr->type = REG_INT32;
	    ptr->vflags = flag;

	    if (ptr->vflags & REG_ENABLE) {
	        ptr->reg = logCreateRegister(lf, ptr->name, "", 
						 ptr->type, ptr->vrate);

	      /* If a NULL handle is returned, we need to report it and
			   then abort the entire operation. */

		if (!ptr->reg) {
		   epicsPrintf("Error creating register %s: lf=%lx type=%d vrate=%d\n", 
				ptr->name, lf, ptr->type, ptr->vrate);
		   return ERROR;
		} 
	    }
	}
	return OK;
}

/*------------------------------------------------------------------------------
  destroyNfsConnection
------------------------------------------------------------------------------*/
static STATUS destroyNfsConnection(void)
{
	return nfsUnmount((char*) thePath);
}

/*------------------------------------------------------------------------------
  disable
------------------------------------------------------------------------------*/
static void disable(void)
{
	fputs("Disabling ... ", stderr);
	foreach(disableOp, 0, OP_WRITE);
	fputc('\n', stderr);
}

static int disableOp(int flgDef, struct RegDef * reg, void const* arg)
{
	if (!flgDef) {
		assert(reg->name);

		reg->vflags &= ~REG_ENABLE;
		fprintf(stderr, "%s ... ", reg->name);
		return 1;
	} else {
		fputs("ERROR: The DISABLE command requires register specification.\n",
			  stderr);
		return 0;
	}
}

/*------------------------------------------------------------------------------
  dump
------------------------------------------------------------------------------*/
static void dump(void)
{
	if (lf)
		foreach(dumpOp, 0, OP_READ);
	else
		fprintf(stderr, "ERROR: Must start a log file first.\n");
}

static int dumpOp(int flgDef, struct RegDef * reg, void const* arg)
{
	int status;

	if ((reg->vflags & REG_ENABLE) && reg->reg) {
		fprintf(stderr, "%s reg:%ld\n", 
		    reg->name, regGet(reg->reg));
	}
	return 0;
}

/*------------------------------------------------------------------------------
  enable
------------------------------------------------------------------------------*/
static void enable(void)
{
	fputs("Enabling ... ", stderr);
	foreach(enableOp, 0, OP_WRITE);
	fputc('\n', stderr);
}

static int enableOp(int flgDef, struct RegDef * reg, void const* arg)
{
	if (!flgDef) {
		assert(reg->name);
		reg->vflags |= REG_ENABLE;
		fprintf(stderr, "%s ... ", reg->name);
		return 1;
	} else
		fputs("ERROR: The ENABLE command requires register specification.\n",
			  stderr);
	return 0;
}

/*------------------------------------------------------------------------------
  foreach

  This function parses a register list and applies a function to each
  register that matches the name specification. This function assumes
  that the command buffer has already been partially parsed and that
  strtok() is pointing to the register list.
------------------------------------------------------------------------------*/
static void foreach(ApplyOp op, void * arg, int optype)
{
	if (!lf || OP_READ == optype) {
		char const* regName = strtok(0, sep);

		if (regName)
			do {
				int ii, found = 0;

				/* Loop through all the registers, looking for the one
				   specified by the user. */

				for (ii = 0; ii < N_REG; ++ii)
					if (cmpName(regList[ii].name, regName)) {
						if (!op(0, regList + ii, arg))
							return;
						found = 1;
					}

				/* If we didn't find the register, the user must have
				   typed it in incorrectly. */

				if (!found) {
					fprintf(stderr, "ERROR: Invalid register name -- '%s'.\n",
							regName);
					return;
				}
			} while (0 != (regName = strtok(0, sep)));
		else {
			int ii;

			/* Loop through all the registers and apply the default
			   case. */

			for (ii = 0; ii < N_REG; ++ii)
				if (!op(1, regList + ii, arg))
					return;
		}
	} else
		fprintf(stderr, "ERROR: Cannot change register set while logging.\n");
}

/*------------------------------------------------------------------------------
  list
------------------------------------------------------------------------------*/
static void list(void)
{
	foreach(listOp, 0, OP_READ);
}

static int listOp(int flgDef, struct RegDef * reg, void const* arg)
{
	assert(reg);
	assert(reg->name);


	    fprintf(stderr, "%*s @ %.2fHz ", (int) sizeof(reg->name),
				reg->name, (double) sampleRate / (double) reg->vrate);
	    if (reg->vflags & REG_ENABLE) 
	      fprintf(stderr, "Enabled\n");
	    else
	      fprintf(stderr, "Disable\n");
	
	return 1;
}

/*------------------------------------------------------------------------------
  OpenLog

  Opens and prepares a log file for writing. The format of this log file is
  fixed (even though the underlying libraries support varying formats.)
------------------------------------------------------------------------------*/
int OpenLog(char const* name, char *cfg)
{
  int ii;
  long status;

  if (checkVersion())

	/* If the handle is zero, then we can open a file. */

	if (!lf) {
	  if (0 != (semNotify = semBCreate(SEM_FULL, SEM_Q_FIFO))) {
           
		if (OK == createNewTask()) {
		  if (OK == createNfsConnection()) {
			char path[1024];
			long status;
			TS_STAMP now;
			int hours;
			char tstxt[80];
			double ldj90;
			int ldj;

/*
*  This works since the EPICS timestamps begin on 1/1/1990.
*/
			slaCldj(1990, 1, 1, &ldj90, &stat);
			tsLocalTime(&now);
			printf("%s, ", tsStampToText(&now, TS_TEXT_MONDDYYYY, tstxt));
			ldj = ldj90 + (now.secPastEpoch/86400);

/*
 * Switch to next MJD if past 10 am local time
*/
			hours = (now.secPastEpoch/3600) % 24;
			if (hours >= 10) ldj++;
			printf("SDSS MJD: %d\n", ldj);

			/* If a directory was returned, we have to first create it
			   on the host machine. Then we need to update the file
			   name. */

			  /* Create the directory. We don't check error returns
                 because we may have already created the directory. */

			sprintf(path, "%s%d", thePath, ldj);
		        printf("Creating directory %s\n", path);	
			mkdir(path);

			  /* Form a new filename that includes the directory. */

			sprintf(path, "%s/%s", path, name);
			printf("Creating logfile %s\n", path);

			if (OK == setupLogFile(path, cfg)) {
			  resetStats();
			  if (TimerStart(sampleRate, 3, sampleData)) {
				return 1;
			  }	else {
				printf("TimerStart(%d,%d,%lx) failed\n", sampleRate, 3, sampleData);
				logClose(&lf);
			  }
			} else {
			    printf("setupLogFile(%s,%s) failed\n", path, name);
			} 
			destroyNfsConnection();
		    }  /* if createNFSConnection */
		}  /* if createNewTask */

		semDelete(semNotify);
		semNotify = 0;

	    } 
	} 
      return 0;
}

/*------------------------------------------------------------------------------
  printHeader
------------------------------------------------------------------------------*/
static void printHeader(void)
{
	static char const txt[] =
		"     /\\       _________  __  ___ .      .  \n"
		"    /  \\/\\   /_  __/ _ \\/  |/  /  *  (   *   "
		"Telescope Performance Monitor\n"
		" /`/   /  \\   / / / ___/ /|_/ /  *  .  *     "
		"$Name:  $\n"
		"-------------/_/ /_/  /_/  /_/-------------  "
	        "Peregrine McGehee (peregrin@fnal.gov)\n"
		"Original by: Rich Neswold (neswold@fnal.gov)\n\n";

	fputs(txt, stderr);
}

/*------------------------------------------------------------------------------
  printHelp
------------------------------------------------------------------------------*/
static void printHelp(void)
{
	char* cmd = strtok(0, sep);

	if (!cmd) {
		int ii, badCmd = 0;
		static char const txt[] =
			"Help is available for the following commands:\n\n";

		printHeader();
		fprintf(stderr, txt);

		for (ii = 0; ii < sizeof(cmdInfo) / sizeof(*cmdInfo); ++ii)
			fprintf(stderr, " %8s%c %s\n", cmdInfo[ii].name,
					cmdInfo[ii].func ? ' ' : (badCmd = 1 ,'*'),
					cmdInfo[ii].brief);
		fputc('\n', stderr);

		/* If any command had a NULL function pointer, it hasn't been
           implemented yet. So display a message already! */

		if (badCmd)
			fprintf(stderr, "* These commands haven't been "
					"implemented yet.\n\n");
	} else {
		int ii;

		for (ii = 0; ii < sizeof(cmdInfo) / sizeof(*cmdInfo); ++ii)
			if (!stricmp(cmd, cmdInfo[ii].name)) {
				fprintf(stderr, "SYNTAX: %s\n", cmdInfo[ii].verbose);
				return;
			}
		fprintf(stderr, "There isn't any help for '%s'.\n", cmd);
	}
}

/*------------------------------------------------------------------------------
  printStatus
------------------------------------------------------------------------------*/
static void printStatus(void)
{
	fprintf(stderr, "\nStatistics:\n"
			"\t Sampling Rate : %u Hz\n"
			"\t    Interrupts : %lu occurred\n"
			"\t  Data Written : %lu samples\n"
			"\tVersion Errors : %lu rereads\n",  sampleRate,
			stats[statInterrupts], stats[statDataWritten],
			stats[statRereadVersion]);
}

/*------------------------------------------------------------------------------
  resetStats
------------------------------------------------------------------------------*/
static void resetStats(void)
{
	int ii;

	for (ii = 0; ii < sizeof(stats) / sizeof(*stats); ++ii)
		stats[ii] = 0;
}

/*------------------------------------------------------------------------------
  sample
------------------------------------------------------------------------------*/
static void sample(void)
{
	char const* arg = strtok(0, sep);

	if (arg) {
		if (!lf) {
			int ii;

			if (stricmp(arg, "default")) {
				double newRate = strtod(arg, 0);

				/* Make sure the value is in range. */

				if (newRate >= .01 && newRate <= (double) sampleRate)
					foreach(sampleOp, &newRate, OP_WRITE);
				else
					fprintf(stderr, "ERROR: The rate must be between .01 "
							"and %u.\n", sampleRate);
			} else {
				sampleRate = DEF_SAMPLE_RATE;

				/* Copy the default values of the sample rate into the
				   variable location. */

				for (ii = 0; ii < N_REG; ++ii) {
					regList[ii].vrate = regList[ii].rate;
				}
			}
		} else
			fputs("ERROR: Cannot update rates while logging data.\n", stderr);
	} else
		fputs("ERROR: The 'sample' command needs arguments. "
			  "Try 'help sample'.\n", stderr);
}

static int sampleOp(int flgDef, struct RegDef * reg, void const* arg)
{
	if (!flgDef) {
		assert(reg);
		assert(arg);

		reg->vrate = (short) (sampleRate / *((double const*) arg) + 0.5);
		fprintf(stderr, "%s will be sampled at %.2fHz.\n", reg->name,
				((double) sampleRate / (double) reg->vrate));
		return 1;
	} else {
		unsigned short minRate = 0xffff;
		int ii;

		/* Find the lowest period used by the registers. */

		for (ii = 0; ii < N_REG; ++ii)
			if (regList[ii].rate < minRate)
				minRate = regList[ii].rate;

		/* If the lowest period is 1, then we have a properly formed
           request. If not we have to massage the other registers'
           periods. */

		return 0;
	}
}

/*------------------------------------------------------------------------------
  sampleData

  This function gets called periodically. It signals the secondary
  task to sample and write the data.
------------------------------------------------------------------------------*/
static void sampleData(void)
{
	++stats[statInterrupts];

	/* Signal the awaiting task to sample and write the data. */

	semGive(semNotify);
}

/*------------------------------------------------------------------------------
  startLog
------------------------------------------------------------------------------*/
void startLog(void)
{
  if (!lf) {
	char const* const name = strtok(0, sep);
	char *cfg = strtok(0, sep);

	if (name) {
	  OpenLog(name, cfg);
	} else
	  fprintf(stderr, "ERROR: must provide a file name.\n");
  } else
	fprintf(stderr, "ERROR: Data is already being logged.\n");
}

/*------------------------------------------------------------------------------
  setupLogFile

  This creates the log file and then configures the registers used by it.

  If the file already exists we'll back it up
------------------------------------------------------------------------------*/
static STATUS setupLogFile(char * name, char * cfg)
{
	int i;
	char newname[100];
	FILE *fp;

	fp = fopen(name, "r");

        if (fp) {	
	   fclose(fp);
	   for  (i = 1; i < 100; i++) {
	      sprintf(newname, "%s-%02d", name, i); /* new name for logfile */

	      fp = fopen(newname, "r");
              if (!fp) break; else fclose(fp);
	   }

	   if(i == 100) {
	      epicsPrintf("ERROR: Failed to find unused version number for %s; not restarting logger\n", name);
	      return ERROR;
	   }
	} else {
	    strcpy(newname, name);
	}

	/* If we successfully open the file, we can start to define the
	   registers. */

        if (configDatabase(cfg) == OK && N_REG > 0) {
	    if ((lf = logCreate(newname, sampleRate, N_REG))) {
/* logSetDescription(lf, tstxt);*/

		if (configRegisters(cfg) == OK) {
		        epicsPrintf("Logging to %s\n", name);
			return OK;
		}
		else {
			epicsPrintf("configRegisters %s failed\n", cfg);
			logClose(&lf);
		}
	    } else {
	       epicsPrintf("error creating logfile %s\n", name);
	    }
        } else if (N_REG > 0) { 
	    epicsPrintf("configDatabase %s failed\n", cfg);
	} else {
	    epicsPrintf("N_REG == 0\n");
	}

	return ERROR;
}

/*------------------------------------------------------------------------------
  stopLog
------------------------------------------------------------------------------*/
static void stopLog(void)
{
	if (lf)
		CloseLog();
	else
		fprintf(stderr, "ERROR: Data is not currently being logged.\n");
}

/*----------------------------------------------------------------------------
------------------------------------------------------------------------------*/
static int stricmp(char const* a, char const* b)
{
	assert(a);
	assert(b);
	for (; *a && *b && tolower(*a) == tolower(*b); ++a, ++b)
		;
	return *a - *b;
}

/*------------------------------------------------------------------------------
  taskMain
------------------------------------------------------------------------------*/
static void taskMain()
{
        float buf = 0.0;
        u_int32_t tmp;
	long status;
        int ii;
        char pvname[40];
	int options=0;
        int nRequest=1;

        assert(semNotify);
   

	while (OK == semTake(semNotify, WAIT_FOREVER)) {
		int ii;

		/* Loop through each register and call the respective sample
		   routine. When this loop terminates, all the registers
		   should have sampled their associated data. */

		for (ii = 0; ii < N_REG; ++ii) {
			struct RegDef * ptr = regList + ii;

		 	if ((ptr->vflags & REG_ENABLE) && ptr->reg) {
          
			  status = dbGetField(&ptr->addr, DBR_FLOAT,&buf,&options,&nRequest,0);	
			  if (status != 0) {
			    epicsPrintf("%s:%x: error in dbGetField call\n", ptr->name, &ptr->addr);
			    continue;
			  }

			  /* Kludge for MIGs */
			  if (ptr->addr.field_type == DBR_FLOAT ||
			  	ptr->addr.field_type == DBR_DOUBLE) {
			    tmp = buf * 10000;
			  } else { 
			    tmp = buf * 1;
			  }
                          regPut(ptr->reg, *(u_int32_t *)&tmp);
			
			}
		}
		logSaveData(lf);
		++stats[statDataWritten];
	}
}

/*------------------------------------------------------------------------------
  tpm
------------------------------------------------------------------------------*/
void startTpm(void)
{
   int ii;

   if (lf != NULL) {
      return;
   }

   /* Make sure we start with all registers enabled. */
   
   for (ii = 0; ii < N_REG; ++ii) {      
      regList[ii].vflags = regList[ii].flags;
      regList[ii].vrate = regList[ii].rate;
   }
}

void stopTpm(void)
{
   if (lf) {
      stopLog();
   }
}

void tpm(void)
{
	int hLed = ledOpen(fileno(stdin), fileno(stderr), 100);

	void shutdownTpm(char const* str)
		{
			stopTpm();
			tyEOFSet(0x04);
			ledClose(hLed);
			fprintf(stderr, str);
		}


	/* If we can't open the line editor, we're screwed. */

	if (hLed == ERROR) {
		return;
	}

	tyEOFSet(0x7e);
	printHeader();
	fprintf(stderr, "Type \"help\" for help.\n\n");
	fprintf(stderr, "Using version %d of shared data located at %p\n",
			SDSS_FRAME_VERSION, data);

	/* Infinite command loop. */

	for (;;) {
		static char const prompt[] = "TPM>";
		char buffer[80];
		char const* cmd;
		int ii;

		(void) checkVersion();
		fprintf(stderr, prompt);
		if (EOF == ledRead(hLed, buffer, sizeof(buffer))) {
			shutdownTpm("ERROR: EOF on input.\n");
			return;
		}

		/* If there aren't any tokens, just start over. */

		if (0 == (cmd = strtok(buffer, sep)))
			continue;

		/* We got a command. Vector off and perform it. */

		for (ii = 0; ii < sizeof(cmdInfo) / sizeof(*cmdInfo); ++ii)
			if (!stricmp(cmd, cmdInfo[ii].name)) {

				/* If the function pointer points to the quit
                   function, we have to leave. */

				if (cmdInfo[ii].func == NULL) {
/*
					shutdownTpm("\nExitting...\n");
*/
					return;
				} else if (cmdInfo[ii].func)
					cmdInfo[ii].func();
				else
					fprintf(stderr, "Not implemented yet. Sorry!\n");
				ii = -1;
				break;
			}

		if (-1 != ii)
			fprintf(stderr, "'%s' is an unknown command. "
					"Use \'help\' for help.\n", cmd);
	}
}

/*****************************************************************************/
/*
 * Define the TMP's commands to cmd_handler()
 */
#include "cmd.h"

void
tpmCmdsInit(void)
{
   int i;

   for(i = 0; i < sizeof(cmdInfo)/sizeof(*cmdInfo); i++) {
      if(strcmp(cmdInfo[i].name, "quit") == 0) {
	 continue;			
      }

      define_cmd(cmdInfo[i].name,  (char *(*)(char *))cmdInfo[i].func, 0, 0,0);
   }
   define_cmd("tpm",  (char *(*)(char *))startTpm, 0, 0,0);
   define_cmd("quit", (char *(*)(char *))stopTpm,  0, 0,0);
}
