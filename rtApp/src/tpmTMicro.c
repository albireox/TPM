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

#define BAD_SENSOR_VALUE -200.0

#define TMBLOCK_MAXSIZE 30 
#define TMBLOCK_NCHAN 8
/*#define DBG 1*/

struct tpmTMDummyMap {
    int module;
    int block;
    int startchan;
    int endchan;
};

struct tpmTMDummyMap tpmTMDummy[] = {
    { 0, 5, 6, 7},
    { 0, 6, 0, 7}, 
    { 0, 7, 0, 7}, 

    { 1, 0, 4, 7},
    { 1, 4, 0, 7},
    { 1, 5, 0, 7},
    { 1, 6, 0, 7},
    { 1, 7, 0, 7},

    { 2, 1, 5, 7},
    { 2, 5, 0, 7},
    { 2, 6, 0, 7},
    { 2, 7, 0, 7}
};
#define NDUMMY (sizeof(tpmTMDummy)/sizeof(struct tpmTMDummyMap))

struct tpmTMBlock {
    int nsamples;
    int maxsamples;	
    int index;
    int filter_type;
    int mapidx;
    double chan[TMBLOCK_NCHAN][TMBLOCK_MAXSIZE]; /* Preserves history! */
    double chan_sort[TMBLOCK_MAXSIZE];
};
    
long tpmTMicroInit(struct genSubRecord *psub)
{
    struct tpmTMBlock * pblock;	
    int mod, blk;
    int i;

    pblock = (struct tpmTMBlock *)malloc(sizeof(struct tpmTMBlock));
    if (!pblock) {
	printf("Unable to allocate tpmTMBlock structure!\n");
	return ERROR;
    }
    pblock->nsamples = 0;
    pblock->maxsamples = 1;
    pblock->index = 0;
    pblock->filter_type = 0;

    pblock->mapidx = -1;
    sscanf(psub->name, "tpm_TM_M%dB%d", &mod, &blk);
    for (i = 0; i < NDUMMY; i++) {
	if (mod == tpmTMDummy[i].module &&
	    blk == tpmTMDummy[i].block) {
	    pblock->mapidx = i;
	    break;
	}
    }
   
    psub->dpvt = (void *)pblock; 
    psub->udf = FALSE;
    return OK;
}

static int dblcompare(const void *pd1, const void *pd2) {
    double d1 = *(double *)pd1;
    double d2 = *(double *)pd2;

    return (d1 < d2 ? -1 : (d1 > d2 ? 1 : 0));
}

long tpmTMicroProcess(struct genSubRecord *psub)
{
    struct tpmTMBlock * pblock = (struct tpmTMBlock *)psub->dpvt;
    char *blk;
    char sjunk[16];
    double *c[TMBLOCK_NCHAN];
    double *c_delta[TMBLOCK_NCHAN];
    double ambient;
    double scale;
    long junk;
    long status;
    long retval;
    int i;

    retval = 0; /* OK */

    blk = genSubStructInput(psub, 120, &psub->a, &status);
    pblock->filter_type = genSubLongInput(psub, &psub->b, &status);
    pblock->maxsamples = genSubLongInput(psub, &psub->c, &status);
    ambient = genSubDoubleInput(psub, &psub->d, &status);
    scale = genSubDoubleInput(psub, &psub->e, &status);

    if (pblock->maxsamples < 1) pblock->maxsamples = 1;
    if (pblock->maxsamples > TMBLOCK_MAXSIZE) pblock->maxsamples = TMBLOCK_MAXSIZE;	

    for (i = 0; i < TMBLOCK_NCHAN; i++) {
        c[i] = genSubDoubleOutput(psub, &psub->vala + i, &status);
        c_delta[i] = genSubDoubleOutput(psub, &psub->vala + i + TMBLOCK_NCHAN, &status);
    }

    sscanf(blk, 
	"%d %d %s %d %lf %d %lf %d %lf %d %lf %d %lf %d %lf %d %lf %d %lf",
	&junk, &junk, sjunk,
	&junk, &pblock->chan[0][pblock->index], 
	&junk, &pblock->chan[1][pblock->index], 
	&junk, &pblock->chan[2][pblock->index], 
	&junk, &pblock->chan[3][pblock->index], 
	&junk, &pblock->chan[4][pblock->index], 
	&junk, &pblock->chan[5][pblock->index], 
	&junk, &pblock->chan[6][pblock->index], 
	&junk, &pblock->chan[7][pblock->index]);

#ifdef DBG
    printf("index:%d n:%d max:%d\n", pblock->index, pblock->nsamples, pblock->maxsamples);
    for (i = 0; i< TMBLOCK_NCHAN; i++) 
	printf("%.3f ", pblock->chan[i][pblock->index]);
    printf("\n");
#endif

    pblock->index = (++pblock->index) % pblock->maxsamples;
    pblock->nsamples = (pblock->nsamples < pblock->maxsamples ? 
			pblock->nsamples + 1 : pblock->maxsamples);


    for (i = 0; i < TMBLOCK_NCHAN; i++) {
        memcpy(pblock->chan_sort, pblock->chan[i], sizeof(pblock->chan[i]));
        qsort(pblock->chan_sort, pblock->nsamples, sizeof(double), dblcompare);
        *c[i] = pblock->chan_sort[pblock->nsamples/2];
	*c_delta[i] = (*c[i] - ambient)*scale;
#ifdef DBG
	printf("%.3f ", *c[i]);
#endif
	if (!(pblock->mapidx >= 0 && 
	  	i >= tpmTMDummy[pblock->mapidx].startchan &&
		i <= tpmTMDummy[pblock->mapidx].endchan) &&
		*c[i] <= BAD_SENSOR_VALUE) {
	    retval = -1;
	}
    }

#ifdef DBG
    printf("\n");
#endif
   
    return retval;
}
