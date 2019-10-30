/**************************************************************************
 Header:        devAcromagIp470.h
 
 Author:        Peregrine M. McGehee
 
 Description:   Header file for Acromag IP470 DIO  Device Support.
 
 History:
 who            when       	what
 ---            -----------   	------------------------------------------------
 PMM            30 Aug 2000     Original
**************************************************************************/
 
#ifndef DEV_ACROMAGIP470_H
#define DEV_ACROMAGIP470_H

struct cblk470 * i470DevCreate(void *);
 
static long init_longin_record(struct longinRecord *);
static long read_longin(struct longinRecord *);

struct
{
        long            number;
        long            (*report)();
        long            (*init)();
        long            (*init_record)(struct longinRecord *);
        long            (*get_ioint_info)();
        long            (*read_longin)(struct longinRecord *);
        long            (*special_linconv)();
} devLonginAcromagIp470 =
{
        6,
        NULL,
        NULL,
        init_longin_record,
        NULL,
        read_longin,
        NULL
};                

long init_longout_record(struct longoutRecord *);
static long write_longout(struct longoutRecord *);
 
struct
{
        long            number;
        long            (*report)();
        long            (*init)();
        long            (*init_record)(struct longoutRecord *);
        long            (*get_ioint_info)();
        long            (*write_longout)(struct longoutRecord *);
        long            (*special_linconv)();
} devLongoutAcromagIp470 =
{
        6,
        NULL,
        NULL,
        init_longout_record,
        NULL,
        write_longout,
        NULL
};
 
#endif                  
