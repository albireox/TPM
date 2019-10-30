/*==============================================================================
  LOG.C

  Test program for the log file library.

  $Id: log.c,v 1.1.1.1 2000/01/27 21:18:18 cvsuser Exp $
==============================================================================*/
#include <stdio.h>
#include <sys/types.h>
#include "logfile.h"

static char const fName[] = "/mcptpm/sample.dat";

void testWrite(void)
{
	HLOGFILE lf = logCreate(fName, 200, 3);

	fprintf(stderr, "log -- Log file testing program\n");
	if (NULL != lf) {
		int ii;
		HLOGREG reg1 = 0, reg2 = 0, reg3 = 0;

		printf("Successfully created '%s'...\n", fName);
		logSetDescription(lf, "This is a sample file.");

		reg1 = logCreateRegister(lf, "REG1", "This is the first register",
								 REG_INT8, 1);
		if (reg1)
			printf("Register #1:\n\tName: '%s'\n\tDescription: '%s'\n"
				   "\tPeriod: '%d'\n\tType: '%d'\n", regGetName(reg1),
				   regGetDescription(reg1), (int) regGetPeriod(reg1),
				   regGetType(reg1));
		else
			fprintf(stderr, "log: couldn't create register #1\n");

		reg2 = logCreateRegister(lf, "REG2", "This is the second register",
								 REG_INT16, 2);
		if (reg2)
			printf("Successfully created register #2...\n");
		else
			fprintf(stderr, "log: couldn't create register #2\n");

		reg3 = logCreateRegister(lf, "REG3", "This is the third register",
								 REG_INT32, 3);
		if (reg3)
			printf("Successfully created register #3...\n");
		else
			fprintf(stderr, "log: couldn't create register #3\n");

		for (ii = 0; ii < 10; ++ii) {
			regPut(reg1, (u_int32_t) ii);
			regPut(reg2, (u_int32_t) ii + 100000);
			regPut(reg3, (u_int32_t) ii + 200000);
			logSaveData(lf);
		}

		logClose(&lf);
	} else
		fprintf(stderr, "log: couldn't create '%s'\n", fName);
}

void testRead(void)
{
	HLOGFILE lf = logOpen(fName);

	if (NULL != lf) {
		unsigned count;
		int ii;

		printf("Information about '%s':\n\tSample Freq: %d hertz\n", fName,
			   logGetSampleRate(lf));

		printf("\t# of Registers: %d\n", count = logGetRegisterCount(lf));

		printf("\nData dump:\n\nSample ");
		for (ii = 0; ii < count; ++ii)
			printf("%10s ", regGetName(logGetRegister(lf, ii)));
		for (ii = 0; logLoadData(lf, ii); ++ii) {
			int jj;

			printf("\n%6d ", ii);
			for (jj = 0; jj < count; ++jj)
				printf("%10ld ", (long) regGet(logGetRegister(lf, jj)));
		}
		putchar('\n');

		logClose(&lf);
	} else
		fprintf(stderr, "log: couldn't open '%s'\n", fName);
}

#if 1
int main(int argc, char *argv[])
{
	testWrite();
#if 1
	testRead();
#endif
	return 0;
}
#endif
