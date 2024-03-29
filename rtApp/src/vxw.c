#include <vxWorks.h>
#include <intLib.h>
#include <iv.h>
#include "ip470.h"

/*
 * VxWork's INUM_TO_IVEC uses (VOIDFUNCPTR*) cast for intConnect(), but
 * intVecSet/Get() require cast to be (FUNCPTR*).  Therefore,
 * redefine INUM_TO_IVEC within this file.
 */
#ifdef INUM_TO_IVEC
#undef INUM_TO_IVEC
#endif
#define INUM_TO_IVEC(num) ((FUNCPTR*)(num<<2))

static FUNCPTR oldisr;

int attach_ihandler( trap, vector, zero, handler, hdata )
   int trap;
   byte vector;
   int zero;
   int (*handler)();
   struct handler_data* hdata; {
   oldisr = intVecGet(INUM_TO_IVEC(vector));
   intVecSet(INUM_TO_IVEC(vector),intHandlerCreate(handler,(int)hdata));
   return(0);
   }

int detach_ihandler( trap, vector, hdata )
   int trap;
   byte vector;
   struct handler_data* hdata; {
   intVecSet(INUM_TO_IVEC(vector),oldisr);
   return(0);
   }
