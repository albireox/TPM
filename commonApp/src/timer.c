/*============================================================================

  T I M E R . C

  This module contains routines that provide a high resolution
  timer.

  $Id: timer.c,v 1.3 2000/12/21 22:35:47 peregrin Exp $
============================================================================*/
#include <vxWorks.h>
#include <iv.h>
#include <intLib.h>
#include <stdio.h>
#include <assert.h>
#include "mv162.h"
#include "timer.h"
#include "errMdef.h"

#define	MCC(off)		(*(unsigned long*)(MCC_BASE_ADRS + (off)))

/* Local prototypes... */

static void dumpStats(void);
static void initStats(void);
static void intRoutine();

/* Local Data... */

static void (*intFunc)() = 0;
static struct {
	unsigned long totalInterrupts;
	unsigned long missedInterrupts;
} stat;

/*----------------------------------------------------------------------------
  dumpStats
----------------------------------------------------------------------------*/
static void dumpStats(void)
{
	epicsPrintf("*** Timer Interrupt Statistics ***\n");
	epicsPrintf(" Total interrupts : %10ld\n", stat.totalInterrupts);
	epicsPrintf("Missed interrupts : %10ld\n", stat.missedInterrupts);
}

/*----------------------------------------------------------------------------
  initStats
----------------------------------------------------------------------------*/
static void initStats(void)
{
	stat.totalInterrupts = stat.missedInterrupts = 0;
}

/*----------------------------------------------------------------------------
  intRoutine

  This function is activated by the timer interrupt. It calls the user-
  supplied function and then resets the interrupt status.
----------------------------------------------------------------------------*/
static void intRoutine()
{
	stat.totalInterrupts++;

	if (intFunc)
		(*intFunc)();

	stat.missedInterrupts += ((MCC(0x1c) >> 12) & 0x0f) - 1;

	/* Clear the overflow counter and clear the interrupt pending
	   bit. */

	MCC(0x1c) |= 0x00000400L;
	MCC(0x18) |= 0x08000000L;
}

/*----------------------------------------------------------------------------
  TimerStart
----------------------------------------------------------------------------*/
int TimerStart(unsigned long freq, unsigned char level, void (*func)())
{
	/* Make sure there's an MCC present. */

	if ((MCC(0x00) & 0xff000000L) == 0x84000000L) {
		unsigned char intBase;
		int lockKey;

		/* Now validate the parameters. */

		assert(freq > 0 && freq <= 100000L);
		assert(level < 8);		

		initStats();
		if ((intBase = MCC(0x0) & 0xff) & 0x0f)
			MCC(0x0) = (MCC(0x0) & ~0xff) | (intBase = 64);

		/* Attach our function to the interrupt vector. */
			
		if (OK == intConnect(INUM_TO_IVEC(intBase + 3), intRoutine, 0)) {
			intFunc = func;

			lockKey = intLock();

			/* Compute the value needed in the compare register. */

			MCC(0x38) = (1000000L + (freq >> 1)) / freq;
			MCC(0x3c) = 0;

			/* Set-up the control register so the counter increments
			   and reloads when it reaches the compare register's
			   value. */

			MCC(0x1c) |= 0x00000700L;

			/* Now enable interrupts. */

			MCC(0x18) |= (level | 0x18) << 24;
			intUnlock(lockKey);
			return 1;
		} else
			epicsPrintf("TIMER: Couldn't connect to interrupt vector.");
	} else
		epicsPrintf("TIMER: MCC isn't present.");
	return 0;
}

/*----------------------------------------------------------------------------
  TimerStop
----------------------------------------------------------------------------*/
void TimerStop()
{
	/* Make sure there is an MCC present. */

	if ((MCC(0x00) & 0xff000000L) == 0x84000000L) {

		/* Shut off the interrupt enable bit for timer 4. Also, make
		   sure we don't change the state of the INT bits for the
		   other timers. */

		MCC(0x18) &= ~0x10202020L;

		/* Clear out the interrupt function. */
		
		intFunc = 0;

		/* Dump statistics. */

		/*dumpStats();*/
	}
}
