#include "ip470.h"

#define C_MASK  (unsigned)0xFFFFFC00  /* Forms carrier base address from IP address */

/*
{+D}
    FILENAME:           carrier.c

    MODULE NAME:        carrier.c

    VERSION:            A

    CREATION DATE:      01/11/95

    CODED BY:           FJM

    ABSTRACT:           This file contains the common carrier board data
			structures.

    CALLING SEQUENCE:

    MODULE TYPE:

    I/O RESOURCES:

    SYSTEM RESOURCES:

    MODULES CALLED:

    REVISIONS:

      DATE      BY      PURPOSE
    --------   -----    ---------------------------------------------------

{-D}
*/

/*
    MODULES FUNCTION DETAILS

    This file contains the common data structures
*/

/*
    Board Status Register bit positions
*/

#define GLOBAL_EN       8       /* global interrupt enable bit position */

/*
    Defined below is the memory map template for the AVME96x0 Board.
    This structure provides access to the various registers on the board.
*/

struct map96x0                  /* Memory map of the board */
{
    byte ip_a_io[128];          /* IPA 128 bytes I/O space */
    struct ip_a_id              /* IPA Module ID PROM */
    {
	char unused1;           /* undefined */
	char prom_a;            /* IPA id information */
    } id_map_a[32];

    byte unused2;               /* undefined */
    byte sts_reg;               /* Status Register */

    byte unused3;               /* undefined */
    byte lev_reg;               /* Interrupt Level Register */

    byte unused4;               /* undefined */
    byte err_reg;               /* Error Register */

    byte unused5;               /* undefined */
    byte mem_en_reg;            /* Memory Enable Register */

    byte unused6[9];            /* undefined */
    byte ipambasr;              /* IPA memory base addr & size register */

    byte unused7;               /* undefined */
    byte ipbmbasr;              /* IPB memory base addr & size register */

    byte unused8;               /* undefined */
    byte ipcmbasr;              /* IPC memory base addr & size register */

    byte unused9;               /* undefined */
    byte ipdmbasr;              /* IPD memory base addr & size register */

    byte unused10[9];           /* undefined */
    byte en_reg;                /* Interrupt Enable Register */

    byte unused11;              /* undefined */
    byte pnd_reg;               /* Interrupt Pending Register */

    byte unused12;              /* undefined */
    byte clr_reg;               /* Interrupt Clear Register */

    byte unused13[26];          /* undefined */

    byte ip_b_io[128];          /* IPB 128 bytes I/O space */
    struct ip_b_id              /* IPB Module ID PROM */
    {
	char unused14;          /* undefined */
	char prom_b;            /* IPB id information */
    } id_map_b[32];

    byte unused15[64];          /* undefined */

    byte ip_c_io[128];          /* IPC 128 bytes I/O space */
    struct ip_c_id              /* IPC Module ID PROM */
    {
	char unused16;          /* undefined */
	char prom_c;            /* IPC id information */
    } id_map_c[32];

    byte unused17[64];          /* undefined */

    byte ip_d_io[128];          /* IPD 128 bytes I/O space */
    struct ip_d_id              /* IPD Module ID PROM */
    {
	char unused18;          /* undefined */
	char prom_d;            /* IPD id information */
    } id_map_d[32];

    byte unused19[64];          /* undefined */
} ;



void carrier(map_ptr)

struct map96x0 *map_ptr;    /* pointer to board memory map */

{
    unsigned char* reg;
/*
    ENTRY POINT OF ROUTINE:
*/

    for( reg = (unsigned char*) 0xFFFBC010;
         reg <= (unsigned char*) 0xFFFBC016;
         reg += 2 )
      *reg = *reg & 0xE8  |  0x11;   /* select lowest level and enable */
}



/*
{+D}
   SYSTEM:          Acromag I/O Board

   MODULE NAME:     cos_isr.c - interrupt handler

   VERSION:         A

   CREATION DATE:  10/30/95

   CODED BY:        FM

   ABSTRACT:        These routines perform interrupt exception handling
		    for interrupts on the IP470 I/O board.

   CALLING
	SEQUENCE:   This subroutine runs as a result of an interrupt occuring.
		    When called, OS-9 provides in 680XX register a2 a pointer
		    to the interrupt handler data area of the associated
		    parent process.

   MODULE TYPE:    n/a

   I/O RESOURCES:  user specific

   SYSTEM
	RESOURCES:

   MODULES
	CALLED:     user specific

   REVISIONS:

 DATE      BY       PURPOSE
-------- ----  ------------------------------------------------

{-D}
*/

/*
   MODULES FUNCTIONAL DETAILS:

   For change of state operation input bits 0 & 4, 1 & 5, 2 & 6, 3 & 7 
   of a port must be connected together via a wire jumper at the 
   termination panel. 
   
   The polarity of input bits 0, 1, 2, 3 is controlled by a single bit in 
   the Event Control Register.    If this bit is a '1' positive edeges are
   sensed.  The polarity of bits 4, 5, 6, 7 is controlled by a different 
   bit in the Event Control Register.  If this bit is a '0' negative edeges 
   are  sensed.
   
   Each time an input changes states it will be sensed by either negative or
   positive edge sense programmed into the Event Control Register.
*/


void cos_isr(cblk)

struct cblk470 *cblk;            /* pointer to interrupting IO port */

{

/*
	External data areas
*/

/*
	Local data areas
*/

struct map96x0 *carrier;        /* pointer to carrier base address */
int h, i, j;                    /* loop control */
byte saved_bank;                /* saved bank value */
byte i_stat;                    /* interrupt status */
byte p_mask;                    /* port interrupt bit mask */
byte i_pend;                    /* interrupting bit */
byte b_mask;                    /* bit interrupt bit mask */
byte mbit;                      /* event control mask */
int cos_bit;                    /* COS bit number 0-23 */
int state;                      /* state of changed bit */

/*
    DECLARE MODULES CALLED:
*/
   
    byte input();            /* read byte routine */
    byte output();           /* write byte routine */
    byte bank_select();      /* select new bank on ip470 */

/*
	Entry point of routine:
*/

    saved_bank = bank_select(BANK1, cblk); /* set & save bank select */
        
    for(i = 0; i < 6; i++)      /* check ports 0-5 for an interrupt pending */
    {
      i_stat = input((unsigned)&cblk->brd_ptr->port[6].b_select); /* interrupt status */
      p_mask = ( 1 << i);       /* form port interrupt bit mask */
      if((p_mask & i_stat) != 0) /* port with interrupt pending? */
      {
	for(j = 0; j < 8; j++)  /* check each bit in an interrupting port */
	{   
          i_pend = input((unsigned)&cblk->brd_ptr->port[i].b_select); /* interrupt sense */
	  b_mask = ( 1 << j);   /* form bit interrupt mask */
	  if((b_mask & i_pend) != 0) /* bit in port with interrupt pending? */
	  {  
            output((unsigned)&cblk->brd_ptr->port[i].b_select,(~b_mask)); /* write 0 to clear the interrupting bit */

/*        
   At this time the interrupting port and bit is known.
   The port number (0-5) is in 'i' and the bit number (0-7) is in 'j'.

   The following code converts from port:bit format to change of state
   format bit number:state (0 thru 23:0 or 1).
*/

	    cos_bit = (i << 2) + j;      /* compute COS bit number */   
	    if(i > 3)                    /* port number > 3 */
	    {
	      mbit = (1 << ((i - 4) << 1)); /* generate event control mask bit */
	      h = 1;                     /* index to ev_control array */   
	    }
	    else                         /* port number <= 3 */
	    {
	      mbit = (1 << (i << 1));    /* generate event control mask bit */
	      h = 0;                     /* index to ev_control array */
	    }
	    if(j > 3)                    /* correct for nibble encoding */
	    {
	      mbit <<= 1;
	      cos_bit -= 4;              /* correct COS bit number */
	    }
            if((cblk->ev_control[h] & mbit) != 0) /* state 0 or 1 */
              state = 1;
	    else
              state = 0;

/*
   Save the change of state bit number and the state value.
   The user may view them from the status menu.
*/

            cblk->last_chan = cos_bit;   /* correct channel number */
            cblk->last_state = state;    /* correct state for channel */

/*
   User code can be added here to complete the interrupt service routine

	   Example:   user_function(cos_bit,state);
*/
	  }
	}
        output((unsigned)&cblk->brd_ptr->port[i].b_select, 0xFF); /* re-enable sense inputs */ 
      }
    }

    bank_select(saved_bank, cblk); /* restore bank select */
}


/*
{+D}
   SYSTEM:         Acromag IP470 I/O Board

   MODULE NAME:    cos - interrupt service routine

   VERSION:        A

   CREATION DATE:  10/30/95

   CODED BY:       FJM

   ABSTRACT:       These routines perform change of state interrupt exception 
		   handling for interrupts from the IP470 I/O board.

   CALLING
       SEQUENCE:   This subroutine runs as a result of an interrupt
		   occuring.  When called, OS-9 provides in 680XX
		   register a2 a pointer to the interrupt handler data
		   area of the associated parent process.

   MODULE TYPE:    n/a

   I/O RESOURCES:  user specific

   SYSTEM
       RESOURCES:

   MODULES
       CALLED:     user specific

   REVISIONS:

 DATE     BY       PURPOSE
-------- ----  ------------------------------------------------

{-D}
*/

/*
   MODULES FUNCTIONAL DETAILS:

   Upon entry to the interrupt handler routine, OS-9 sets the a2 register
   to point to a global data area. The user specifies the location of this
   data area when the interrupt handler is initially attached to the vector
   number.  For this routine, a2 is presumed to point to a data structure
   of type "handler_data".

   The hd_ptr member of handler_data points to "c_blk" data
   structure that identifies the interrupt source and places parameters
   needed by the interrupt service routine in register d0.

   The following diagram summarizes the structure relationships.


          hdata                          c_blk
          (struct handler_data)          (struct c_blk)
 OS-9
 (a2)       +-----------+     (a0)       +----------+ (d0)
 ------>  0 |   h_pid   |        ----> 0 |  cblk470 | -----
            +-----------+       |        +----------+      |
          4 |  hd_ptr   | ------                           |
            +-----------+                                  |
							   |
    -------------------------------------------------------
   |
   |         (struct map470)
   |         +-------------+
    -----> 0 |             |
	   | /             /
	   V /             /
	  XXX|             |
	     +-------------+
*/

void cos( hdata )
   struct handler_data* hdata; {
   cos_isr( &hdata->hd_ptr );
   return;
   }

/*
{+D}
   SYSTEM:          Acromag I/O Board

   MODULE NAME:     lev_isr.c - interrupt handler

   VERSION:         A

   CREATION DATE:  10/30/95

   CODED BY:        FM

   ABSTRACT:        These routines perform interrupt exception handling
		    for interrupts on the IP470 I/O board.

   CALLING
	SEQUENCE:   This subroutine runs as a result of an interrupt occuring.
		    When called, OS-9 provides in 680XX register a2 a pointer
		    to the interrupt handler data area of the associated
		    parent process.

   MODULE TYPE:    n/a

   I/O RESOURCES:  user specific

   SYSTEM
	RESOURCES:

   MODULES
	CALLED:     user specific

   REVISIONS:

 DATE      BY       PURPOSE
-------- ----  ------------------------------------------------

{-D}
*/

/*
   MODULES FUNCTIONAL DETAILS:

   For level operation, each input bit must be connected to a switch input. 
   
   The polarity of input bits is controlled in nibbles by a single bit in 
   the Event Control Register.    If this bit is a '1' positive edeges are
   sensed.  If this bit is a '0' negative edeges are sensed.
   
   Each time an input changes to the state programmed into the sense register
   an interrupt is generated.
*/


void lev_isr(cblk)

struct cblk470 *cblk;            /* pointer to interrupting IO port */

{

/*
	External data areas
*/

/*
	Local data areas
*/

struct map96x0 *carrier;        /* pointer to carrier base address */
int h, i, j;                    /* loop control */
byte saved_bank;                /* saved bank value */
byte i_stat;                    /* interrupt status */
byte p_mask;                    /* port interrupt bit mask */
byte i_pend;                    /* interrupting bit */
byte b_mask;                    /* bit interrupt bit mask */
byte mbit;                      /* event control mask */
int lev_bit;                    /* LEV bit number 0-23 */
int state;                      /* state of changed bit */

/*
    DECLARE MODULES CALLED:
*/
   
    byte input();            /* read byte routine */
    byte output();           /* write byte routine */
    byte bank_select();      /* select new bank on ip470 */

/*
	Entry point of routine:
*/

    saved_bank = bank_select(BANK1, cblk); /* set & save bank select */
        
    for(i = 0; i < 6; i++)      /* check ports 0-5 for an interrupt pending */
    {
      i_stat = input((unsigned)&cblk->brd_ptr->port[6].b_select); /* interrupt status */
      p_mask = ( 1 << i);       /* form port interrupt bit mask */
      if((p_mask & i_stat) != 0) /* port with interrupt pending? */
      {
	for(j = 0; j < 8; j++)  /* check each bit in an interrupting port */
	{   
          i_pend = input((unsigned)&cblk->brd_ptr->port[i].b_select); /* interrupt sense */
	  b_mask = ( 1 << j);   /* form bit interrupt mask */
	  if((b_mask & i_pend) != 0) /* bit in port with interrupt pending? */
	  {  
            output((unsigned)&cblk->brd_ptr->port[i].b_select,(~b_mask)); /* write 0 to clear the interrupting bit */

/*        
   At this time the interrupting port and bit is known.
   The port number (0-5) is in 'i' and the bit number (0-7) is in 'j'.

   The following code converts from port:bit format to level
   format bit number:state (0 thru 47:0 or 1).
*/

            lev_bit = i * 8 + j;        /* compute bit number */   
            if(i > 3)                   /* port number > 3 */
	    {
	      mbit = (1 << ((i - 4) << 1)); /* generate event control mask bit */
	      h = 1;                     /* index to ev_control array */   
	    }
	    else                         /* port number <= 3 */
	    {
	      mbit = (1 << (i << 1));    /* generate event control mask bit */
	      h = 0;                     /* index to ev_control array */
	    }
	    if(j > 3)                    /* correct for nibble encoding */
              mbit <<= 1;

            if((cblk->ev_control[h] & mbit) != 0) /* state 0 or 1 */
              state = 1;
	    else
              state = 0;

/*
   Save the bit number and the state value.
   The user may view them from the status menu.
*/

            cblk->last_chan = lev_bit;   /* correct channel number */
            cblk->last_state = state;    /* correct state for channel */

/*
   User code can be added here to complete the interrupt service routine

           Example:   user_function(lev_bit,state);
*/
	  }
	}
        output((unsigned)&cblk->brd_ptr->port[i].b_select, 0xFF); /* re-enable sense inputs */ 
      }
    }

    bank_select(saved_bank, cblk); /* restore bank select */
}




/*
{+D}
   SYSTEM:         Acromag IP470 I/O Board

   MODULE NAME:    lev - interrupt service routine

   VERSION:        A

   CREATION DATE:  10/30/95

   CODED BY:       FJM

   ABSTRACT:       These routines perform interrupt exception 
		   handling for interrupts from the IP470 I/O board.

   CALLING
       SEQUENCE:   This subroutine runs as a result of an interrupt
		   occuring.  When called, OS-9 provides in 680XX
		   register a2 a pointer to the interrupt handler data
		   area of the associated parent process.

   MODULE TYPE:    n/a

   I/O RESOURCES:  user specific

   SYSTEM
       RESOURCES:

   MODULES
       CALLED:     user specific

   REVISIONS:

 DATE     BY       PURPOSE
-------- ----  ------------------------------------------------

{-D}
*/

/*
   MODULES FUNCTIONAL DETAILS:

   Upon entry to the interrupt handler routine, OS-9 sets the a2 register
   to point to a global data area. The user specifies the location of this
   data area when the interrupt handler is initially attached to the vector
   number.  For this routine, a2 is presumed to point to a data structure
   of type "handler_data".

   The hd_ptr member of handler_data points to "c_blk" data
   structure that identifies the interrupt source and places parameters
   needed by the interrupt service routine in register d0.

   The following diagram summarizes the structure relationships.


          hdata                          c_blk
          (struct handler_data)          (struct c_blk)
 OS-9
 (a2)       +-----------+     (a0)       +----------+ (d0)    
 ------>  0 |   h_pid   |        ----> 0 |  cblk470 | -----
	    +-----------+       |        +----------+      |
          4 |  hd_ptr   | ------                           |
            +-----------+                                  |
							   |
    -------------------------------------------------------
   |
   |         (struct map470)
   |         +-------------+
    -----> 0 |             |
	   | /             /
	   V /             /
	  XXX|             |
	     +-------------+
*/

void lev( hdata )
   struct handler_data* hdata; {
   lev_isr( &hdata->hd_ptr );
   return;
   }

/*
{+D}
    SYSTEM:         Library Software

    MODULE NAME:    bank_select - select bank on 470 board

    VERSION:        A

    CREATION DATE:  10/18/95

    CODED BY:       FJM

    ABSTRACT:       This module is used to select register banks on 
		    the IP470 board.

    CALLING
	SEQUENCE:   old_bank = bank_select(new_bank, c_blk);
		    where:
			cblk (pointer to structure)
			    Pointer to the configuration block structure.
			new_bank (byte)
			    New bank number 00, 01, 10 only
			old_bank (byte)
			    Old bank number 00, 01, 10 only
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
    
    This module is used to select register banks on the IP470 board.
*/



byte bank_select(new_bank, c_blk)

byte new_bank;
struct cblk470 *c_blk;

{

/*
    declare local storage
*/

    byte old_bank;              /* old bank number */
    byte bank_bits;             /* bank select info */

/*
    DECLARE MODULES CALLED:
*/
   
    byte input();            /* read byte routine */
    byte output();           /* write byte routine */

/*
    ENTRY POINT OF ROUTINE:
*/

   bank_bits = input((unsigned)&c_blk->brd_ptr->port[7].b_select); /* get current */
   old_bank = ((bank_bits & 0xC0) >> 6);/* isolate bank select bits */
   
   if(old_bank == new_bank)             /* same bank ? */
     return(old_bank);                  /* no need to change bits */
   
   if(old_bank == BANK1)                /* special treatment required ? */
     bank_bits = c_blk->ev_control[1];  /* yes must use ev_control bits */

   bank_bits &= 0x3F;                   /* save all but bank select bits */
   bank_bits |= (new_bank << 6);        /* OR in new bank bits */

   output((unsigned)&c_blk->brd_ptr->port[7].b_select, bank_bits);
   return(old_bank);
}

/*
{+D}
	SYSTEM:         Library Software

	MODULE NAME:    output.c

	VERSION:        V1.0

	CREATION DATE:  03/27/95

	DESIGNED BY:    FJM

	CODED BY:       FJM

	ABSTRACT:       Low level interface to write byte routine.

	CALLING
	  SEQUENCE:     byte = output(addr,b);
			Where
				addr (pointer*) to unsigned I/O port address
				b    (int) character to write
	MODULE TYPE:

	I/O
	  RESOURCES:    None

	SYSTEM
	  RESOURCES:    None

	MODULES
	  CALLED:

	REVISIONS:

  DATE      BY     PURPOSE
---------  ----   -------------------------------------------------------

{-D}
*/

/*
   MODULES FUNCTIONAL DETAILS:
*/


byte output(addr,b)

unsigned *addr;
int b;

{
  
/*
	DECLARE MODULES CALLED:
*/

/*
	Entry point of routine
*/

  *((char *)addr) = (char)b; 
  return(0);
}


/*
{+D}
	SYSTEM:         Library Port Software

	MODULE NAME:    input.c

	VERSION:        V1.0

	CREATION DATE:  03/27/95

	DESIGNED BY:    FJM

	CODED BY:       FJM

	ABSTRACT:       Low level interface to read byte routine.

	CALLING
	  SEQUENCE:     return = input(addr);
			Where
				return (byte) character read
				addr (pointer*) to unsigned I/O port address
	MODULE TYPE:

	I/O
	  RESOURCES:    None

	SYSTEM
	  RESOURCES:    None

	MODULES
	  CALLED:

	REVISIONS:

  DATE      BY     PURPOSE
---------  ----   -------------------------------------------------------

{-D}
*/

/*
   MODULES FUNCTIONAL DETAILS:
*/


byte input(addr)

unsigned *addr;

{

/*
	DECLARE MODULES CALLED:
*/

/*
	Entry point of routine
*/

  return((byte) *((char *)addr));
}

