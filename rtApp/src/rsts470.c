#include "ip470.h"

/*
{+D}
    SYSTEM:         Library Software - ip470 Board

    FILENAME:       rsts470.c

    MODULE NAME:    rsts470 - read status of ip470 board

    VERSION:        A

    CREATION DATE:  10/16/95

    DESIGNED BY:    FJM

    CODED BY:       FJM
    
    ABSTRACT:       This module is used to read status of the ip470 board.

    CALLING
	SEQUENCE:   rsts470(c_blk);
		    where:
			c_blk (pointer to structure)
			    Pointer to the configuration structure.

    MODULE TYPE:    void

    I/O RESOURCES:  

    SYSTEM
	RESOURCES:  

    MODULES
	CALLED:     

    REVISIONS:      

  DATE    BY        PURPOSE
-------  ----   ------------------------------------------------

{-D}
*/


/*
    MODULES FUNCTIONAL DETAILS:

    This module is used to perform the read status function for the ip470
    board.  A pointer to the configuration structure will be passed to this 
    routine.  The routine will use a pointer together with offsets to 
    reference the registers on the Board and will transfer the status 
    information from the configuration structure.
*/



void rsts470(c_blk)
struct cblk470 *c_blk;
{
/*
    declare local storage
*/

/*
    DECLARE MODULES CALLED:
*/

   byte bank_select();      /* bank switching function */
   byte input();            /* read byte routine */

/*
    ENTRY POINT OF ROUTINE
*/

    bank_select(BANK1, c_blk);      /* select I/O bank */
    c_blk->enable = input((unsigned)&c_blk->brd_ptr->ier); /* interrupt enable status */
    c_blk->vector = input((unsigned)&c_blk->brd_ptr->ivr); /* interrupt vector */
}



/*
{+D}
    SYSTEM:         Library Software - ip470 Board

    FILENAME:       rmid470.c

    MODULE NAME:    rsts470 - read ID of ip470 board

    VERSION:        V1.0

    CREATION DATE:  10/16/95

    DESIGNED BY:    F.J.M

    CODED BY:       F.J.M.
    
    ABSTRACT:       This module is used to read status of the ip470 board.

    CALLING
	SEQUENCE:   rmid470(c_blk);
		    where:
			c_blk (pointer to structure)
			    Pointer to the configuration structure.

    MODULE TYPE:    void

    I/O RESOURCES:  

    SYSTEM
	RESOURCES:  

    MODULES
	CALLED:     

    REVISIONS:      

  DATE    BY        PURPOSE
-------  ----   ------------------------------------------------

{-D}
*/


/*
    MODULES FUNCTIONAL DETAILS:

    This module is used to perform the read ID function for the ip470 board.  
    A pointer to the configuration structure will be passed to this routine.  
    The routine will use a pointer within the structure together with 
    offsets to reference the registers on the Board and will transfer the 
    status information from the Board to the configuration structure.
*/



void rmid470(c_blk)
struct cblk470 *c_blk;
{
/*
    declare local storage
*/

    byte *id_ptr;               /* pointer to board ID memory map */
    int i,j;                    /* loop index */

/*
    DECLARE MODULES CALLED:
*/
   byte input();            /* read byte routine */
   
/*
    ENTRY POINT OF ROUTINE
*/

    id_ptr = (byte *)c_blk->brd_ptr + 0x80;

/*
    read board id information
*/

    for(i = 0, j = 1; i < 32; i++, j += 2)
	c_blk->id_prom[i] = input((unsigned)&id_ptr[j]);
}
