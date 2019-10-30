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
#include "timer.h"
#include "ip470.h"
#include "drvGsIp16ADC.h"

extern long ip470Write(struct cblk470 *, int, int, int);
extern void * PMSS;
extern struct cblk470 * IP470_B;

SEM_ID pmssSem;

/* 
WIND CHANS:
	0 wind speed 
	1 wind direction
	2 imager LN2 pressure
	3 spectro LN2 pressure
*/
#define WIND_NCHAN 4 
#define WIND_MEDFIL 49 

struct PMSS_info {
    int cur_chan;
    int nsamples[WIND_NCHAN];
    int index[WIND_NCHAN];
    int chan[WIND_NCHAN][WIND_MEDFIL];	/* preserves history! */
    int chan_sort[WIND_MEDFIL];
};

#define PMSS_NCHAN 16

static int intcompare(const void *pd1, const void *pd2) {
    int d1 = *(int *)pd1;
    int d2 = *(int *)pd2;

    return (d1 < d2 ? -1 : (d1 > d2 ? 1 : 0));
}

void pmssSync()
{
    semGive(pmssSem);
}

int tpmPMSSLoop = 9;

long tpmPMSSInit(struct genSubRecord *psub)
{
    struct PMSS_info *pinfo;
    long status;
    double settle_ms;
    unsigned long settle_freq;
    int i;

    pinfo = (struct PMSS_info *)malloc(sizeof(struct PMSS_info));
    pinfo->cur_chan = -1;

    pmssSem = semBCreate(SEM_Q_FIFO, SEM_EMPTY);

    settle_ms = genSubDoubleInput(psub, &psub->r, &status);
    settle_freq = settle_ms > 0 ? 1000.0/settle_ms : 1000;

    for (i = 0; i < WIND_NCHAN; i++) {
	pinfo->nsamples[i] = pinfo->index[i] = 0;
    }
    psub->dpvt = pinfo;

    TimerStart(settle_freq, 3, pmssSync);

    return OK;
}

int tpmReadADCMean(void * dev, int chan, int nloop, int delay)
{
    int i;
    long status;
    Word val;
    int sval = 0;

    if (nloop > 0) {
	for (i = 0; i < nloop; i++) {
	    status = IP16ADC_get_channel(dev, chan, &val, delay);
	    sval += val;
	}
	sval /= nloop;
    }

    return sval;
}
	
long tpmPMSSProcess(struct genSubRecord *psub)
{
    double scale[PMSS_NCHAN];
    double *values[PMSS_NCHAN];
    double *wvalues[WIND_NCHAN];
    long mask;
    long *idx;
    long status;
    int i;
    int zone, param, cmd, cmdtmp;
    struct PMSS_info *pinfo = (struct PMSS_info *)psub->dpvt;
    Word val;
    double wind_max[WIND_NCHAN] = {110.72834, 360.0, 30.0, 30.0};
    double wind_offset[WIND_NCHAN] = {1.11844681, 0.0, 0.0, 0.0};
    int wind_medfil[WIND_NCHAN] = {WIND_MEDFIL, WIND_MEDFIL, 1, 1};

    mask = genSubLongInput(psub, &psub->q, &status);

    if (mask & 0xffff) {

        idx = genSubLongOutput(psub, &psub->valq, &status);

        for (i = 0; i < PMSS_NCHAN; i++) {
            scale[i] = genSubDoubleInput(psub, &psub->a + i, &status);
            values[i] = genSubDoubleOutput(psub, &psub->vala + i, &status);
        }

        i = 0;
        do {
            pinfo->cur_chan = (pinfo->cur_chan + 1) % PMSS_NCHAN;
	    i++;
        } while (!(mask & (1<<pinfo->cur_chan)) && i <= PMSS_NCHAN);

        zone = (pinfo->cur_chan & 0xc) >> 2;
        zone = ~zone & 0x3; 
/*        zone = zone & 0x3; */
        param = pinfo->cur_chan & 0x3;
/*    cmd = (param << 2) + zone; */
        cmdtmp =  ~((zone << 2) + param);

/* Bit order reversed in the wiring?? */
        cmd = (cmdtmp & 8) ? 1 : 0;
        cmd += (cmdtmp & 4) ? 2 : 0;
        cmd += (cmdtmp & 2) ? 4 : 0;
        cmd += (cmdtmp & 1) ? 8 : 0;

/*
 * Wait to synchronize with timer.
*/	
        status = semTake(pmssSem, 20);
        ip470Write(IP470_B, 4, 0xf0, cmd<<4);

	if (tpmPMSSLoop < 1) tpmPMSSLoop = 1;
        status = semTake(pmssSem, 20);
	val = tpmReadADCMean(PMSS, 0, tpmPMSSLoop, 0);

        *values[pinfo->cur_chan] = (val - 32768)*(10.0/32768.0)*scale[pinfo->cur_chan];
/* 
 * Correct for differential driver gain 
 * JEG designed at unity gain rather than half gain.
*/
        *values[pinfo->cur_chan] *= 0.5; 
        *idx = pinfo->cur_chan;
    }

/*
 * Obtain the wind speed and direction.
*/
    for (i = 0; i < WIND_NCHAN; i++) {
        wvalues[i] = genSubDoubleOutput(psub, &psub->valr + i, &status);

/*
 * Delay of -1 ticks means don't perform a new conversion,
 * just obtain the existing values.
 *
 * But force a conversion if there is no work to be done
 * on the PMSS interface.
*/
        status = IP16ADC_get_channel(PMSS, i+1, &val, 
	 (mask & 0xffff || i != 0 ? -1 : 0));
        pinfo->chan[i][pinfo->index[i]] = val;
        pinfo->index[i] = (pinfo->index[i] + 1) % wind_medfil[i];
        pinfo->nsamples[i] = (pinfo->nsamples[i] < wind_medfil[i] ?
	    pinfo->nsamples[i] +1 : wind_medfil[i]);

        memcpy(pinfo->chan_sort, pinfo->chan[i], sizeof(pinfo->chan[i]));
        qsort(pinfo->chan_sort, pinfo->nsamples[i], sizeof(int), intcompare);
        *wvalues[i] = (pinfo->chan_sort[pinfo->nsamples[i]/2] - 32768)*
			(wind_max[i]/32768) + wind_offset[i];
    }
    return 0;
}
