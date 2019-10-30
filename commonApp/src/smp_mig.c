/*=============================================================================
smp_mig.c - MIG access functions for TPM logger
=============================================================================*/
#include <vxWorks.h>
#include <stdio.h>
#include "errlogLib.h"
#include "logfile.h"
#include "migtask.h"

/*
 * Common access routine for mig data.
*/
int smp_mig(unsigned nmig, unsigned nchan)
{
    HMIG mig;
    if ((mig = migHandleGet(nmig))) 
	return (int) (migGetData(mig, nchan) * 10000.0);
    else
	return 0;
}
