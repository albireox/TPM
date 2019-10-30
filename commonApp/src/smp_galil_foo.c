/*=============================================================================
smp_galil.c - GALIL access functions for TPM logger
=============================================================================*/
#include <vxWorks.h>
#include <stdio.h>
#include "errlogLib.h"
#include "logfile.h"
#include "galiltask.h"

/*
 * Common access routine for galil data.
 * t:	0 Actual
 *	1 Commanded
 *	2 Homed?
*/
int smp_galil(unsigned ngalil, unsigned nchan, unsigned t)
{ return 0; }
