/*
 * tpmGalil.st - EPICS state machine for parsing SDSS Galil status
 * Peregrine M. McGehee/LANL
 *
 * 28 April 2004	- Changed WF length to 140
 * 26 April 2004 	- Added M2 piezos.
 * 9 March 2001 	- Original.
*/

program galil_parse 
%%#include <math.h>

/*
 * Here are the strings returned from the M1 and M2 Galil Aux Ports.
*/
char gal1_wf[140];
assign gal1_wf to "tpm_GAL1";
monitor gal1_wf;
evflag efgal1;
sync gal1_wf efgal1;

char gal2_wf[140];
assign gal2_wf to "tpm_GAL2";
monitor gal2_wf;
evflag efgal2;
sync gal2_wf efgal2;

int i;
int status;
char str[40];

/* 
 * M1 Galil specific channels (6 axes)
*/
#define NM1AXIS 6
#define NM1PIEZO 0 
float gal1_cur[NM1AXIS];
float gal1_com[NM1AXIS];
short gal1_hom[NM1AXIS];
float gal1_tscur;
float gal1_tscom;

assign gal1_cur to {"", "", "", "", "", "" };
assign gal1_com to {"", "", "", "", "", "" };
assign gal1_hom to {"", "", "", "", "", "" };

assign gal1_tscur to "tpm_GAL1:tsCur";
assign gal1_tscom to "tpm_GAL1:tsCom";

/*
 * M2 Galil specific channels (5 axes + 3 piezos)
*/
#define NM2AXIS 5
#define NM2PIEZO 3
float gal2_cur[NM2AXIS];
float gal2_com[NM2AXIS];
short gal2_hom[NM2AXIS];
float gal2_tscur;
float gal2_tscom;
float gal2_piezo[NM2PIEZO];
short gal2_piezostat;

assign gal2_cur to {"", "", "", "", "" };
assign gal2_com to {"", "", "", "", "" };
assign gal2_hom to {"", "", "", "", "" };
assign gal2_piezo to {"tpm_M2GAL_PZA", "tpm_M2GAL_PZB", "tpm_M2GAL_PZC"};

assign gal2_tscur to "tpm_GAL2:tsCur";
assign gal2_tscom to "tpm_GAL2:tsCom";
assign gal2_piezostat to "tpm_M2GAL_PZStat";


/*
 * Stateset galil_parse 
*/
ss galil_parse {
    state init {
	when() {
	    for (i = 0; i < NM1AXIS; i++) {
	        sprintf(str, "tpm_M1GAL%dA", i);
	        pvAssign(gal1_cur[i], str);
		sprintf(str, "tpm_M1GAL%dC", i);
		pvAssign(gal1_com[i], str);
		sprintf(str, "tpm_M1GAL%dH", i);
		pvAssign(gal1_hom[i], str);
	    }	
	    for (i = 0; i < NM2AXIS; i++) {
	        sprintf(str, "tpm_M2GAL%dA", i);
	        pvAssign(gal2_cur[i], str);
		sprintf(str, "tpm_M2GAL%dC", i);
		pvAssign(gal2_com[i], str);
		sprintf(str, "tpm_M2GAL%dH", i);
		pvAssign(gal2_hom[i], str);
	    }	
	} state check 
    }	

    state check {
	when(efTestAndClear(efgal1)) {
	    status = 
		update_status(NM1AXIS, NM1PIEZO, 
			gal1_wf, gal1_cur, gal1_com, gal1_hom, 
			&gal1_tscur, &gal1_tscom,
			NULL, NULL);
	    if (status == 0) {
	        for (i = 0; i < NM1AXIS; i++) {
		    pvPut(gal1_cur[i]);
		    pvPut(gal1_com[i]);
		    pvPut(gal1_hom[i]);
	        }
		pvPut(gal1_tscur);
		pvPut(gal1_tscom);
	    }
	} state check

	when(efTestAndClear(efgal2)) {
	    status =
		update_status(NM2AXIS, NM2PIEZO,
			gal2_wf, gal2_cur, gal2_com, gal2_hom,
			&gal2_tscur, &gal2_tscom,
			gal2_piezo, &gal2_piezostat);
	    if (status == 0) {
		for (i = 0; i < NM2AXIS; i++) {
	    	    pvPut(gal2_cur[i]);
		    pvPut(gal2_com[i]);
		    pvPut(gal2_hom[i]);
		}
		pvPut(gal2_tscur);
		pvPut(gal2_tscom);
		for (i = 0; i < NM2PIEZO; i++) {
		    pvPut(gal2_piezo[i]);
		}
		pvPut(gal2_piezostat);
	    }
	} state check
    }
}

%{
int GALIL_DEBUG = 0;

int 
update_status(int naxis, int npiezo,
		char * wf, float *cur, float *com, short *hom, 
		float *tscur, float *tscom,
		float *piezo, short *piezostat)
{
    int nb;
    int sl;	
    int i;
    float t[15]; /* static allocation with room to spare. */
    float *ptr;
    int idx_count;
    int idx_axis;
    int idx_homed;
    int idx_type;
    int idx_tstmp;
    int idx_piezo;
    int idx_pstat;

    idx_count = 0;	/* character count */
    idx_axis = 1;	/* first axis */
    idx_homed = idx_axis + naxis; /* Homed status word */
    idx_type = idx_homed + 1; /* commanded/current */
    idx_tstmp = idx_type + 1; /* timestamp */
    idx_piezo = idx_tstmp + 1; /* first piezo */
    idx_pstat = idx_piezo + npiezo; /* Piezo status word */

    nb = sscanf(wf, 
		"%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f",
		&t[0], &t[1], &t[2], &t[3], &t[4],
		&t[5], &t[6], &t[7], &t[8], &t[9],
		&t[10], &t[11], &t[12], &t[13], &t[14]);
    if (GALIL_DEBUG && npiezo > 0)  printf("Read %d tokens from %s\n", nb, wf);

/*
 * Sanity check the string for number of tokens and length
 */
    if ((sl = strlen(wf)) < 5) {
	return -1;
    }
    if (nb < 4) {
        printf("Only %d tokens read from %s\n", nb, wf);
	return -2;
    }
    if (sl < t[idx_count]) {
	printf("Read %d chars when expected %f\n", sl, t[idx_count]);
	return -3;
    }
/*
 * String seems OK, lets go ahead and update.
*/
    if (GALIL_DEBUG && npiezo > 0) printf("updating cur=1/com=0: %f\n", t[idx_type]);

    ptr = (t[idx_type] ? cur : com);
    for (i = 0; i < naxis; i++)	{
	ptr[i] = t[idx_axis + i];
	hom[i] = (((int)t[idx_homed]) & (1 << i)) ? 1 : 0;
	if (GALIL_DEBUG && npiezo > 0) printf("ptr[%d]=%f hom[%d]=%f\n", i, ptr[i], i, hom[i]);
    }
    if (t[nb-2]) {
	*tscur = t[idx_tstmp];
    } else {
	*tscom = t[idx_tstmp];
    }

/*
 * Update piezos if supplied...
*/
    if (npiezo > 0) {
	for (i = 0; i < npiezo; i++) {
	    piezo[i] = t[idx_piezo + i];
	    if (GALIL_DEBUG && npiezo > 0) printf("piezo[%d]=%f\n", i, piezo[i]);
	}
	*piezostat = t[idx_pstat];
    }

    return 0;
}
}%
