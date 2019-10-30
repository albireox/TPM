/*
 Module:        devAcromagIp470.c
 
 Author:        Peregrine M. McGehee
 
 Description:   Device Support routines for the Acromag IP-470 DIO IPM.
 
 Functions:
 name           description
 ----           -----------------------------------------------------------
 <funcName>     What this function does.
 
 History:
 who            when       	what
 ---            --------   	------------------------------------------------
 PMM            30 Aug 2000   	Original
 PMM		13 Sep 2000	Inverted bits on input/output.
 PMM		21 Dec 2000	Added bit offset support (from S INP field)
**************************************************************************/
#include        <vxWorks.h>
#include        <types.h>            
#include        <stdioLib.h>
#include        <intLib.h>
#include        <sysLib.h>
#include        <string.h>
#include        <logLib.h>
#include        <symLib.h>
#include        <sysSymTbl.h>
#include 	<stdlib.h>
 
#include        <alarm.h>
#include        <cvtTable.h>
#include        <dbDefs.h>
#include        <dbAccess.h>
#include        <recSup.h>
#include        <devSup.h>
#include        <link.h>
#include        <dbScan.h>
#include        <errMdef.h>
 
#include        <dbCommon.h>         
#include        <longinRecord.h>
#include        <longoutRecord.h>
 
#include        "ip470.h"
#include        "devAcromagIp470.h"
 
void cnfg470(); 
void rmid470();
void rsts470();
long rprt470();
long wprt470();

/*
 * Use of Device Private field (DPVT)
 * DPVT stores the address of the struct cblk470 created by the driver
 * to hold the module configuration
 * 
 * Before iocInit:
 * devname = ip470DevCreate(void *addr)
 *
 * In record:
 * INP/OUT (VME_IO) = #C<port> S<bit> @<devname>
 * <port> is 0..5, <bitmask> is 0..255
 * 
*/
int IP470_DBG=0;

struct cblk470 * 
ip470DevCreate(void *addr)
{
    struct cblk470 *pcblk;
    int i;

    pcblk = (struct cblk470 *)malloc(sizeof(struct cblk470));

    if (pcblk) 
    {
	pcblk->brd_ptr = (struct map470 *)addr;
        pcblk->ip_pos = (((int)addr >> 8)  & 3);
        pcblk->param = 0;
	pcblk->e_mode = 0;
	pcblk->mask_reg = 0;
	pcblk->deb_control = 0;
	pcblk->deb_clock = 0;
	pcblk->enable = 0;
	pcblk->vector = 176;
        for (i=0; i < 2; i++)
        {
	    pcblk->ev_control[i] = 0;
	    pcblk->deb_duration[i] = 0;
	}
	pcblk->last_chan = 0;
	pcblk->last_state = 0;

        cnfg470(pcblk);
    } else {
	printf("Malloc of cblk470 failed!\n");
    }

    return pcblk;
}	

void
ip470Report(struct cblk470 *pcblk)
{
    int i;

    rmid470(pcblk); /* get board's ID info into structure */
    rsts470(pcblk); /* get board's status info into structure */

    printf("\n\nBoard Status Information\n");
    printf("\nInterrupt Enable Register:   %02x",pcblk->enable);
    printf("\nInterrupt Vector Register:   %02x",pcblk->vector);
    printf("\nLast Interrupting Channel:   %02x",pcblk->last_chan);
    printf("\nLast Interrupting State:     %02x",pcblk->last_state);
 
    printf("\n\nBoard ID Information\n");
    printf("\nIdentification:              ");
    for(i = 0; i < 4; i++)          /* identification */
        printf("%c",pcblk->id_prom[i]);
    printf("\nManufacturer's ID:           %x",(byte)pcblk->id_prom[4]);
    printf("\nIP Model Number:             %x",(byte)pcblk->id_prom[5]);
    printf("\nRevision:                    %x",(byte)pcblk->id_prom[6]);
    printf("\nReserved:                    %x",(byte)pcblk->id_prom[7]);
    printf("\nDriver I.D. (low):           %x",(byte)pcblk->id_prom[8]);
    printf("\nDriver I.D. (high):          %x",(byte)pcblk->id_prom[9]);
    printf("\nTotal I.D. Bytes:            %x",(byte)pcblk->id_prom[10]);
    printf("\nCRC:                         %x",(byte)pcblk->id_prom[11]);      
}

devAcromagIp470InitTable(struct dbCommon *precord, struct vmeio *pvmeio)
{                                                                  
    SYM_TYPE stype;
    char tableName[10];
    struct cblk470** pcblk = NULL;
 
    precord->dpvt = NULL;
 
    /* What's the name of the device... */
    sprintf(tableName, "%s", pvmeio->parm);
 
    /* Find the object that implements this device.*/
    if (symFindByName(sysSymTbl, tableName, (char **)&pcblk, &stype) ==
                OK)
    {
        precord->dpvt = (void *)*pcblk;
        precord->udf = FALSE;
        return (0);
    }
 
    recGblRecordError(S_db_badField,(void *)precord,      
           "\n\tdevAcromagIp470 (init_record) Illegal INP/OUT field");
    return (S_db_badField);
}
 
 
/*
 * LONGIN Record
 */
static long
init_longin_record(struct longinRecord *pli)
{
    struct cblk470 *pcblk;
    struct vmeio *pvmeio;
    long status = OK;	

    if (pli->inp.type == VME_IO) 
    {
        pvmeio = (struct vmeio *)&pli->inp.value.vmeio;
        if ((status = devAcromagIp470InitTable((struct dbCommon *)pli, pvmeio)) == OK) 
	{
	    pcblk = (struct cblk470 *)(pli->dpvt);
	    pcblk->mask_reg |= (1 << pvmeio->card);	
	    cnfg470(pcblk);
        }
    } else
    {
        recGblRecordError(S_db_badField, (void *)pli,
                          "\n\tdevAcromagIp470 (init_record) Illegal INP field");
	status = S_db_badField;
    }
    return status;
}
 
 
static long
read_longin(struct longinRecord *pli)
{
    int i;
    int status;
    struct vmeio *pvmeio = (struct vmeio *)&pli->inp.value.vmeio;
    struct cblk470 *pcblk = (struct cblk470 *)pli->dpvt;

    status = rprt470(pcblk, pvmeio->card);
    pli->val = (~status) & pvmeio->signal;
 
    return (0);
}

/*
 * LONGOUT Record
 */
static long
init_longout_record(struct longoutRecord *plo)
{
    struct cblk470 *pcblk;
    struct vmeio *pvmeio;
    long status = OK;	

    if (plo->out.type == VME_IO) 
    {
        pvmeio = (struct vmeio *)&plo->out.value.vmeio;
        if ((status = devAcromagIp470InitTable((struct dbCommon *)plo, pvmeio)) == OK) 
	{
	    pcblk = (struct cblk470 *)(plo->dpvt);
	    pcblk->mask_reg &= ~(1 << pvmeio->card);	
	    cnfg470(pcblk);
        }
    } else
    {
        recGblRecordError(S_db_badField, (void *)plo,
                          "\n\tdevAcromagIp470 (init_record) Illegal OUT field");
	status = S_db_badField;
    }
    return status;
}
 
 
long
ip470Write(struct cblk470 * pcblk, int port, int bmask, int val)
{
    long status;
    int oldval;

    oldval = rprt470(pcblk, port);
    oldval = oldval & ~bmask | (~val & bmask);
    if (IP470_DBG) printf("mask:%02x val:%02x wrote:%02x\n", 
		bmask, val,  oldval);
    status = wprt470(pcblk, port, oldval);
 
    return status;
}

static long
write_longout(struct longoutRecord *plo)
{
    int status;
    struct vmeio *pvmeio = (struct vmeio *)&plo->out.value.vmeio;
    struct cblk470 *pcblk = (struct cblk470 *)plo->dpvt;

    status = ip470Write(pcblk, pvmeio->card, pvmeio->signal, plo->val);
 
    return (0);
}
