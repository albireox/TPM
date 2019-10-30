/*
#define PCAPP 0          / * compiler flag for PC application * /
*/

#define VMEAPP 0         /* compiler flag for VME application */

/*
{+D}
    SYSTEM:         IP470 Software

    FILE NAME:      ip470.h

    VERSION:        V1.0

    CREATION DATE:  09/03/95

    DESIGNED BY:    FM

    CODED BY:       FM

    ABSTRACT:       This module contains the definitions and structures
		    used by the ip470 library.
    CALLING
	SEQUENCE:

    MODULE TYPE:    header file

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

    This module contains the definitions and structures used by
    the ip470 library.
*/

/*
    DEFINITIONS:
*/

typedef unsigned char bool;
typedef unsigned char byte;
typedef unsigned short word;
/*
*/
#define CLOSED  FALSE
#define OPEN    TRUE

#define BANK0   (byte)0
#define BANK1   (byte)1
#define BANK2   (byte)2

#define MAXPORTS 5
#define MAXBITS  7
#define POINTVAL 1

#define RESET 2                 /* bit position in int. enable reg. */
#define INTEN 1                 /* bit position in int. enable reg. */

/*
   INTERRUPT VECTOR
*/

#define VECTOR0      0xB0       /* interrupt vector for port 0 */

/*
    Parameter mask bit positions
*/

#define ENHANCED       1        /* enhanced mode configuration */
#define MASK           2        /* write mask register */
#define EVCONTROL      4        /* event control register */
#define DEBCLOCK       8        /* debounce clock register */
#define DEBCONTROL  0x10        /* debounce control register */
#define DEBDURATION 0x20        /* debounce duration register */
#define RESET_INTEN 0x40        /* interrupt enable register */
#define VECT        0x80        /* interrupt vector register */


/*
   DATA STRUCTURES
*/

struct map470
{
  struct
   {
#ifdef VMEAPP           /* needed for VME applications */
     byte nu0;                  
#endif     
	/* common to all banks */
      
	byte b_select;          /* bank select register */
      
#ifdef PCAPP            /* needed for PC applications */
     byte nu0;                  
#endif
   } port[8];
#ifdef VMEAPP           /* needed for VME applications */
   byte nu1;                  
#endif     
   byte nu2[0x0E];              /* not used */
   byte ier;                    /* interrupt enable register */
   byte nu3[0x0F];              /* not used */
   byte ivr;                    /* interrupt vector register */
};


/*
    Defined below is the structure which is used to hold the
    board's configuration information.
*/

struct cblk470
{
    struct map470 *brd_ptr; /* base address of the I/O board */
    word param;             /* parameter mask for configuring board */
    byte e_mode;            /* enhanced operation flag */
    byte mask_reg;          /* output port mask register */
    byte ev_control[2];     /* event control registers */
    byte deb_control;       /* debounce control register */
    byte deb_duration[2];   /* debounce duration registers */
    byte deb_clock;         /* debounce clock select register */
    byte enable;            /* interrupt enable register */
    byte vector;            /* interrupt vector register */
    byte int_enable[6];	    /* individual bits enabled for interrupt */
    byte id_prom[32];       /* board ID Prom */
    byte ip_pos;            /* IP under service position */
    byte last_chan;         /* last interrupt input channel number */
    byte last_state;        /* last state of the interrupt channel */
};

struct handler_data {
   int h_pid;
   char* hd_ptr;
   };
