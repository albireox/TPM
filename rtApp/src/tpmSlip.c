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
#include <semLib.h>
#include "ip470.h"

#define SLIP_NCHAN 8
#define SLIP_MEDFIL 5

extern long ip470Write(struct cblk470 *, int, int, int);
extern struct cblk470 * IP470_B;
extern long rprt470(struct cblk470 *, byte);

int SLIP_MAXLOOP = 50;
int SLIP_DEBUG = 0;
int SLIP_TIMING = 0;
int SLIP_DOMF = 0;

static int *values[SLIP_NCHAN];
static int *counts[SLIP_NCHAN];

struct tpmSlipBlock {
    int nchan;
    int nsamples[SLIP_NCHAN];
    int index[SLIP_NCHAN];
    int chan[SLIP_NCHAN][SLIP_MEDFIL]; /* Preserves history! */
    int chan_sort[SLIP_MEDFIL];
};

static int intcompare(const void *pd1, const void *pd2) {
    int d1 = *(int *)pd1;
    int d2 = *(int *)pd2;

    return (d1 < d2 ? -1 : (d1 > d2 ? 1 : 0));
}

long tpmSlipInit(struct genSubRecord *psub)
{
    int i;
    int status;
    struct tpmSlipBlock * pblock;	

    pblock = (struct tpmSlipBlock *)malloc(sizeof(struct tpmSlipBlock));
    if (!pblock) {
	printf("Unable to allocate tpmSlipBlock structure!\n");
	return ERROR;
    }
   
    psub->dpvt = (void *)pblock; 

    for (i = 0; i < SLIP_NCHAN; i++) {
        values[i] = genSubLongOutput(psub, &psub->vala + i, &status);
	*values[i] = 0;
        counts[i] = genSubLongOutput(psub, &psub->vala + SLIP_NCHAN + i, &status);
	*counts[i] = 0;
	pblock->nsamples[i] = pblock->index[i] = 0;
    }
    pblock->nchan = 0;

    return OK;
}

int read_slip_mux(int channel, int * val)
{
    int lowbyte=1, highbyte=0;
    int j;
    int status;

    if (SLIP_DEBUG) printf("\n%d: ", channel);

/*
 * Ask for the channel
 */
    ip470Write(IP470_B, 4, 0x0f, channel);

/*
 * Wait for the RDY bit to go high
 */
    for (j = 0; j < SLIP_MAXLOOP && (lowbyte & 0x01 || SLIP_TIMING); j++) {
        lowbyte = rprt470(IP470_B, 3);
    }
    highbyte = rprt470(IP470_B, 2);

/* Reset */
    ip470Write(IP470_B, 4, 0x0f, 0x0f);

    *counts[channel] = j;
    if (SLIP_DEBUG && SLIP_DEBUG < 2) printf("%1d ",j);

    status = 1; /* initial pessimism */

    if (j < SLIP_MAXLOOP) {
	if (SLIP_DEBUG) printf("%02x %02X", highbyte, lowbyte);
	if (highbyte != 0) {
           *val = lowbyte >> 1;
           *val += highbyte << 7;
           *val = (0x7fff & ~(*val));
	   status = 0;
        }
    }

    return status;
}

long tpmSlipProcess(struct genSubRecord *psub)
{
    int i, j;
    int status;
    int val;
    struct tpmSlipBlock * pblock = (struct tpmSlipBlock *)psub->dpvt;
    
    i = pblock->nchan;

    status = read_slip_mux(i, &val); 

    if (status == 0) { 
        if (SLIP_DOMF == 1) {
	    pblock->chan[i][pblock->index[i]] = val;
	    pblock->index[i] = (pblock->index[i] + 1) % SLIP_MEDFIL;
	
            pblock->nsamples[i] = (pblock->nsamples[i] < SLIP_MEDFIL ? 
		pblock->nsamples[i] + 1 : SLIP_MEDFIL);

            memcpy(pblock->chan_sort, pblock->chan[i], sizeof(pblock->chan[i]));
            qsort(pblock->chan_sort, pblock->nsamples[i], sizeof(int), intcompare);
	    if (SLIP_DEBUG > 4) {
	        for (j = 0; j < pblock->nsamples[i]; j++)
		        printf("%d ", pblock->chan_sort[j]);
	        printf("\n");
	    }
            *values[i] = pblock->chan_sort[pblock->nsamples[i]/2];
	} else
	    *values[i] = val;

    } else
	if (SLIP_DEBUG) printf(" failed\n");

    pblock->nchan = (pblock->nchan + 1) % SLIP_NCHAN;

    return 0;
}
