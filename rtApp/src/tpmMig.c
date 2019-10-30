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

#define MAXMIG 6

long tpmMigInit(struct genSubRecord *psub)
{
    return OK;
}

long tpmMigProcess(struct genSubRecord *psub)
{
    long status;
    int i;
    char *blk;
    double c[MAXMIG];
    double *cptr[MAXMIG];

    blk = genSubStructInput(psub, 120, &psub->a, &status);

    for (i = 0; i < MAXMIG; i++) {
        cptr[i] = genSubDoubleOutput(psub, &psub->vala + i, &status);
	c[i] = 0.0;
    }

    sscanf(blk, 
	"1:%lf 2:%lf 3:%lf 4:%lf 5:%lf 6:%lf",
	&c[0], &c[1], &c[2], &c[3], &c[4], &c[5]);

    for (i = 0; i < MAXMIG; i++) *cptr[i] = c[i];
	
    return 0;
}
