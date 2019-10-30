/*==============================================================================
//
//	L O A D T I M E . C P P
//
//	$Id: loadtime.c,v 1.1.1.1 2000/01/27 21:18:18 cvsuser Exp $
==============================================================================*/
#include <vxWorks.h>
#include <sysLib.h>
#include <ioLib.h>
#include <hostLib.h>
#include <sockLib.h>
#include <private/timerLibP.h>
#include <stdio.h>
#include <string.h>
#include "loadtime.h"

#define TIME_OFFSET	((unsigned long) 25567 * 24 * 60 * 60)

/*------------------------------------------------------------------------------
//	dateGet
//
//	This gives VxWorks a Unix-like command to print the date.
------------------------------------------------------------------------------*/
void dateGet()
{
	time_t t = time(0);
	struct tm const* const ptr = localtime(&t);

	printf("System time is %02d-%02d-%4d %02d:%02d:%02d", ptr->tm_mon + 1,
		ptr->tm_mday, ptr->tm_year + 1900, ptr->tm_hour, ptr->tm_min,
		ptr->tm_sec);
}

/*------------------------------------------------------------------------------
//	systemTimeSet
//
//	Retrieves the time from a networked machine using port 37.
------------------------------------------------------------------------------*/
STATUS systemTimeSet(char const* name)
{
	STATUS result = ERROR;
	int const ip = hostGetByName((char*) name);

	if (ERROR != ip) {
		int const s = socket(AF_INET, SOCK_STREAM, 0);

		if (ERROR != s) {
			struct sockaddr_in addr;

			addr.sin_family = AF_INET;
			addr.sin_port = htons(37);
			addr.sin_addr.s_addr = htonl(ip);

			if (ERROR != connect(s, (struct sockaddr*) &addr, sizeof(addr))) {
				unsigned long t;

				/* Read the 4-byte time value sent by the server. */

				if (read(s, (char*) &t, sizeof(t)) == sizeof(t)) {
					struct timespec ts;

					ts.tv_sec = 0;
					ts.tv_nsec = 16666666;
					clock_setres(CLOCK_REALTIME, &ts);

					/* Subtract off our 70 year offset since the time
					 server expresses time in seconds from 1990 and we need an
					 offset from 1970. */

					ts.tv_sec = t - TIME_OFFSET;
					ts.tv_nsec = 0;
					result = clock_settime(CLOCK_REALTIME, &ts);

					if (ERROR != result)
						dateGet();
				}
			}
			close(s);
		}
	}
	return result;
}
