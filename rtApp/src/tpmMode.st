/*
 * tpmMode.st - EPICS state machine for SDSS Telescope Mode.
 * Peregrine M. McGehee/LANL
 * 12 January 2003
*/
/*
tpm_TM_M1Upper_FB.SEVR	M1_TEMP
tpm_TM_M1Lower_FB.SEVR	M1_TEMP
tpm_TM_M1Right_FB.SEVR	M1_TEMP
tpm_TM_M1Left_FB.SEVR	M1_TEMP
tpm_TM_M1_FB.SEVR	M1_TEMP
tpm_TM_M1_Rgrad.SEVR	M1_TEMP
tpm_TM_M1_Telair.SEVR	M1_TEMP
tpm_TM_M1_Denv.SEVR	M1_TEMP
tpm_TM_M2Upper_FB.SEVR	M2_TEMP
tpm_TM_M2Lower_FB.SEVR	M2_TEMP
tpm_TM_M2Right_FB.SEVR	M2_TEMP
tpm_TM_M2Left_FB.SEVR	M2_TEMP
tpm_TM_M2_FB.SEVR	M2_TEMP
tpm_TM_M2_Rgrad.SEVR	M2_TEMP
tpm_TM_Floor_Telair.SEVR	ENV_TEMP	
tpm_TM_Return_Telair.SEVR  	ENV_TEMP	
tpm_TM_Lowair_Telair.SEVR	ENV_TEMP
tpm_alttrackchk.SEVR	 	TRACKING	
tpm_aztrackchk.SEVR	 	TRACKING	
tpm_rottrackchk.SEVR		TRACKING	
tpm_ALMTRC1.SEVR	CURRENTS
tpm_ALMTRC2.SEVR	CURRENTS
tpm_AZMTRC1.SEVR	CURRENTS
tpm_AZMTRC2.SEVR	CURRENTS
tpm_ROMTRC.SEVR        	CURRENTS
tpm_Ndewar_spectro.SEVR		DEWAR	
tpm_Sdewar_imager.SEVR		DEWAR	
tpm_MIG1A.SEVR			M1_MIG	
tpm_MIG2A.SEVR		 	M2_MIG	
tpm_GAL1.SEVR			M1_GALIL	
tpm_GAL2.SEVR		 	M2_GALIL	
tpm_TM_M0B0.SEVR	 	TMICRO	
tpm_TM_M0B0Chans.SEVR	TMICRO
tpm_TM_M0B1Chans.SEVR	TMICRO
tpm_TM_M0B2Chans.SEVR	TMICRO
tpm_TM_M0B3Chans.SEVR	TMICRO
tpm_TM_M0B4Chans.SEVR	TMICRO
tpm_TM_M0B5Chans.SEVR	TMICRO
tpm_TM_M0B6Chans.SEVR	TMICRO
tpm_TM_M0B7Chans.SEVR	TMICRO
tpm_TM_M1B0Chans.SEVR	TMICRO
tpm_TM_M1B1Chans.SEVR	TMICRO
tpm_TM_M1B2Chans.SEVR	TMICRO
tpm_TM_M1B3Chans.SEVR	TMICRO
tpm_TM_M1B4Chans.SEVR	TMICRO
tpm_TM_M1B5Chans.SEVR	TMICRO
tpm_TM_M1B6Chans.SEVR	TMICRO
tpm_TM_M1B7Chans.SEVR	TMICRO
tpm_TM_M2B0Chans.SEVR	TMICRO
tpm_TM_M2B1Chans.SEVR	TMICRO
tpm_TM_M2B2Chans.SEVR	TMICRO
tpm_TM_M2B3Chans.SEVR	TMICRO
tpm_TM_M2B4Chans.SEVR	TMICRO
tpm_TM_M2B5Chans.SEVR	TMICRO
tpm_TM_M2B6Chans.SEVR	TMICRO
tpm_TM_M2B7Chans.SEVR	TMICRO
*/
program mode_check
%%#include <math.h>
/* 
 * Here are the variables and channels for each of the alarms.
*/
#define NTA 17 
int ita;
float tahihi[NTA];
float talolo[NTA];
assign tahihi to {""};
assign talolo to {""};
string taname;
%%LOCAL void set_scale(float);
%%LOCAL char * get_tachan(int);
%%LOCAL float get_talim(int);

int telescope_mode;
assign telescope_mode to "tpm_telescope_mode";
monitor telescope_mode;

/*
 * Stateset temp_config 
*/
ss temp_config {
    state init {
	when() {
	    set_scale(3.0);
	    for (ita = 0; ita < NTA; ita++) {
		sprintf(taname, "%s.HIHI", get_tachan(ita));
		pvAssign(tahihi[ita], taname);
		tahihi[ita] = get_talim(ita);
		pvPut(tahihi[ita]); 

		sprintf(taname, "%s.LOLO", get_tachan(ita));
		pvAssign(talolo[ita], taname);
		talolo[ita] = -tahihi[ita];
		pvPut(talolo[ita]);
	    }
	} state check_mode 
    }	

    state check_mode {
        when (telescope_mode == 1) {
	    printf("Telescope is now in use\n");
	    set_scale(3.0);
	    for (ita = 0; ita < NTA; ita++) {
		tahihi[ita] = get_talim(ita);
		pvPut(tahihi[ita]); 

		talolo[ita] = -tahihi[ita];
		pvPut(talolo[ita]);
	    }
	} state telescope_on 

        when (telescope_mode == 0) {
	    printf("Telescope is now off\n");
	    set_scale(100.0);
	    for (ita = 0; ita < NTA; ita++) {
		tahihi[ita] = get_talim(ita);
		pvPut(tahihi[ita]); 

		talolo[ita] = -tahihi[ita];
		pvPut(talolo[ita]);
	    }
	} state telescope_off
    }

    state telescope_on {
	when (telescope_mode == 0) {
/* just return to check_mode to do the work */
	} state check_mode
    }
	
    state telescope_off {
	when (telescope_mode == 1) {
/* just return to check_mode to do the work */
	} state check_mode
    }
}

%{
LOCAL temp_alarm_scale = 3.0;

LOCAL void set_scale(float scale)
{
    temp_alarm_scale = scale;
    printf("Changed temperature alarm scale to %f\n", scale);
}
 
LOCAL char * get_tachan(int i)
{
    static char tachan[NTA][40] = {
	"tpm_TM_M1Upper_FB",
	"tpm_TM_M1Lower_FB",
	"tpm_TM_M1Right_FB",
	"tpm_TM_M1Left_FB",
	"tpm_TM_M1_FB",
	"tpm_TM_M1_Rgrad",
	"tpm_TM_M1_Telair",
	"tpm_TM_M1_Denv",
	"tpm_TM_M2Upper_FB",
	"tpm_TM_M2Lower_FB",
	"tpm_TM_M2Right_FB",
	"tpm_TM_M2Left_FB",
	"tpm_TM_M2_FB",
	"tpm_TM_M2_Rgrad",
	"tpm_TM_Floor_Telair",
	"tpm_TM_Return_Telair",
	"tpm_TM_Lowair_Telair"
    };
    if (i >= 0 && i < NTA) 
	return tachan[i];
    else
	return 0; 
}

LOCAL float get_talim(int i)
{
    static float tlim[NTA] = {
	0.2, 0.2, 0.2, 0.2, 0.2, 1.0, 2.0, 2.0,
	0.2, 0.2, 0.2, 0.2, 0.2, 1.0,
	2.0, 2.0, 3.0
    };
    if (i >= 0 && i < NTA) 
	return temp_alarm_scale * tlim[i];
    else
	return 0; 
}
}%
