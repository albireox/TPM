#include "ip470.h"

/*
{+D}
    SYSTEM:         Software

    FILENAME:       cnfg470.c

    MODULE NAME:    cnfg470 - configure 470 board

    VERSION:        A

    CREATION DATE:  10/18/95

    CODED BY:       FJM

    ABSTRACT:       This module is used to perform the configure function
		    for the IP470 board.

    CALLING
	SEQUENCE:   cnfg470(ptr);
		    where:
			ptr (pointer to structure)
			    Pointer to the configuration block structure.

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

    This module is used to perform the "configure board" function for 
    the IP470 board.  A pointer to the Configuration Block will be passed 
    to this routine.  The routine will use a pointer within the 
    Configuration Block to reference the registers on the Board.  
    Based on flag bits in the Attribute and Parameter Flag words in the 
    Configuration Block, the board will be configured and various 
    registers will be updated with new information which will be 
    transfered from the Configuration Block to registers on the Board.
*/



void cnfg470(c_blk)

struct cblk470 *c_blk;

{

/*
    declare local storage
*/

   int i;

/*
    DECLARE MODULES CALLED:
*/

   byte bank_select();      /* bank switching function */
   byte input();            /* read byte routine */
   byte output();           /* write byte routine */

/*
    ENTRY POINT OF ROUTINE:
    If the I/O device is to be reset then do so.
*/

   if((c_blk->param & RESET_INTEN) && (c_blk->enable & RESET))
      output((unsigned)&c_blk->brd_ptr->ier, RESET); /* set reset bit */

/*
    Check to see if the Interrupt Vector Register is to be updated.
    Update the Vector Register before enabling Global Interrupts so that
    the board will always output the correct vectors upon interrupt.
*/

   if(c_blk->param & VECT )
      output((unsigned)&c_blk->brd_ptr->ivr, c_blk->vector);

/*
    If in standard mode and the Mask Register is to be updated, then update it.
*/

   if((c_blk->e_mode == 0) && (c_blk->param & MASK))
   {
      bank_select(BANK0, c_blk);
      output((unsigned)&c_blk->brd_ptr->port[7].b_select, (c_blk->mask_reg & 0x3F));
   }

/*
    Enhanced mode
*/
    
   if((c_blk->param & ENHANCED) && (c_blk->e_mode != 0))
   {
     output((unsigned)&c_blk->brd_ptr->port[7].b_select, 0x07);
     output((unsigned)&c_blk->brd_ptr->port[7].b_select, 0x0D);
     output((unsigned)&c_blk->brd_ptr->port[7].b_select, 0x06);
     output((unsigned)&c_blk->brd_ptr->port[7].b_select, 0x12);
   }

   if(c_blk->e_mode != 0)       /* configure if enhanced mode flag is set */
   {

/*
    If the Mask Register is to be updated, then update it.
*/

     if(c_blk->param & MASK)
     {
       bank_select(BANK0, c_blk);
       output((unsigned)&c_blk->brd_ptr->port[7].b_select, (c_blk->mask_reg & 0x3F));
     }

/*
    If the Event Control Registers are to be updated . . .
*/

     if(c_blk->param & EVCONTROL)
     {
       /* Note: bank_select 1 writes the event sense polarity for R1,P7,B1 */
       bank_select(BANK1, c_blk);
       output((unsigned)&c_blk->brd_ptr->port[6].b_select, c_blk->ev_control[0]);
       output((unsigned)&c_blk->brd_ptr->port[7].b_select, 
		c_blk->ev_control[1]|(BANK1<<6));
     }

/*
    If the Debounce Control Register is to be updated, then update it.
*/

     if(c_blk->param & DEBCONTROL)
     {
       bank_select(BANK2, c_blk);
       output((unsigned)&c_blk->brd_ptr->port[0].b_select, c_blk->deb_control);
     }

/*
    If the Debounce Duration Register is to be updated, then update it.
*/

     if(c_blk->param & DEBDURATION)
     {
       bank_select(BANK2, c_blk);
       output((unsigned)&c_blk->brd_ptr->port[1].b_select, c_blk->deb_duration[0]);
       output((unsigned)&c_blk->brd_ptr->port[2].b_select, c_blk->deb_duration[1]);
     }

/*
    If the Debounce Clock Register is to be updated, then update it.
*/

     if(c_blk->param & DEBCLOCK)
     {
       bank_select(BANK2, c_blk);
       output((unsigned)&c_blk->brd_ptr->port[3].b_select, c_blk->deb_clock);
     }

/*
    If the Interrupt Enable Register is to be updated then do so.
*/

     if((c_blk->param & RESET_INTEN) && (c_blk->enable & INTEN))
     {
       bank_select(BANK1, c_blk);
       for(i = 0; i != 6; i++)
       {
         output((unsigned)&c_blk->brd_ptr->port[i].b_select, (byte) 0);
         output((unsigned)&c_blk->brd_ptr->port[i].b_select, 
		(byte)c_blk->int_enable[i]);
       }
       output((unsigned)&c_blk->brd_ptr->ier, INTEN);
     }
   }
}
