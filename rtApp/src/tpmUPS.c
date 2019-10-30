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

/*
 * Example response string from "Q1[13]" query.
 *
 * (123.0 123.0 119.0 029 59.9 0110 25.0 00000000[13]
 */

long tpmUPSInit(struct genSubRecord *psub)
{
    return OK;
}

long tpmUPSProcess(struct genSubRecord *psub)
{
    long status;
    char *stat;    
    double *pinput, *pfault, *poutput;
    double *pload, *pfreq, *pbattery, *ptemp;
    long *pb7, *pb6, *pb5, *pb4, *pb3, *pb2, *pb1, *pb0;

    stat = genSubStructInput(psub, 120, &psub->a, &status);

    /* Input voltage */
    pinput = genSubDoubleOutput(psub, &psub->vala, &status);
    /* Input fault voltage */
    pfault = genSubDoubleOutput(psub, &psub->valb, &status);
    /* Output voltage */
    poutput = genSubDoubleOutput(psub, &psub->valc, &status);
    /* Output load */
    pload = genSubDoubleOutput(psub, &psub->vald, &status);
    /* Input frequency */
    pfreq = genSubDoubleOutput(psub, &psub->vale, &status);
    /* Battery voltage */
    pbattery = genSubDoubleOutput(psub, &psub->valf, &status);
    /* Temperature */
    ptemp = genSubDoubleOutput(psub, &psub->valg, &status);
    /* Status Bits: 1=utility failure */
    pb7 = genSubLongOutput(psub, &psub->valh, &status); 
    /* Status Bits: 1=battery low */
    pb6 = genSubLongOutput(psub, &psub->vali, &status);
    /* Status Bits: 1=bypass active */
    pb5 = genSubLongOutput(psub, &psub->valj, &status); 
    /* Status Bits: 1=ups failed */
    pb4 = genSubLongOutput(psub, &psub->valk, &status);
    /* Status Bits: 1=offline */
    pb3 = genSubLongOutput(psub, &psub->vall, &status); 
    /* Status Bits: 1=test */
    pb2 = genSubLongOutput(psub, &psub->valm, &status);
    /* Status Bits: 1=shutdown */
    pb1 = genSubLongOutput(psub, &psub->valn, &status); 
    /* Status Bits: reserved (always 0) */
    pb0 = genSubLongOutput(psub, &psub->valo, &status);


    sscanf(stat, 
    "(%lf %lf %lf %lf %lf %lf %lf %1ld%1ld%1ld%1ld%1ld%1ld%1ld%1ld",
	   pinput, pfault, poutput, pload, pfreq, pbattery, ptemp,
	   pb7, pb6, pb5, pb4, pb3, pb2, pb1, pb0);

    return 0;
}
