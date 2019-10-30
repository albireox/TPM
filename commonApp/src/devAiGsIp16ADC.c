/* *****************************************************************************
**
**	Module:		devAiGsIp16ADC.c
**	Purpose:	Device Support Routines for GreenSpring IP-16ADC analog input
**
**		This file was taken directly from Kay-Uwe Kasemir's 4-9-96 code.
**		In most cases, no changes were necessary, if changes were made,
**		they are documented in the comments below.  The revision history
**		before the start date on this file is maintained so that prior
**		revisions can be tracked.
**
**		This module contains the EPICS device support options for the
**		IP16ADC analog input modules.
**
** *****************************************************************************
**
**	Written By:		G.A. Domer, AlliedSignal FM&T
**	Started On:		05/05/98
**	Project Title:		EPICS - Driver
**	Commissioned by:	LANL
**
** *****************************************************************************
**
**	Functions Included:
**
**				static long special_linconv (struct aiRecord *pai, int after)
**
**				static long init_record (struct aiRecord *pai)
**
**				static long read_ai (struct aiRecord *pai)
**
** *****************************************************************************
**
**	  Date		Revisor	Ver #	Modification Description
**	________	_______	_____  	_________________________________________
**	04/09/96	 kuk			original version
**	05/05/98	 gad	1.0		Took over code development.
**	05/27/98	 gad			Changed the default range from -10 - 10V to
**								0 - 10V.
**	06/08/98	 gad			Removed references to ticks.
**  02/24/99     GAD            Added capabitlity to bypass filtering of a channel by putting
**                              a 'u' at the end of the options string for stating the range
**                              in the database.
**
**
**
**
**
**
**
**
**
**
** ****************************************************************************/

#include <vxWorks.h>
#include <types.h>
#include <stdioLib.h>
#include <string.h>
#include <alarm.h>
#include <cvtTable.h>
#include <dbDefs.h>
#include <dbAccess.h>
#include <recSup.h>
#include <devSup.h>
#include <devLib.h>
#include <link.h>
#include <dbScan.h>
#include <aiRecord.h>
#include <drvGsIp16ADC.h>

typedef struct {
	IP16ADC			*ip;
}	DEV_PRIV;

/**/
/***********\
**
**  FUNCTION:       special_linconv - EPICS record special linear conversion
**
**  SYNTAX:         static long special_linconv (struct aiRecord *pai, int after)
**
**  ARGUMENTS:      struct aiRecord *pai	- pointer to record in database
**					int after				- flag to trigger conversion
**
**  RETURN VALUES:  0 - normal return (always returns 0)
**
**  DESCRIPTION:   This function calculates the conversion slope from the
**                 engineering units high and low (eguf and egul) and stores
**                 it back into the record at eslo.
**
\**********/
static long special_linconv (struct aiRecord *pai, int after) {
    if (after) {
		/* set linear conversion slope*/
		pai->eslo = (pai->eguf - pai->egul) / 0x10000;
	}

    return(0);
}

/**/
/***********\
**
**  FUNCTION:       init_record - EPICS record initialization
**
**  SYNTAX:         static long init_record (struct aiRecord *pai)
**
**  ARGUMENTS:      struct aiRecord *pai	- pointer to record in database
**
**  RETURN VALUES:  0 				- normal return
**					S_db_badField	- error
**
**  DESCRIPTION:	This function performs the initialization of the EPICS
**					record.  It verifies that the IP card and signal number
**					are in the proper range and then calls the device init
**					function in the driver code to complete the initialization
**					of the card.  This function is called once for each
**					record that uses this module type.
**
**
**
**
\**********/
static long init_record (struct aiRecord *pai) {
	short			card, signal;
	IP16ADC_V_range	range;
	DEV_PRIV		*priv;

	DEBUG(
	printf ("init_record '%s'\n", pai->name);
	)

    /* ai.inp must be an VME_IO */
    if (pai->inp.type != VME_IO) {
		recGblRecordError(S_db_badField,(void *)pai,
			"devAiGsIp16ADC (init_record) Illegal INP field");
		return(S_db_badField);
    }

	special_linconv (pai, 1);

    /*	check card & signal numbers */
	card   = pai->inp.value.vmeio.card;
	signal = pai->inp.value.vmeio.signal;

	if (card < 0  ||  card >= IPIC_NUM_CARDS) {
		recGblRecordError(S_db_badField,(void *)pai,
					"devAiGsIp16ADC (init_record) invalid card number");
		return(S_db_badField);
	}
	if (signal < 0  ||  signal > IP16ADC_NUM_CHANNELS) {
		recGblRecordError(S_db_badField,(void *)pai,
					"devAiGsIp16ADC (init_record) invalid signal number");
		return(S_db_badField);
	}

	range = ip16_0_10;		/* set the defualt range to 0-10VDC */
    if (strstr (pai->inp.value.vmeio.parm, "r0_5"))
		range = ip16_0_5;
	if (strstr (pai->inp.value.vmeio.parm, "r0_10"))
		range = ip16_0_10;
	if (strstr (pai->inp.value.vmeio.parm, "r-5_5"))
		range = ip16_m5_5;
	if (strstr (pai->inp.value.vmeio.parm, "r-10_10"))
		range = ip16_m10_10;
    if (strstr (pai->inp.value.vmeio.parm, "r0_5u"))
        range = ip16_0_5u;
    if (strstr (pai->inp.value.vmeio.parm, "r0_10u"))
        range = ip16_0_10u;
    if (strstr (pai->inp.value.vmeio.parm, "r-5_5u"))
        range = ip16_m5_5u;
    if (strstr (pai->inp.value.vmeio.parm, "r-10_10u"))
        range = ip16_m10_10u;

	if (! (priv = (DEV_PRIV *) malloc (sizeof (DEV_PRIV))))	{
		recGblRecordError(S_db_badField,(void *)pai,
				"devAiGsIp16ADC (init_record) malloc DEV_PRIV failed");
		return(S_db_badField);
	}

	DEBUG(
	printf ("init_record '%s' calling ip16adc_init\n", pai->name);
	)

    if (! (priv->ip = ip_16adc_init (card, signal, range))) {
		recGblRecordError(S_db_badField,(void *)pai,
					"devAiGsIp16ADC (init_record) cannot initialize IP module");
		return(S_db_badField);
	}

	pai->dpvt = priv;

    return 0;
}

/**/
/***********\
**
**  FUNCTION:       read_ai - EPICS record write analog output
**
**  SYNTAX:         static long read_ai (struct aiRecord *pai)
**
**  ARGUMENTS:      struct aiRecord *pai	- pointer to record in database
**
**  RETURN VALUES:  0 	- normal return
**					2	- no conversion
**
**  DESCRIPTION:	This function performs the reading of the analog input
**					to the module.
**
\**********/
static long read_ai (struct aiRecord *pai) {
	DEV_PRIV		*priv = (DEV_PRIV *) pai->dpvt;
	Word			value;

	if (2 == ip16adc_get_channel (priv->ip,
			pai->inp.value.vmeio.signal, &value)) {
		if (recGblSetSevr(pai,READ_ALARM,INVALID_ALARM) && errVerbose
			&& (pai->stat!=READ_ALARM || pai->sevr!=INVALID_ALARM))
				recGblRecordError (-1,(void *)pai,
						"GreenSpring IP-16ADC driverError");

		return 2;	/* don't convert */
	}

	pai->rval = value;

	return 0;
}

/*	setup device-set */
struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	read_ai;
	DEVSUPFUN	special_linconv;
}	devAiGsIp16ADC = {
	6,
	NULL,
	NULL,
	init_record,
	NULL,
	read_ai,
	special_linconv
};


/*	EOF devAiGsIp16ADC.c */
