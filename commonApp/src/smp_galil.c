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
{
    HGALIL galil;
    int h;
    if ((galil = galilHandleGet(ngalil))) 
	switch (t) {
	case 0:	return (int)galilGetActual(galil, nchan);
		break;
	case 1:	return (int)galilGetCommanded(galil, nchan);
		break;
	case 2:	h = galilGetHomed(galil, nchan);
		return h;
		break;
	default:	return 0;
		break;
	}
    else
	return 0;
}
