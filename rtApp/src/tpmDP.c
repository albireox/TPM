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

int DPDBG=0;

long tpmDPInit (struct genSubRecord *psub)
{
    return OK;
}

int dpchecksum(char * p)
{
    int cksum = 0;

    while (*p && (*p != 'P' || *p != 'F')) {
	cksum += *(p++);
    }
    cksum &= 0xFF;

    return cksum;
}

long tpmDPProcess (struct genSubRecord *psub)
{
    long status;
    int cksum, cnt;

/* 
* estring needs to be long enough to accomodate all errors at once,
	even though that is very unlikely.  That string is:

	"Fan Failure 0C Calibration 50C Calibration Overheat Self-Maintenance Mirror Servo Error Critical Voltages Dirty Mirror"
which is 120 characters, including the NULL.
*
*/

    char *t1, *t3;
    char dp_status, eflags[8], *estring, *p;
    char tsgn, dsgn;
    char tstr[6], dstr[6];
    double at, dpt;
    double *pat, *pdpt;

/* Parse T1 response on INPA */
    t1 = genSubStringInput(psub, &psub->a, &status);

/* Scan the input and check for validity */
    strncpy(tstr, t1+3, 5);
    strncpy(dstr, t1+11, 5);
    tstr[5] = 0;
    dstr[5] = 0;
    tsgn = t1[2];
    dsgn = t1[10];
    dp_status = t1[17];
    sscanf(t1+18, "%x", &cksum);

/* Parse the temperature and DP readings */
   at = 0.0;
   dpt = 0.0;

   if (tstr[0] >= '0' && tstr[0] <= '9') at += (tstr[0] - '0')*100.0;
   if (tstr[1] >= '0' && tstr[1] <= '9') at += (tstr[1] - '0')*10.0;
   if (tstr[2] >= '0' && tstr[2] <= '9') at += (tstr[2] - '0')*1.0;
   if (tstr[4] >= '0' && tstr[4] <= '9') at += (tstr[4] - '0')*0.1;
   if (tsgn == '-') at *= -1;

   if (dstr[0] >= '0' && dstr[0] <= '9') dpt += (dstr[0] - '0')*100.0;
   if (dstr[1] >= '0' && dstr[1] <= '9') dpt += (dstr[1] - '0')*10.0;
   if (dstr[2] >= '0' && dstr[2] <= '9') dpt += (dstr[2] - '0')*1.0;
   if (dstr[4] >= '0' && dstr[4] <= '9') dpt += (dstr[4] - '0')*0.1;
   if (dsgn == '-') dpt *= -1;

   if(DPDBG == 1) {
	printf("%c..%s..%f\t%c..%s..%f\n",
	tsgn, tstr, at,
	dsgn, dstr, dpt);
   }
/* Calculate and check the checksum */
/*
    if ((cnt=dpchecksum(t1)) != cksum) {
	printf("T1 checksum failed %d %d\n",cnt, cksum);
	return ERROR;
    }
*/

/* Get pointers to ambient temp (VALA) and dew point temp (VALB) */
    pat = genSubDoubleOutput (psub, &psub->vala, &status);
    pdpt = genSubDoubleOutput (psub, &psub->valb, &status);

/* If all is well, return new values */
    if (dp_status == 'P') {
/* Convert from degF to degC! */
	at = (at-32.0)/1.8;
	dpt = (dpt-32.0)/1.8;
	*pat = at;
	*pdpt = dpt;
	return 0;
    }

/* Failed self check, so we need to parse T3 on INPB */
    t3 = genSubStringInput (psub, &psub->b, &status);

/* Scan the input and check for validity */
    if (sscanf(t3+1, "T/D %8c%2x", &eflags, &cksum) != 2) {
	printf("T3 parse failed of %s\n", t3+1);
	return ERROR;
    }

/* Calculate and check the checksum */
/*
    if ((cnt=dpchecksum(t3)) != cksum) {
	printf("T3 checksum failed %d %d\n",cnt, cksum);
	return ERROR;
    }
*/

/* Get pointer to error string (VALC) */
    estring = genSubStructOutput (psub, 120, &psub->valc, &status);

/* Clear error string */
    *estring = NULL;

/* Check for each error and concatenate message */
    if (eflags[0] == '1') (void) strcat (estring, "Fan Failure");
    if (eflags[1] == '1') (void) strcat (estring, " 0C Calibration");
    if (eflags[2] == '1') (void) strcat (estring, " 50C Calibration");
    if (eflags[3] == '1') (void) strcat (estring, " Overheat");
    if (eflags[4] == '1') {	/* Self-Maintenance - no data */
	(void) strcat (estring, " Self-Maintenance");
    }
    if (eflags[5] == '1') (void) strcat (estring, " Mirror Servo Error");
    if (eflags[6] == '1') (void) strcat (estring, " Critical Voltages");
    if (eflags[7] == '1') (void) strcat (estring, " Dirty Mirror");

    return 0;
}
