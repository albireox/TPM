/*
 * tpmAlarm.st - EPICS state machine for SDSS telescope performance tests.
 * Peregrine M. McGehee/LANL
 * 10 February 2000
*/

program performance_check
%%#include <math.h>

/*
 * Here are variables and channels for the motor current limit checks.
*/
#define NCUR 3
int icur;
float current[NCUR]; 
assign current[0] to "tpm_ALMTRC1";
assign current[1] to "tpm_AZMTRC1";
assign current[2] to "tpm_ROMTRC";
float current_limit[NCUR];
assign current_limit[0] to "tpm_alcurlim";
assign current_limit[1] to "tpm_azcurlim";
assign current_limit[2] to "tpm_rocurlim";
int current_state[NCUR];
assign current_state[0] to "tpm_alcurstate";
assign current_state[1] to "tpm_azcurstate";
assign current_state[2] to "tpm_rocurstate";

/*
 * Here are the variables and the channels for the MIG 
 * rate of change tests.
*/
#define NMIG 10
int imig;
float dmigdt[NMIG];
assign dmigdt[0] to "tpm_dmig1a_dt";
assign dmigdt[1] to "tpm_dmig1b_dt";
assign dmigdt[2] to "tpm_dmig1c_dt";
assign dmigdt[3] to "tpm_dmig1dt_dt";
assign dmigdt[4] to "tpm_dmig1el_dt";
assign dmigdt[5] to "tpm_dmig1fl_dt";
assign dmigdt[6] to "tpm_dmig2a_dt";
assign dmigdt[7] to "tpm_dmig2b_dt";
assign dmigdt[8] to "tpm_dmig2c_dt";
assign dmigdt[9] to "tpm_dmig2dt_dt";
float dmigdt_limit[NMIG];
assign dmigdt_limit[0] to "tpm_dmig1a_dtlim";
assign dmigdt_limit[1] to "tpm_dmig1b_dtlim";
assign dmigdt_limit[2] to "tpm_dmig1c_dtlim";
assign dmigdt_limit[3] to "tpm_dmig1dt_dtlim";
assign dmigdt_limit[4] to "tpm_dmig1el_dtlim";
assign dmigdt_limit[5] to "tpm_dmig1fl_dtlim";
assign dmigdt_limit[6] to "tpm_dmig2a_dtlim";
assign dmigdt_limit[7] to "tpm_dmig2b_dtlim";
assign dmigdt_limit[8] to "tpm_dmig2c_dtlim";
assign dmigdt_limit[9] to "tpm_dmig2dt_dtlim";
int dmigdt_state[NMIG];
assign dmigdt_state[0] to "tpm_dmig1a_dtstate";
assign dmigdt_state[1] to "tpm_dmig1b_dtstate";
assign dmigdt_state[2] to "tpm_dmig1c_dtstate";
assign dmigdt_state[3] to "tpm_dmig1dt_dtstate";
assign dmigdt_state[4] to "tpm_dmig1el_dtstate";
assign dmigdt_state[5] to "tpm_dmig1fl_dtstate";
assign dmigdt_state[6] to "tpm_dmig2a_dtstate";
assign dmigdt_state[7] to "tpm_dmig2b_dtstate";
assign dmigdt_state[8] to "tpm_dmig2c_dtstate";
assign dmigdt_state[9] to "tpm_dmig2dt_dtstate";

/*
 * Here are the variables and channels for the servo error tests.
*/
#define NTRACK 3
int itrack;
float mcpvel[NTRACK];
assign mcpvel[0] to "tpm_ALMCPVEL";
assign mcpvel[1] to "tpm_AZMCPVEL";
assign mcpvel[2] to "tpm_ROMCPVEL";
float erravg[NTRACK];
assign erravg[0] to "tpm_alterravg";
assign erravg[1] to "tpm_azerravg";
assign erravg[2] to "tpm_roterravg";
float track_limit[NTRACK];
assign track_limit[0] to "tpm_altrack";
assign track_limit[1] to "tpm_aztrack";
assign track_limit[2] to "tpm_rotrack";
float err_limit[NTRACK];
assign err_limit[0] to "tpm_alerrlim";
assign err_limit[1] to "tpm_azerrlim";
assign err_limit[2] to "tpm_roerrlim";
int mcpvel_state[NTRACK];
assign mcpvel_state[0] to "tpm_alerrstate";
assign mcpvel_state[1] to "tpm_azerrstate";
assign mcpvel_state[2] to "tpm_roerrstate"; 

/*
 * Variables for watcher interface.
*/
int i;
int seqno = 0;
char buf[132];

/*
 * Stateset current_check
*/
ss current_check {
    state init {
	when() {
	    for (icur = 0; icur < NCUR; icur++) {
	        current_limit[icur] = 5000.0;
	        pvPut(current_limit[icur]);
	    }
	} state check
    }	

    state check {
	when(delay(0.2)) {
	    for (icur = 0; icur < NCUR; icur++) {
		pvGet(current[icur]);
		pvGet(current_limit[icur]);
		current_state[icur] = 
			(fabs(current[icur]) <= current_limit[icur]);
	        pvPut(current_state[icur]);
	    }
	} state check
    }	
}

/*
 * Stateset dmigdt_check
*/
ss dmigdt_check {
    state init {
	when() {
	    for (imig = 0; imig < NMIG; imig++) {
	        dmigdt_limit[imig] = 1.0;
	        pvPut(dmigdt_limit[imig]);
	    }
	} state check
    }	

    state check {
	when(delay(0.2)) {
	    for (imig = 0; imig < NMIG; imig++) {
		pvGet(dmigdt[imig]);
		pvGet(dmigdt_limit[imig]);
		dmigdt_state[imig] = 
			(fabs(dmigdt[imig]) <= dmigdt_limit[imig]);
	        pvPut(dmigdt_state[imig]);
	    }
	} state check
    }	
}

/*
 * Stateset mcpvel_check
*/
ss mcpvel_check {
    state init {
	when() {
	    for (itrack = 0; itrack < NTRACK; itrack++) {
	        track_limit[itrack] = 10000.0;
	        pvPut(track_limit[itrack]);
		err_limit[itrack] = 10.0;
		pvPut(err_limit[itrack]);
	    }
	} state check
    }	

    state check {
	when(delay(0.2)) {
	    for (itrack = 0; itrack < NTRACK; itrack++) {
		pvGet(mcpvel[itrack]);
		pvGet(erravg[itrack]);
		pvGet(track_limit[itrack]);
		pvGet(err_limit[itrack]);
		mcpvel_state[itrack] = 
			(fabs(mcpvel[itrack]) > track_limit[itrack]) ||
			(fabs(erravg[itrack]) < err_limit[itrack]);
	        pvPut(mcpvel_state[itrack]);
	    }
	} state check
    }	
}
/*
 * Stateset watcher - send UDP packets since channel access 
 * not spoken here.
*/
ss watcher {
    state init {
	when() {
	    if (udpInit(30311)) {
	        printf("UDP init failed!\n");
	    }
	} state send
    } 

    state send {
	when(delay(10.0)) {
#ifdef TEST
	    for (i = 0; i < NCUR; i++) {
		if (!current_state[i]) printf("cur %d fault\n", i);
	    }
	    for (i = 0; i < NMIG; i++) {
		if (!mig_state[i]) printf("mig %d fault\n", i);
	    }
	    for (i = 0; i < NTRACK; i++) {
		if (!mcpvel_state[i]) printf("track %d fault\n", i);  
            }	
#endif
	    seqno++;
/* create a suitable buffer... */	
	    udpSend(buf);
	
	} state send
    }
}

%{
int udpInit(int pnum) {
    return 0;
}

int udpSend(char * buf) {
    return 0;
}
}%
 


	



