/*==============================================================================
//
//		E R R L O G L I B . H
//
//		WARNING -- This file is not to be edited directly!
//		   You must make your modifications to the repository
//		   and then do a make install to update this file
//		   with your modifications.
//		   Modifications to this file not also made to the
//		   repository will be overwritten.
//
//		$Id: errlogLib.h,v 1.1.1.1 2000/01/27 21:18:15 cvsuser Exp $
//============================================================================*/
#ifndef __errlogLib_h
#define __errlogLib_h

#include <assert.h>

#define	MSG_DEBUG		(0)
#define	MSG_INFORM		(1)
#define	MSG_WARN		(2)
#define	MSG_CRITICAL	(3)

/*
 *		Function prototypes...
 */

#define __LOCATION__ "(" __FILE__ ":" __THE_LINE1(__LINE__) ") "
#define __THE_LINE1(z)	__THE_LINE2(z)
#define __THE_LINE2(z)	#z

struct LogObject;
typedef struct LogObject const* HLOG;

#ifdef __cplusplus

struct LogObject {
 private:
	char name[11];

	LogObject();
	operator= (LogObject const&);

 public:
	LogObject(char const*);
	~LogObject();
	void logMsg(int, int, char const*, int = 0, int = 0, int = 0, int = 0,
				int = 0, int = 0) const;
};

extern "C" {
#endif

void logClientAdd(char const*, int);
void logInit(void);
void logOff(void);
void logOn(void);
void _logMsg(HLOG, int, int, char const*, int, int, int, int, int, int);
HLOG logRegister(char const*);
void logStats(void);
void logTerm(void);
void logUnregister(HLOG*);

#ifdef __cplusplus
};
#endif

#define logAlarm6(l,a,b,c,d,e,f,g,h)	\
		_logMsg((l), MSG_CRITICAL, a, b,\
				(int)(c), (int)(d), (int)(e), (int)(f), (int)(g), (int)(h))
#define logAlarm5(l,a,b,c,d,e,f,g)	\
		_logMsg((l), MSG_CRITICAL, a, b,\
				(int)(c), (int)(d), (int)(e), (int)(f), (int)(g), 0)
#define logAlarm4(l,a,b,c,d,e,f)		\
		_logMsg((l), MSG_CRITICAL, a, b,\
				(int)(c), (int)(d), (int)(e), (int)(f), 0, 0)
#define logAlarm3(l,a,b,c,d,e)		\
		_logMsg((l), MSG_CRITICAL, a, b,\
				(int)(c), (int)(d), (int)(e), 0, 0, 0)
#define logAlarm2(l,a,b,c,d)			\
		_logMsg((l), MSG_CRITICAL, a, b,\
				(int)(c), (int)(d), 0, 0, 0, 0)
#define logAlarm1(l,a,b,c)			\
		_logMsg((l), MSG_CRITICAL, a, b,\
				(int)(c), 0, 0, 0, 0, 0)
#define logAlarm0(l,a,b)				\
		_logMsg((l), MSG_CRITICAL, a, b,\
				0, 0, 0, 0, 0, 0)

#define logWarn6(l,a,b,c,d,e,f,g,h)	\
		_logMsg((l), MSG_WARN, a, b,\
				(int)(c), (int)(d), (int)(e), (int)(f), (int)(g), (int)(h))
#define logWarn5(l,a,b,c,d,e,f,g)		\
		_logMsg((l), MSG_WARN, a, b,\
				(int)(c), (int)(d), (int)(e), (int)(f), (int)(g), 0)
#define logWarn4(l,a,b,c,d,e,f)		\
		_logMsg((l), MSG_WARN, a, b,\
				(int)(c), (int)(d), (int)(e), (int)(f), 0, 0)
#define logWarn3(l,a,b,c,d,e)			\
		_logMsg((l), MSG_WARN, a, b,\
				(int)(c), (int)(d), (int)(e), 0, 0, 0)
#define logWarn2(l,a,b,c,d)			\
		_logMsg((l), MSG_WARN, a, b,\
				(int)(c), (int)(d), 0, 0, 0, 0)
#define logWarn1(l,a,b,c)				\
		_logMsg((l), MSG_WARN, a, b,\
				(int)(c), 0, 0, 0, 0, 0)
#define logWarn0(l,a,b)				\
		_logMsg((l), MSG_WARN, a, b,\
				0, 0, 0, 0, 0, 0)

#define logInform6(l,a,b,c,d,e,f,g)	\
		_logMsg((l), MSG_INFORM, 0, a, (int)(b), (int)(c), (int)(d), (int)(e), \
                (int)(f), (int)(g))
#define logInform5(l,a,b,c,d,e,f)	\
		_logMsg((l), MSG_INFORM, 0, a, (int)(b), (int)(c), (int)(d), (int)(e), \
                (int)(f), 0)
#define logInform4(l,a,b,c,d,e)	\
		_logMsg((l), MSG_INFORM, 0, a, (int)(b), (int)(c), (int)(d), (int)(e), \
                0, 0)
#define logInform3(l,a,b,c,d)		\
		_logMsg((l), MSG_INFORM, 0, a, (int)(b), (int)(c), (int)(d), 0, 0, 0)
#define logInform2(l,a,b,c)		\
		_logMsg((l), MSG_INFORM, 0, a, (int)(b), (int)(c), 0, 0, 0, 0)
#define logInform1(l,a,b)			\
		_logMsg((l), MSG_INFORM, 0, a, (int)(b), 0, 0, 0, 0, 0)
#define logInform0(l,a)			\
		_logMsg((l), MSG_INFORM, 0, a, 0, 0, 0, 0, 0, 0)

#ifndef NDEBUG

#define logDebug6(l,a,b,c,d,e,f,g)	\
		_logMsg((l), MSG_DEBUG, 0, __LOCATION__ ## a,\
				(int)(b), (int)(c), (int)(d), (int)(e), (int)(f), (int)(g))
#define logDebug5(l,a,b,c,d,e,f)		\
		_logMsg((l), MSG_DEBUG, 0, __LOCATION__ ## a,\
				(int)(b), (int)(c), (int)(d), (int)(e), (int)(f), 0)
#define logDebug4(l,a,b,c,d,e)		\
		_logMsg((l), MSG_DEBUG, 0, __LOCATION__ ## a,\
				(int)(b), (int)(c), (int)(d), (int)(e), 0, 0)
#define logDebug3(l,a,b,c,d)			\
		_logMsg((l), MSG_DEBUG, 0, __LOCATION__ ## a,\
				(int)(b), (int)(c), (int)(d), 0, 0, 0)
#define logDebug2(l,a,b,c)			\
		_logMsg((l), MSG_DEBUG, 0, __LOCATION__ ## a,\
				(int)(b), (int)(c), 0, 0, 0, 0)
#define logDebug1(l,a,b)				\
		_logMsg((l), MSG_DEBUG, 0, __LOCATION__ ## a,\
				(int)(b), 0, 0, 0, 0, 0)
#define logDebug0(l,a)				\
		_logMsg((l), MSG_DEBUG, 0, __LOCATION__ ## a,\
				0, 0, 0, 0, 0, 0)

#else

#define logDebug6(l,a,b,c,d,e,f,g)
#define logDebug5(l,a,b,c,d,e,f)
#define logDebug4(l,a,b,c,d,e)
#define logDebug3(l,a,b,c,d)
#define logDebug2(l,a,b,c)
#define logDebug1(l,a,b)
#define logDebug0(l,a)

#endif

#endif
