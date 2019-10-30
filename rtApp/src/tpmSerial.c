#include <vxWorks.h>
#include <types.h>
#include <stdioLib.h>
#include <stdlib.h>
#include <string.h>
#include <dbDefs.h>
#include <genSubRecord.h>
#include <genSubUtil.h>
#include <dbCommon.h>
#include <recSup.h>
#include <tsDefs.h>

#define NCHANS 8 
#define MAX_NSAMPLES 90 
/* Maximum number of samples. */

/* only filter communications alarms, value alarms are passed
 through directly */
int tpmSerialHistory[NCHANS][MAX_NSAMPLES];
int tpmSerialSamples[NCHANS] = {
	90, /* Ndewar_spectro: 15 min */
	90, /* Sdewar_imager: 15 min */
	12, /* MIG1: 2 min */
	12, /* MIG2: 2 min */
	12, /* GAL1: 2 min */
	12, /* GAL2: 2 min */
	12, /* TMICRO: 2 min */
	12  /* UPS: 2 min */
};

static char alarmSevr[5][10] = {"NO_ALARM", "MINOR", "MAJOR",
			"INVALID", "BOGUS"};

int tpmSerialDebug = 0;

int sevrToN(char * serv)
{
    int i;
    for (i = 0; i < 4; i++) {
	if (!strcmp(serv, alarmSevr[i])) return i;
    } 
    return 4;
}

char * nToSevr(int n)
{
    if (n >= 0 && n <= 3) return alarmSevr[n];
    else return alarmSevr[4];
}

LOCAL int tpmFilterAlarm(int nsevr, int nchan)  
{
    int i;
    int ncomsevr = nsevr;

/* shift and repopulate, finding the minimum severity along the way */
    for (i = tpmSerialSamples[nchan] - 1; i > 0; i--) {
        tpmSerialHistory[nchan][i] = tpmSerialHistory[nchan][i-1];
	if (tpmSerialHistory[nchan][i] < ncomsevr) 
	    ncomsevr = tpmSerialHistory[nchan][i];
    }
    tpmSerialHistory[nchan][0] = nsevr;

    return (ncomsevr);
}


long tpmSerialInit (struct genSubRecord *psub)
{
    int i, j;

    for (i = 0; i < NCHANS; i++)
        for (j = 0; j < MAX_NSAMPLES; j++)
	  tpmSerialHistory[i][j] = 0;
   
    return OK;
}

  
void tpmDSH()
{
    int i, j;

    for (i = 0; i < NCHANS; i++) {
        printf("%d: ", i);
        for (j = 0; j < tpmSerialSamples[i]; j++)
	   printf("%d", tpmSerialHistory[i][j]);
        printf("\n");
    }
}

  
long tpmSerialProcess (struct genSubRecord *psub)
{
    long status;
    char *sevr, *newcomsevr, *newvalsevr, *stat;
    int i;
    int nsevr, ncomsevr, nvalsevr;
    static long iloop = 0L;

    iloop++;

    for (i = 0; i < NCHANS; i++) {
        sevr = genSubStringInput(psub, &psub->a + i, &status);
        stat = genSubStringInput(psub, &psub->a + i + NCHANS, &status);

	nsevr = sevrToN(sevr);
        nvalsevr = 0;
	ncomsevr = 0;

/* communications alarm (or lack thereof) 
   if there is a READ alarm then we use the severity (2 = MAJOR)
   otherwise we add a (0 = NO_ALARM) to the history.
*/
        ncomsevr = tpmFilterAlarm((!strncmp(stat, "READ", 4) ? sevr : 0), i);
        if (tpmSerialDebug) 
	printf("%06d %d: %d %d %s.\n", iloop, i, nsevr, ncomsevr, stat);

/* value alarm (or lack thereof) 
   if it isn't a READ alarm then we care 
*/
        if (strncmp(stat, "READ", 4)) {
            nvalsevr = nsevr;
        }    

/* Update comm alarm status */
        newcomsevr = genSubStringOutput(psub, &psub->vala + i, &status);
	newcomsevr[0] = 0;
        strcpy(newcomsevr, nToSevr(ncomsevr));

/* Update val alarm status */
	newvalsevr = genSubStringOutput(psub, &psub->vala + i + NCHANS, &status);
	newvalsevr[0] = 0;
        strcpy(newvalsevr, nToSevr(nvalsevr));
        if (tpmSerialDebug && ncomsevr) 
	    printf("%06d %d: com:%10s val:%10s\n", iloop, i, newcomsevr, newvalsevr);
    }

    return 0;
}
