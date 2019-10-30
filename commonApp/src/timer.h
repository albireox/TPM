/*============================================================================
//
//	T I M E R . C
//
//  $Id: timer.h,v 1.1.1.1 2000/01/27 21:18:15 cvsuser Exp $
============================================================================*/
int TimerStart(unsigned long freq, unsigned char level, void (*func)());
void TimerStop();
