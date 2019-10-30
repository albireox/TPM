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

#define PATCH_VALUE -50.0
#define BAD_VALUE -200.0

long tpmMeanInit(struct genSubRecord *psub)
{
    psub->udf = FALSE;
    return OK;
}

long tpmMeanProcess(struct genSubRecord *psub)
{
    int retval;
    int nf;
    int i;
    int npatch = 0;	/* BAD < val <= PATCH */ 
    int nbad = 0;	/* val <= BAD */
    int ngood = 0;
    double tval;
    double sum = 0.0;
    double *psum;
    int *pgood;
    int *ppatch;
    int *pbad;
    long status;
	
    retval = 0; /* OK */

    nf = genSubLongInput(psub, &psub->u, &status);

    if (nf >= 0 && nf <= 20) {
	for (i=0; i < nf; i++) {
            tval = genSubDoubleInput(psub, &psub->a + i, &status);
	    if (tval <= BAD_VALUE) {
		nbad++;
	    } else if (tval <= PATCH_VALUE) {
		npatch++;
	    } else {
		sum += tval;
		ngood++;
	    }
	}

	if (ngood > 0) sum /= ngood;
	

        psum = genSubDoubleOutput(psub, &psub->vala, &status);
        pgood = genSubLongOutput(psub, &psub->valb, &status);
        ppatch= genSubLongOutput(psub, &psub->valc, &status);
        pbad = genSubLongOutput(psub, &psub->vald, &status);

	*psum = sum;
	*pgood = ngood;
	*ppatch = npatch;
	*pbad = nbad;
    } else {
	retval = -1;
    }

    return retval;
}
