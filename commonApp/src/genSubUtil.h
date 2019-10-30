/* $Id: genSubUtil.h,v 1.1 2000/09/20 14:37:16 peregrin Exp $ */

/*+*********************************************************************
  Module:       genSubUtil.h

  Author:       W. Lupton

  Description:  Prototypes for genSubUtil.c functions
  *********************************************************************/

#ifndef INCgenSubUtilh
#define INCgenSubUtilh

/*
 * Allow inclusion of this file without defining genSub record structure
 */
#ifndef INCgenSubH
#define genSubRecord void
#endif

long                            /* (<) long value, 0 on error */
genSubLongInput(
    genSubRecord *prec,         /* (>) address of genSub record */
    void **field,               /* (>) address of A-U field */
    long *status );             /* (<) modified status, set to ERROR on error */

long *                          /* (<) address of long value, NULL on error  */
genSubLongInputPtr(
    genSubRecord *prec,         /* (>) address of genSub record */
    void **field,               /* (>) address of A-U field */
    long *status );             /* (<) modified status, set to ERROR on error */

long *                          /* (<) address of long value, NULL on error */
genSubLongOutput(
    genSubRecord *prec,         /* (>) address of genSub record */
    void **field,               /* (>) address of VALA-VALU field */
    long *status );             /* (<) modified status, set to ERROR on error */

double                          /* (<) double value, 0.0 on error */
genSubDoubleInput(
    genSubRecord *prec,         /* (>) address of genSub record */
    void **field,               /* (>) address of A-U field */
    long *status );             /* (<) modified status, set to ERROR on error */

double *                        /* (<) address of double value, NULL on error */
genSubDoubleInputPtr(
    genSubRecord *prec,         /* (>) address of genSub record */
    void **field,               /* (>) address of A-U field */
    long *status );             /* (<) modified status, set to ERROR on error */

double *                        /* (<) address of double value, NULL on error */
genSubDoubleOutput(
    genSubRecord *prec,         /* (>) address of genSub record */
    void **field,               /* (>) address of VALA-VALU field */
    long *status );             /* (<) modified status, set to ERROR on error */

char *                          /* (<) address of string, NULL on error */
genSubStringInput(
    genSubRecord *prec,         /* (>) address of genSub record */
    void **field,               /* (>) address of A-U field */
    long *status );             /* (<) modified status, set to ERROR on error */

char *                          /* (<) address of string value, NULL on error */
genSubStringOutput(
    genSubRecord *prec,         /* (>) address of genSub record */
    void **field,               /* (>) address of VALA-VALU field */
    long *status );             /* (<) modified status, set to ERROR on error */

void *                          /* (<) address of structure, NULL on error */
genSubStructInput(
    genSubRecord *prec,         /* (>) address of genSub record */
    size_t size,                /* (>) size (in bytes) of structure */
    void **field,               /* (>) address of A-U field */
    long *status );             /* (<) modified status, set to ERROR on error */

void *                          /* (<) address of structure, NULL on error */
genSubStructOutput(
    genSubRecord *prec,         /* (>) address of genSub record */
    size_t size,                /* (>) size (in bytes) of structure */
    void **field,               /* (>) address of VALA-VALU field */
    long *status );             /* (<) modified status, set to ERROR on error */

#endif  /* INCgenSubUtilh */

/*+*********************************************************************
* $Log: genSubUtil.h,v $
* Revision 1.1  2000/09/20 14:37:16  peregrin
* First checkin for v2_1_0.
*
* Revision 1.1  1999/04/08 15:58:15  gurd
* adding some InputPtr routines - adding this to share
*
* Revision 1.2  1998/09/23 23:03:03  gurd
* Add genSubLongInputPtr and genSubDoubleInputPtr routines
*
* Revision 1.1  1998/07/24 17:51:54  wright
* added support for gensub
*
* Revision 1.2  1998/02/06 08:25:36  ktsubota
* Initial EPICS R3.13 conversion
*
* Revision 1.1.2.1  1997/10/15 01:17:32  wlupton
* R3.13 version from Andy Foster
*
* Revision 1.1  1997/06/27 20:26:24  wlupton
* initial insertion
*
***********************************************************************/
