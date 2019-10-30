/*==============================================================================

  L O A D T I M E . H

  $Id: loadtime.h,v 1.1.1.1 2000/01/27 21:18:18 cvsuser Exp $
==============================================================================*/
#ifndef __LOADTIME_H
#define __LOADTIME_H

#ifdef __cplusplus
extern "C" {
#endif

void dateGet(void);
STATUS systemTimeSet(char const*);

#ifdef __cplusplus
};
#endif

#endif
