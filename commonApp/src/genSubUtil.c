/* $Id: genSubUtil.c,v 1.1 2000/09/20 14:37:15 peregrin Exp $ */

/*+*********************************************************************
  Module:       genSubUtil.c
 
  Author:       W. Lupton
 
  Description:  Parameter validation routines for interfacing EPICS genSub
                records to C code

  Notes:        1. These routines validate that genSub fields reference data
                   of the expected type and size, returning either the field
                   value or else a pointer to the value

                2. They use a "modified status convention"; status is assumed
                   initialized to OK and is set to ERROR on an error; thus if
                   it is OK after several calls, all calls succeeded
  *********************************************************************/

#include    "vxWorks.h"         /* VxWorks definitions */

#include    "dbDefs.h"          /* EPICS definitions */
#include    "dbAccess.h"
#include    "stdio.h"
#include    "epicsPrint.h"

#include    "genSubRecord.h"    /* genSub record */
#include    "genSubUtil.h"      /* genSub utilities */

/*
 * Alpha suffices for field names (genSub has 21 inputs and 21 outputs)
 */
static char *alpha[] =
    { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J",
      "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U" };

#define NFIELD ( sizeof( alpha ) / sizeof( char * ) )

/*+*********************************************************************
  Function:     genSubLongInput()

  Author:       W. Lupton

  Abstract:     Return the value of a genSub long input field

  Description:  The supplied field address should be the address of one of
                the A-U fields. This is validated, the type is confirmed to
                be long, and the value is returned as the function value
  *********************************************************************/

long                            /* (<) long value, 0 on error */
genSubLongInput(
    genSubRecord *prec,         /* (>) address of genSub record */
    void **field,               /* (>) address of A-U field */
    long *status )              /* (<) modified status, set to ERROR on error */
{
    /* Field index (0=A, 1=B etc) */
    int i = field - &prec->a;

    /* Value and field type arrays */
    void           **val = &prec->a;
    unsigned short  *ft  = &prec->fta;

    /* If index is invalid, set status and return 0.0 */
    if ( i < 0 || i >= NFIELD ) {
        epicsPrintf( "%s bad field address passed to genSubLongInput(); "
                     "index %d\n", prec->name, i );
        *status = ERROR;
        return 0;
    }

    /* If type is not long, set status and return 0 */
    if ( ft[i] != DBF_LONG ) {
        epicsPrintf( "%s.FT%s should be LONG\n", prec->name, alpha[i] );
        *status = ERROR;
        return 0;
    }

    /* Otherwise, return value */
    else {
        return * ( long * ) val[i];
    }
}


/*+*********************************************************************
  Function:     genSubLongInputPtr()

  Author:       P. Gurd, after W. Lupton

  Abstract:     Return the address of a genSub long input field

  Description:  The supplied field address should be the address of one of
                the A-U fields. This is validated, the type is confirmed to
                be long, and the value is returned as the function value
  *********************************************************************/

long *                          /* (<) address of long value, NULL on error  */
genSubLongInputPtr(
    genSubRecord *prec,         /* (>) address of genSub record */
    void **field,               /* (>) address of A-U field */
    long *status )              /* (<) modified status, set to ERROR on error */
{
    /* Field index (0=A, 1=B etc) */
    int i = field - &prec->a;

    /* Value and field type arrays */
    void           **val = &prec->a;
    unsigned short  *ft  = &prec->fta;

    /* If index is invalid, set status and return 0.0 */
    if ( i < 0 || i >= NFIELD ) {
        epicsPrintf( "%s bad field address passed to genSubLongInput(); "
                     "index %d\n", prec->name, i );
        *status = ERROR;
        return NULL;
    }

    /* If type is not long, set status and return 0 */
    if ( ft[i] != DBF_LONG ) {
        epicsPrintf( "%s.FT%s should be LONG\n", prec->name, alpha[i] );
        *status = ERROR;
        return NULL;
    }

    /* Otherwise, return value */
    else {
        return ( long * ) val[i];
    }
}

/*+*********************************************************************
  Function:     genSubLongOutput()

  Author:       W. Lupton

  Abstract:     Return the address of a genSub long output field

  Description:  The supplied field address should be the address of one of
                the VALA-VALU fields. This is validated, the type is confirmed
                to be long, and the address of the value is returned as the
                function value
  *********************************************************************/

long *                          /* (<) address of long value, NULL on error */
genSubLongOutput(
    genSubRecord *prec,         /* (>) address of genSub record */
    void **field,               /* (>) address of VALA-VALU field */
    long *status )              /* (<) modified status, set to ERROR on error */
{
    /* Field index (0=VALA, 1=VALB etc) */
    int i = field - &prec->vala;

    /* Value and field type arrays */
    void           **val = &prec->vala;
    unsigned short  *ftv = &prec->ftva;

    /* If index is invalid, set status and return NULL */
    if ( i < 0 || i >= NFIELD ) {
        epicsPrintf( "%s bad field address passed to genSubLongOutput(); "
                     "index %d\n", prec->name, i );
        *status = ERROR;
        return NULL;
    }

    /* If type is not long, set status and return NULL */
    if ( ftv[i] != DBF_LONG ) {
        epicsPrintf( "%s.FTV%s should be LONG\n", prec->name, alpha[i] );
        *status = ERROR;
        return NULL;
    }

    /* Otherwise, return pointer in which to place value */
    else {
        return ( long * ) val[i];
    }
}

/*+*********************************************************************
  Function:     genSubDoubleInput()

  Author:       W. Lupton

  Abstract:     Return the value of a genSub double precision input field

  Description:  The supplied field address should be the address of one of
                the A-U fields. This is validated, the type is confirmed to
                be double, and the value is returned as the function value
  *********************************************************************/

double                          /* (<) double value, 0.0 on error */
genSubDoubleInput(
    genSubRecord *prec,         /* (>) address of genSub record */
    void **field,               /* (>) address of A-U field */
    long *status )              /* (<) modified status, set to ERROR on error */
{
    /* Field index (0=A, 1=B etc) */
    int i = field - &prec->a;

    /* Value and field type arrays */
    void           **val = &prec->a;
    unsigned short  *ft  = &prec->fta;

    /* If index is invalid, set status and return 0.0 */
    if ( i < 0 || i >= NFIELD ) {
        epicsPrintf( "%s bad field address passed to genSubDoubleInput(); "
                     "index %d\n", prec->name, i );
        *status = ERROR;
        return 0.0;
    }

    /* If type is not double, set status and return 0.0 */
    if ( ft[i] != DBF_DOUBLE ) {
        epicsPrintf( "%s.FT%s should be DOUBLE\n", prec->name, alpha[i] );
        *status = ERROR;
        return 0.0;
    }

    /* Otherwise, return value */
    else {
        return * ( double * ) val[i];
    }
}

/*+*********************************************************************
  Function:     genSubDoubleInputPtr()

  Author:       P. Gurd after W. Lupton

  Abstract:     Return the address of a genSub double precision input field

  Description:  The supplied field address should be the address of one of
                the A-U fields. This is validated, the type is confirmed to
                be double, and the value is returned as the function value
  *********************************************************************/

double *                        /* (<) address of double value, NULL on error */
genSubDoubleInputPtr(
    genSubRecord *prec,         /* (>) address of genSub record */
    void **field,               /* (>) address of A-U field */
    long *status )              /* (<) modified status, set to ERROR on error */
{
    /* Field index (0=A, 1=B etc) */
    int i = field - &prec->a;

    /* Value and field type arrays */
    void           **val = &prec->a;
    unsigned short  *ft  = &prec->fta;

    /* If index is invalid, set status and return 0.0 */
    if ( i < 0 || i >= NFIELD ) {
        epicsPrintf( "%s bad field address passed to genSubDoubleInput(); "
                     "index %d\n", prec->name, i );
        *status = ERROR;
        return NULL;
    }

    /* If type is not double, set status and return 0.0 */
    if ( ft[i] != DBF_DOUBLE ) {
        epicsPrintf( "%s.FT%s should be DOUBLE\n", prec->name, alpha[i] );
        *status = ERROR;
        return NULL;
    }

    /* Otherwise, return pointer */
    else {
        return ( double * ) val[i];
    }
}

/*+*********************************************************************
  Function:     genSubDoubleOutput()

  Author:       W. Lupton

  Abstract:     Return the address of a genSub double precision output field

  Description:  The supplied field address should be the address of one of
                the VALA-VALU fields. This is validated, the type is confirmed
                to be double, and the address of the value is returned as the
                function value
  *********************************************************************/

double *                        /* (<) address of double value, NULL on error */
genSubDoubleOutput(
    genSubRecord *prec,         /* (>) address of genSub record */
    void **field,               /* (>) address of VALA-VALU field */
    long *status )              /* (<) modified status, set to ERROR on error */
{
    /* Field index (0=VALA, 1=VALB etc) */
    int i = field - &prec->vala;

    /* Value and field type arrays */
    void           **val = &prec->vala;
    unsigned short  *ftv = &prec->ftva;

    /* If index is invalid, set status and return NULL */
    if ( i < 0 || i >= NFIELD ) {
        epicsPrintf( "%s bad field address passed to genSubDoubleOutput(); "
                     "index %d\n", prec->name, i );
        *status = ERROR;
        return NULL;
    }

    /* If type is not double, set status and return NULL */
    if ( ftv[i] != DBF_DOUBLE ) {
        epicsPrintf( "%s.FTV%s should be DOUBLE\n", prec->name, alpha[i] );
        *status = ERROR;
        return NULL;
    }

    /* Otherwise, return pointer in which to place value */
    else {
        return ( double * ) val[i];
    }
}

/*+*********************************************************************
  Function:     genSubStringInput()

  Author:       W. Lupton

  Abstract:     Return the value of a genSub string input field

  Description:  The supplied field address should be the address of one of
                the A-U fields. This is validated, the type is confirmed to
                be string, and the value is returned as the function value
  *********************************************************************/

char *                          /* (<) address of string, NULL on error */
genSubStringInput(
    genSubRecord *prec,         /* (>) address of genSub record */
    void **field,               /* (>) address of A-U field */
    long *status )              /* (<) modified status, set to ERROR on error */
{
    /* Field index (0=A, 1=B etc) */
    int i = field - &prec->a;

    /* Value and field type arrays */
    void           **val = &prec->a;
    unsigned short  *ft  = &prec->fta;

    /* If index is invalid, set status and return NULL */
    if ( i < 0 || i >= NFIELD ) {
        epicsPrintf( "%s bad field address passed to genSubDoubleInput(); "
                     "index %d\n", prec->name, i );
        *status = ERROR;
        return NULL;
    }

    /* If type is not string, set status and return NULL */
    if ( ft[i] != DBF_STRING ) {
        epicsPrintf( "%s.FT%s should be STRING\n", prec->name, alpha[i] );
        *status = ERROR;
        return NULL;
    }

    /* Otherwise, return value */
    else {
        return ( char * ) val[i];
    }
}

/*+*********************************************************************
  Function:     genSubStringOutput()

  Author:       W. Lupton

  Abstract:     Return the address of a genSub string output field

  Description:  The supplied field address should be the address of one of
                the VALA-VALU fields. This is validated, the type is confirmed
                to be string, and the address of the value is returned as the
                function value
  *********************************************************************/

char *                          /* (<) address of string value, NULL on error */
genSubStringOutput(
    genSubRecord *prec,         /* (>) address of genSub record */
    void **field,               /* (>) address of VALA-VALU field */
    long *status )              /* (<) modified status, set to ERROR on error */
{
    /* Field index (0=VALA, 1=VALB etc) */
    int i = field - &prec->vala;

    /* Value and field type arrays */
    void           **val = &prec->vala;
    unsigned short  *ftv = &prec->ftva;

    /* If index is invalid, set status and return NULL */
    if ( i < 0 || i >= NFIELD ) {
        epicsPrintf( "%s bad field address passed to genSubStringOutput(); "
                     "index %d\n", prec->name, i );
        *status = ERROR;
        return NULL;
    }

    /* If type is not string, set status and return NULL */
    if ( ftv[i] != DBF_STRING ) {
        epicsPrintf( "%s.FTV%s should be STRING\n", prec->name, alpha[i] );
        *status = ERROR;
        return NULL;
    }

    /* Otherwise, return pointer in which to place value */
    else {
        return ( char * ) val[i];
    }
}

/*+*********************************************************************
  Function:     genSubStructInput()

  Author:       W. Lupton

  Abstract:     Return the address of a genSub structured input field

  Description:  The supplied field address should be the address of one of
                the A-U fields. This is validated, the type is confirmed to
                be char, the field length is checked, and the address of the
                value is returned as the function value
  *********************************************************************/

void *                          /* (<) address of structure, NULL on error */
genSubStructInput(
    genSubRecord *prec,         /* (>) address of genSub record */
    size_t size,                /* (>) size (in bytes) of structure */
    void **field,               /* (>) address of A-U field */
    long *status )              /* (<) modified status, set to ERROR on error */
{
    /* Field index (0=A, 1=B etc) */
    int i = field - &prec->a;

    /* Value, field type and number of element arrays */
    void           **val = &prec->a;
    unsigned short  *ft  = &prec->fta;
    unsigned long   *no  = &prec->noa;

    /* If index is invalid, set status and return NULL */
    if ( i < 0 || i >= NFIELD ) {
        epicsPrintf( "%s bad field address passed to genSubStructInput(); "
                     "index %d\n", prec->name, i );
        *status = ERROR;
        return NULL;
    }

    /* If type is not char, set status and return NULL */
    if ( ft[i] != DBF_CHAR ) {
        epicsPrintf( "%s.FT%s should be CHAR\n", prec->name, alpha[i] );
        *status = ERROR;
        return NULL;
    }

    /* If number of elements is too small, set status and return NULL */
    else if ( no[i] < size ) {
        epicsPrintf( "%s.NO%s only %d bytes; should be %d\n", prec->name,
                     alpha[i], no[i], size );
        *status = ERROR;
        return NULL;
    }

    /* Otherwise, return value */
    else {
        return val[i];
    }
}

/*+*********************************************************************
  Function:     genSubStructOutput()

  Author:       W. Lupton

  Abstract:     Return the address of a genSub structured input field

  Description:  The supplied field address should be the address of one of
                the VALA-VALU fields. This is validated, the type is confirmed
                to be char, the field length is checked, and the address of the
                value is returned as the function value
  *********************************************************************/

void *                          /* (<) address of structure, NULL on error */
genSubStructOutput(
    genSubRecord *prec,         /* (>) address of genSub record */
    size_t size,                /* (>) size (in bytes) of structure */
    void **field,               /* (>) address of VALA-VALU field */
    long *status )              /* (<) modified status, set to ERROR on error */
{
    /* Field index (0=VALA, 1=VALB etc) */
    int i = field - &prec->vala;

    /* Value, field type and number of element arrays */
    void **val = &prec->vala;
    unsigned short *ftv = &prec->ftva;
    unsigned long  *nov = &prec->nova;

    /* If index is invalid, set status and return NULL */
    if ( i < 0 || i >= NFIELD ) {
        epicsPrintf( "%s bad field address passed to genSubStructOutput(); "
                     "index %d\n", prec->name, i );
        *status = ERROR;
        return NULL;
    }

    /* If type is not char, set status and return NULL */
    if ( ftv[i] != DBF_CHAR ) {
        epicsPrintf( "%s.FTV%s should be DOUBLE\n", prec->name, alpha[i] );
        *status = ERROR;
        return NULL;
    }

    /* If number of elements is too small, set status and return NULL */
    else if ( nov[i] < size ) {
        epicsPrintf( "%s.NOV%s only %d bytes; should be %d\n", prec->name,
                     alpha[i], nov[i], size );
        *status = ERROR;
        return NULL;
    }

    /* Otherwise, return pointer in which to place value */
    else {
        return val[i];
    }
}

/*+*********************************************************************
* $Log: genSubUtil.c,v $
* Revision 1.1  2000/09/20 14:37:15  peregrin
* First checkin for v2_1_0.
*
* Revision 1.1  1999/04/08 15:58:39  gurd
* adding some InputPtr routines - adding this to share
*
* Revision 1.2  1998/09/23 23:02:40  gurd
* Add genSubLongInputPtr and genSubDoubleInputPtr routines
*
* Revision 1.1  1998/07/24 17:51:53  wright
* added support for gensub
*
* Revision 1.3  1998/02/06 08:25:34  ktsubota
* Initial EPICS R3.13 conversion
*
* Revision 1.2.2.2  1998/01/20 20:05:50  ktsubota
* Misc updates
*
* Revision 1.2.2.1  1997/10/15 01:17:31  wlupton
* R3.13 version from Andy Foster
*
* Revision 1.2  1997/07/10 22:55:14  ktsubota
* Added inclusion of stdio.h
*
* Revision 1.1  1997/06/27 20:26:23  wlupton
* initial insertion
*
***********************************************************************/

