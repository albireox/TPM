#include <vxWorks.h>
#include <types.h>
#include <stdioLib.h>
#include <stdlib.h>
#include <string.h>
#include <dbDefs.h>
#include <subRecord.h>
#include <dbCommon.h>
#include <recSup.h>
#include <tsDefs.h>
#include <drvTS.h>

long tpmTimeTestInit(struct subRecord *psub)
{
    return (0);
}

long tpmMJDInit(struct subRecord *psub)
{
    return (0);
}

long tpmTimeTestProcess(struct subRecord *psub)
{
    TS_STAMP now;
    struct timespec ts;
    long diffs, diffns;
    double d;

    tsLocalTime(&now);
    TSgetUnixTime(&ts);
    diffns = now.nsec - ts.tv_nsec;
    diffs = now.secPastEpoch - ts.tv_sec + TS_1900_TO_EPICS_EPOCH;
    d = diffs + diffns/1.0e9;

    psub->val = d;
    return 0;
}
	

long tpmMJD(float hour_offset, int debug)
{
    TS_STAMP now;
    int mon, day, year, hours;
    char tstxt[80];
    double ldj;
    int stat;

    tsLocalTime(&now);
    now.secPastEpoch += 3600*hour_offset;
    tsStampToText(&now, TS_TEXT_SDSS, tstxt);
    sscanf(tstxt, "%d-%d-%d %d",  &year, &mon, &day, &hours);
    slaCldj(year, mon, day, &ldj, &stat);

/*
 * Switch to next MJD if past 17:00 UT
*/
    if (hours >= 17) ldj += 1.0;

    if (debug) {
       printf("%s, hours: %d MJD: %f\n", 
		tstxt, hours, ldj);
    }
    return (long)ldj;
}


long tpmMJDProcess(struct subRecord *psub)
{
    psub->val = tpmMJD(0.0, 0);

    return 0;
}

void tpmMJDTest()
{
    float offset;
    long mjd;

    for (offset = 0.0; offset <= 24.0; offset += 1.0) 
	mjd = tpmMJD(offset, 1);
}
