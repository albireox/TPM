riginal Author: Matthias Clausen
 *      Current Author: Gabor Csuka, Bernd Schoeneburg
 *      Date:           
 *
 *      Experimental Physics and Industrial Control System (EPICS)
 *
 *      Initial development by:   DESY, Hamburg
 *
 *******************************************************************************
 * THIS IS NOT A FINAL VERSION !  THIS RECORD IS CURRENTLY UNDER DEVELLOPMENT !
 *******************************************************************************
 *
 *
 * Modification Log:
 * -----------------
 *
 *      .01     sbg     dec-02-1997     remove unused fields, double buffer, global flush
 *      .02     sbg     dec-03-1997     move to EPICS 3.13
 *
 */

#include        
#include        
#include        
#include        
#include        
#include        
#include        
#include        
#include        
#include        
#include        

#include        
#include        
#include        
#include        
#include        
#include        
#include        
#include        
#define GEN_SIZE_OFFSET
#include        
#undef  GEN_SIZE_OFFSET


/* Create RSET - Record Support Entry Table*/
#define report NULL
#define initialize NULL
static long init_record();
static long process();
static long special();
static long get_value();
static long cvt_dbaddr();
static long get_array_info();
static long put_array_info();
static long get_units();
static long get_precision();
#define get_enum_str NULL
#define get_enum_strs NULL
#define put_enum_str NULL
static long get_graphic_double();
static long get_control_double();
#define get_alarm_double NULL

struct rset archRSET={
        RSETNUMBER,
        report,
        initialize,
        init_record,
        process,
        special,
        get_value,
        cvt_dbaddr,
        get_array_info,
        put_array_info,
        get_units,
        get_precision,
        get_enum_str,
        get_enum_strs,
        put_enum_str,
        get_graphic_double,
        get_control_double,
        get_alarm_double };

static void monitor(archRecord *parch);
static void do_archive(archRecord *parch);


typedef struct { 

        short           status;                 /* status of value */
        short           severity;               /* severity of alarm */
        TS_STAMP        stamp;                  /* time stamp */
        float           value;                  /* current value */
} my_time_float;

static unsigned long FtofGlobal = 0L;





static long init_record(void *precord, int pass) 


{
 archRecord     *parch = (archRecord *) precord;
 int            i;
 long           status;
 SYM_TYPE       symType;
 FUNCPTR        psubscribe_fktn;

    if (pass==0){
        parch->ltim = 0L;
        parch->vals = (float *)malloc(parch->nval * sizeof(float));
        parch->tims = (long *)malloc(parch->nval * sizeof(unsigned long));
        parch->nscs = (long *)malloc(parch->nval * sizeof(unsigned long));
        parch->arch = (void *)malloc(parch->nval * sizeof(my_time_float));
        if (parch->vals == NULL || parch->tims == NULL
          || parch->nscs == NULL || parch->arch == NULL) {
                parch->udf = TRUE;
                parch->pact = TRUE;
                logMsg("archive: init: failed to allocate buffers!\n", 0,0,0,0,0,0);
                return 0;
        }
/* 
 *   Init   parch->tims [ ]   to  zero  !
 *          parch->lval       to  1.0e-99
 */

        parch->lval = 1.0e-99 ;


        for(i=0; i < parch->nval; i++)   
        {
                parch->tims[i] = 0;
                parch->nscs[i] = 0;
        }

        FtofGlobal++;
        parch->ftof = FtofGlobal % parch->ftim;
    }
    else {

        status = symFindByName(sysSymTbl, "_S_archFlushInit", (char**) &psubscribe_fktn, &symType);
        if ((status == 0) && (symType == (SYM_GLOBAL | SYM_TEXT)))
                                                        /* siehe /usr/wind/target/h/symbol.h */
                if ((*psubscribe_fktn)(parch) == ERROR)
                        logMsg("archive: init: failed to execute _S_archFlushInit!\n", 0,0,0,0,0,0);

        if (parch->inp.type == CONSTANT)
                recGblInitConstantLink(&parch->inp, DBF_DOUBLE, &parch->rval);
    
/**** CSG ****/ 
        recGblGetTimeStamp(parch);
        /* CURRENT - Force Time (Forceing send immideatelly ) + delay a little bit */
        parch->ltim = parch->time.secPastEpoch - parch->ftim + parch->ftof;
    }

    parch->pact = FALSE;
    return(0);
}


static long process(void *precord)
{
        archRecord      *parch = (archRecord *) precord;

        if (parch->pact)        /* memory alloc failed, can not work */
                return 0;

        parch->pact = TRUE;


        if (dbGetLink (&(parch->inp), DBR_DOUBLE, &(parch->rval), 0, 0) != 0)
                recGblSetSevr (parch, LINK_ALARM, INVALID_ALARM);

        recGblGetTimeStamp(parch);

        do_archive(parch);

        /* check event list */
        parch->udf=FALSE;

        monitor(parch);

        /* process the forward scan link record */
        recGblFwdLink(parch);

        parch->pact=FALSE;
        return(0);
}


static long special(DBADDR *paddr,int after)
{
    archRecord   *parch = (archRecord *)(paddr->precord);
    int                 special_type = paddr->special;
    int                 fieldIndex;

    if(!after) return(0);
    switch(special_type) {
    case(SPC_MOD):
        fieldIndex = dbGetFieldIndex(paddr);
        if ((fieldIndex == archRecordFLSH) && (parch->flsh != 0))
            monitor (parch);
        return(0);
    case(SPC_RESET):
        parch->nuse = 0;
        parch->ccnt = 0;
        parch->res  = 0;
        parch->tims[0] = 0;
        parch->nscs[0] = 0;
        return(0);
    default:
        recGblDbaddrError(S_db_badChoice,paddr,"arch: special");
        return(S_db_badChoice);
    }
}

static long get_value(void *precord, struct valueDes *pvdes)
{
    archRecord  *parch = (archRecord *) precord;

    pvdes->no_elements = parch->nval;
    pvdes->pvalue = parch->vals;
    pvdes->field_type = DBF_FLOAT;

    return(0);
}

static long cvt_dbaddr(DBADDR *paddr)
{
    archRecord          *parch = (archRecord *)paddr->precord;
    int                 fieldIndex = dbGetFieldIndex(paddr);

    paddr->no_elements = parch->nval;

    if(fieldIndex == archRecordVAL) {
        paddr->pfield = (void *)(parch->vals);
        paddr->field_type = DBF_FLOAT;
        paddr->field_size = sizeof(float);
        paddr->dbr_field_type = DBR_FLOAT;
    }
    else  {
        paddr->pfield = (void *)(parch->tims);
        paddr->field_type = DBF_ULONG ;
        paddr->field_size = sizeof(long);
        paddr->dbr_field_type = DBR_ULONG;
    }

    return(0);
}

static long get_array_info(DBADDR *paddr, long *no_elements, long *offset)
{
    archRecord *parch=(archRecord *)paddr->precord;

    *no_elements =  parch->nuse;
    *offset = 0;
    return(0);
}

static long put_array_info(DBADDR *paddr, long nNew)
{
  /*  not nessesary to do anything since we are using double buffering */
  /*  archRecord        *parch = (archRecord *)paddr->precord; */

  /*  parch->ccnt = (parch->ccnt + nNew) % parch->nval; */
  /*  parch->nuse = (parch->nuse + nNew); */
  /*  if(parch->nuse > parch->nval) parch->nuse = parch->nval; */
    return(0);
}

static long get_units(DBADDR *paddr, char *units)
{
    archRecord  *parch = (archRecord *)paddr->precord;
    int         fieldIndex = dbGetFieldIndex(paddr);

    switch (fieldIndex) {
        case archRecordVAL:
        case archRecordCVAL:
        case archRecordAVAR:
        case archRecordLVAL:
                strncpy(units,parch->egu,DB_UNITS_SIZE);
                break;
        case archRecordTIM:
        case archRecordSTIM:
        case archRecordFTIM:
        case archRecordLTIM:
        case archRecordFTOF:
                strcpy(units,"seconds");
                break;
        case archRecordRVAR:
                strcpy(units,"%");
                break;
        default:
                units[0] = '\0';
                break;
    }

    return(0);
}

static long get_precision(DBADDR *paddr, long *precision)
{
    archRecord  *parch=(archRecord *)paddr->precord;
    int         fieldIndex = dbGetFieldIndex(paddr);

    switch (fieldIndex) {
        case archRecordCVAL:
        case archRecordAVAR:
        case archRecordLVAL:
                *precision = parch->prec;
                break;
        default:
                recGblGetPrec(paddr,precision);
                break;
    }

    return(0);
}

static long get_graphic_double(DBADDR *paddr, struct dbr_grDouble *pgd)
{
    archRecord *parch=(archRecord *)paddr->precord;
    int         fieldIndex = dbGetFieldIndex(paddr);

    switch (fieldIndex) {
        case archRecordCVAL:
        case archRecordLVAL:
                pgd->upper_disp_limit = parch->hopr;
                pgd->lower_disp_limit = parch->lopr;
                break;
        default:
                recGblGetGraphicDouble(paddr,pgd);
                break;
    }

    return(0);
}

static long get_control_double(DBADDR *paddr, struct dbr_ctrlDouble *pcd)
{
    archRecord *parch=(archRecord *)paddr->precord;
    int         fieldIndex = dbGetFieldIndex(paddr);

    switch (fieldIndex) {
        case archRecordCVAL:
        case archRecordLVAL:
                pcd->upper_ctrl_limit = parch->hopr;
                pcd->lower_ctrl_limit = parch->lopr;
                break;
        default:
                recGblGetControlDouble(paddr,pcd);
                break;
    }

    return(0);
}


static void copyBuffer (archRecord *parch)
{
        register int offset, i;

        if (parch->nuse < parch->nval)
                offset = 0;
        else
                offset=parch->ccnt+1;

        for (i=0; i < parch->nuse; i++) {

/* filling my_time_float */ 

                parch->vals[i] = ((my_time_float *) parch->arch)[offset].value;
                parch->tims[i] = ((my_time_float *) parch->arch)[offset].stamp.secPastEpoch;
                parch->nscs[i] = ((my_time_float *) parch->arch)[offset].stamp.nsec;
                offset = (offset + 1) % parch->nval;
        }
        parch->nusb = parch->nuse;
}


static void monitor(archRecord *parch) 

{
        unsigned short  monitor_mask;

        /* update stat and sevr */
        monitor_mask = recGblResetAlarms(parch);

        /* alarm condition changed this scan */
        if (monitor_mask) {
                /* post stat and nsev fields */


db_post_events(parch,&parch->stat,DBE_VALUE); db_post_events(parch,&parch->sevr,DBE_VALUE); 

        }


        if (    (parch->ccnt == 0) || (parch->ccnt == (parch->nval)/2)
                                        /* we nulled a counter */
        ||      (parch->time.secPastEpoch - parch->ltim >= parch->ftim) ) {
                                        /* we waited the specified time interval */
                monitor_mask = DBE_VALUE | DBE_LOG;
                parch->ltim = parch->time.secPastEpoch;
        }

        if (parch->flsh != 0) {
                /* DON'T UPDATE THE ltm FIELD !!! Important otherwise disturb CPU load equilization */
                monitor_mask = DBE_VALUE | DBE_LOG;
                parch->flsh  = 0;
        }

        if (monitor_mask) {
                copyBuffer (parch);
                db_post_events(parch,parch->vals,monitor_mask);
                db_post_events(parch,parch->tims,monitor_mask);
                db_post_events(parch,parch->nscs,monitor_mask);
        }

        return;
}

static void do_archive(archRecord *parch)
{
/* treat  vals  as a pointer to a circular buffer*/

        long              offset= parch->ccnt;
        long              nuse  = parch->nuse;
        long              nval  = parch->nval;
        double            local_val, local_diff, local_dband;
        unsigned short    new_val_short, local_val_short;
        unsigned int      archive_flag;

        /* update stat and sevr */
        archive_flag = (unsigned int) recGblResetAlarms(parch);


/* do bitmask if bitmask field MASK is not equal to 0 */ 

        if (archive_flag)
                local_val = parch->cval;
        else {
                if(parch->mask) {
                        new_val_short   = (unsigned short)(parch->rval);
                        local_val_short = new_val_short & parch->mask;
                        local_val       = (double)(local_val_short);
                        local_diff = 0.0;       /* unused */
                        local_dband =  0.0;     /* unused */
                }
                else {
                        local_val = parch->rval;
                        local_diff = fabs(local_val - parch->lval);
                        local_dband = fabs(parch->lval) * parch->rvar / 100.0; /* % usage !!!! */
                }
        

                if (parch->time.secPastEpoch - parch->tims[offset] > parch->stim)
                        archive_flag = TRUE;
                else
                {
                        if (parch->mask)
                                archive_flag = (local_val != parch->lval);
                        else {

switch (parch->pcab) 

                                
                                {
                                case archPCAB_absolute:         /*  absolute    */
                                        archive_flag = (local_diff > parch->avar);
                                        break;
                                case archPCAB_relative:         /* % - relative */
                                        archive_flag = (local_diff > local_dband);
                                        break;
                                case archPCAB_abs_and_rel:      /* abs && rel = maximum */
                                        archive_flag = (local_diff > parch->avar) && 
                                                (local_diff > local_dband);
                                        break;
                                case archPCAB_abs_or_rel:       /* abs || rel = minimum */
                                        archive_flag = (local_diff > parch->avar) ||
                                                (local_diff > local_dband);
                                        break;
                                case archPCAB_on_change:        /* on any change */
                                        archive_flag = (local_diff > 0.0);
                                        break;
                                case archPCAB_allways:
                                        archive_flag = TRUE;
                                        break;
                                default:
                                        archive_flag = FALSE;
                                        break;

                                }
                        }
                }


                parch->cval = local_val;
        }

        if(archive_flag) {

                parch->lval = local_val;

/*
 *           Increment our counter within the boundaries of the circular
 *                     buffer
 *
 */

/*
 *      ONLY ONCE AT INITIAL TIME (indicator: parch->tims[0] == 0)
 *      (not nice but working)
 */
                if (parch->tims[0] != 0)
                {
                        offset++;
                        if (offset == parch->nval) offset = 0;
                }

/*
 *    Store time, value and alarm in archive array:
 *              value = new_value*span + calc_offset
 */

                ((my_time_float *) parch->arch)[offset].value = local_val;
                ((my_time_float *) parch->arch)[offset].stamp = parch->time;
                ((my_time_float *) parch->arch)[offset].status = parch->stat;
                ((my_time_float *) parch->arch)[offset].severity = parch->sevr;

                nuse = nuse+1;
                if(nuse>nval) nuse=nval;
                parch->ccnt = offset;
                parch->nuse = nuse;

        }

        return;
}



