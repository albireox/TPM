/*==============================================================================

  L O G F I L E . C

  This module defines the routines that make up the log file API.

  $Id: logfile.c,v 1.2 2000/06/23 01:54:14 peregrin Exp $
==============================================================================*/
#include <stdlib.h>
#include <string.h>
#include "lf.h"
#include <sys/stat.h>
#include "errMdef.h"

#define	SIGNATURE		0x00131c0bL
#define VERSION			0x0101

/*
 *		Local function prototypes...
 */
static long computeOffset(HLOGFILE, unsigned);
static unsigned getAbsoluteLastIndex(HLOGFILE const);
static unsigned lastIndex(unsigned, unsigned);
static int logReadHeader(HLOGFILE const);
static int logWriteHeader(HLOGFILE const);
static long occurances(unsigned, unsigned);

/*------------------------------------------------------------------------------
  computeOffset

  Computes the offset into the file that corresponds to the base
  offset of an index.
------------------------------------------------------------------------------*/
static long computeOffset(HLOGFILE file, unsigned idx)
{
	int ii;

	/* Calculate the file header. */

	long offset = sizeof(struct FileHeader)
		+ sizeof(struct FileRegister) * file->nReg;

	/* Loop through the registers and calculate the amount of space
       taken by each of them. */

	for (ii = 0; ii < file->nReg; ++ii)
		offset += occurances(file->reg[ii].period, idx)
			* regDataSize(file->reg + ii);
	return offset;
}

/*------------------------------------------------------------------------------
  getAbsoluteLastIndex
------------------------------------------------------------------------------*/
static unsigned getAbsoluteLastIndex(HLOGFILE const file)
{
	struct stat fStat;

	assert(modeRead == file->mode && file->fp);

	/* Get the file status information. We can use the file length to
       optimize our calculations. */

	if (!fstat(fileno(file->fp), &fStat)) {
		unsigned long const offset = sizeof(struct FileHeader)
			+ sizeof(struct FileRegister) * file->nReg;
		unsigned long highIndex = (unsigned long) -1, lowIndex;
		unsigned ii, elementSize = 0, minSize = (unsigned) -1, maxPeriod = 0;

		/* Loop through the registers, taking this time to compute the
		   smallest data and the largest sample period. */

		for (ii = 0; ii < file->nReg; ++ii) {
			unsigned const dataSize = regDataSize(file->reg + ii);
			unsigned long const period = regGetPeriod(file->reg + ii);

			/* If someone puts a 0Hz register in the file, make sure
               we don't choke! */

			if (period > 0) {
				elementSize += dataSize;

				/* Keep track of the smallest record. */

				if (dataSize < minSize)
					minSize = dataSize;

				/* Keep track of the largest period. */

				if (period > maxPeriod)
					maxPeriod = period;
			}
		}

		/* Compute the lowest possible index. This is found by
		   assuming all the registers are being saved each time. */

		lowIndex = (fStat.st_size - offset) / elementSize;

		/* Now compute the high boundary. We can get reasonably close
           by breaking the data up into "buckets". Each bucket
           represents 'maxPeriod' samples. If we find out how many of
           these buckets fit using the file size, we've narrowed down
           the high boundary to within 'maxPeriod' samples. */

		{
			unsigned long bucketSize = 0;

			for (ii = 0; ii < file->nReg; ++ii) {
				unsigned long const period = regGetPeriod(file->reg + ii);

				if (period > 0)
					bucketSize += regDataSize(file->reg + ii) *
						(maxPeriod / period);
			}
			highIndex = ((fStat.st_size - offset) / bucketSize + 2) * maxPeriod;
		}

		/* Repeat until we converge on the result. */

		while (highIndex != lowIndex) {
			unsigned long const tmp =
				(unsigned long) ((highIndex - lowIndex) / 2.0 + lowIndex + 0.5);

			if (logLoadData(file, tmp))
				lowIndex = tmp;
			else
				highIndex = tmp - 1;
		}
		return lowIndex;
	}
	return 0;
}

/*------------------------------------------------------------------------------
  lastIndex

  Computes the last index that a given index would have been saved,
  based upon the specified period.
------------------------------------------------------------------------------*/
static unsigned lastIndex(unsigned period, unsigned idx)
{
	assert(period != 0);

	return idx - (idx % period);
}

/*------------------------------------------------------------------------------
  logClose

  Shuts down a log file.
------------------------------------------------------------------------------*/
void logClose(HLOGFILE* file)
{
	assert(*file);
	if (*file) {

		/* If we've opened a file, close it. */

		if (NULL != (*file)->fp)
			fclose((*file)->fp);

		/* If we allocated memory, free it up. */

		if (NULL != (*file)->reg) {
			int ii;

			for (ii = 0; ii < (*file)->nReg; ++ii)
				if (IS_BLOB((*file)->reg + ii))
					free(((*file)->reg + ii)->data.b);
			free((*file)->reg);
		}

		/* Now free up the structure and NULL the pointer. */

		free(*file);
		*file = NULL;
	}
}

/*------------------------------------------------------------------------------
  logCreate

  Creates a new log file. The log file is placed in "pre-write" mode.
------------------------------------------------------------------------------*/
HLOGFILE logCreate(char const* name, unsigned short saveRate,
				   unsigned short nReg)
{
	HLOGFILE ptr = (HLOGFILE) malloc(sizeof(struct _LogFile));

	if (ptr) {

		/* Initialize the fields that aren't going to get initialized
           by the parameters. */

		ptr->description[0] = '\0';
		ptr->mode = modeInvalid;
		ptr->totReg = ptr->nReg = ptr->tick = 0;
		ptr->saveRate = saveRate;
		ptr->reg = 0;

		if ((ptr->fp = fopen(name, "w"))) {

			/* Assign the passed arguments to the log file object. */

			ptr->mode = modePreWrite;
			ptr->totReg = nReg;
			ptr->reg = (HLOGREG) malloc(sizeof(struct _LogRegister) * nReg);

			/* If we couldn't allocate enough space for the register,
			   free up the memory we already used and return NULL to
			   the caller. */

			if (!ptr->reg) {
				fclose(ptr->fp);
				free(ptr);
				ptr = 0;
			}
		} else {
			epicsPrintf("Failed to open logfile: %s\n", name);
			free(ptr);
			ptr = 0;
		}
	}
	return ptr;
}

/*------------------------------------------------------------------------------
  logCreateRegister

  Adds a register to the log file. The log file must be in PreWrite
  mode for this to work.
------------------------------------------------------------------------------*/
HLOGREG logCreateRegister(HLOGFILE file, char const* name, char const* desc,
						  RegType type, unsigned long period)
{
	assert(file);

	/* If we're in pre-write mode and there is still some space in the
       register list, we grab the next entry. */

	if (modePreWrite == file->mode && file->nReg < file->totReg) {
		HLOGREG const ptr = file->reg + file->nReg++;

		memset(ptr, 0, sizeof(*ptr));

		/* Transfer the name, taking care to not exceed our buffer. */

		strncpy(ptr->name, name, sizeof(ptr->name) - 1);
		ptr->name[sizeof(ptr->name) - 1] = '\0';

		/* Transfer the description, taking care to not exceed our
           buffer. */

		strncpy(ptr->descr, desc, sizeof(ptr->descr) - 1);
		ptr->descr[sizeof(ptr->descr) - 1] = '\0';

		/* Transfer the other two fields. */

		ptr->period = period;
		ptr->type = (unsigned char) type;
		if (IS_BLOB(ptr))
			ptr->data.b = malloc(BLOB_SIZE(ptr));
		else
			regPut(ptr, 0);
		return ptr;
	} else {
		if (modePreWrite != file->mode) printf("Not in pre-write mode\n");
		if (file->nReg > file->totReg) printf("nReg %d exceeds totReg %d\n", file->nReg, file->totReg);

		return 0;
	}
}

/*------------------------------------------------------------------------------
  logGetBaseTime
------------------------------------------------------------------------------*/
time_t logGetBaseTime(HLOGFILE file)
{
	assert(file);
	return file->baseTime;
}

/*------------------------------------------------------------------------------
  logGetDescription
------------------------------------------------------------------------------*/
char const* logGetDescription(HLOGFILE file)
{
	assert(file);
	return file->description;
}

/*------------------------------------------------------------------------------
  logGetLastIndex

  Returns the last index of the file. This function is only valid when
  the log file is opened for reading.
------------------------------------------------------------------------------*/
unsigned long logGetLastIndex(HLOGFILE file)
{
	return modeRead == file->mode ? file->maxIndex : 0;
}

/*------------------------------------------------------------------------------
  logGetNamedRegister

  Returns a handle to the register of the log file with the given name.
------------------------------------------------------------------------------*/
HLOGREG logGetNamedRegister(HLOGFILE file, char const* name)
{
	int ii;

	assert(file);
	for (ii = 0; ii < file->nReg; ++ii)
		if (!strcmp(name, file->reg[ii].name))
			return file->reg + ii;
	return 0;
}

/*------------------------------------------------------------------------------
  logGetSampleRate

  Returns the sample rate of the file.
------------------------------------------------------------------------------*/
unsigned logGetSampleRate(HLOGFILE file)
{
	assert(file);
	return file->saveRate;
}

/*------------------------------------------------------------------------------
  logGetRegister

  Returns a handle to the idx'th register of the log file.
------------------------------------------------------------------------------*/
HLOGREG logGetRegister(HLOGFILE file, unsigned idx)
{
	assert(file);
	return idx < file->nReg ? file->reg + idx : NULL;
}

/*------------------------------------------------------------------------------
  logGetRegisterCount

  Returns the total number of registers present in the log file.
------------------------------------------------------------------------------*/
unsigned short logGetRegisterCount(HLOGFILE file)
{
	assert(file);
	return file->nReg;
}

/*------------------------------------------------------------------------------
  logLoadData

  Loads the registers with the values that were present when index
  'idx' occurred.
------------------------------------------------------------------------------*/
int logLoadData(HLOGFILE file, unsigned idx)
{
	int ii, err = 1;

	/* Loop through every register of the log file. */

	for (ii = 0; ii < file->nReg; ++ii) {
		unsigned const last = lastIndex(file->reg[ii].period, idx);
		int extra = 0, jj;

		/* Compute the extra space taken up by registers prior to the
           one we want. */

		for (jj = 0; jj < ii; ++jj) {
			assert(file->reg[jj].period != 0);

			if (!(last % file->reg[jj].period))
				extra += regDataSize(file->reg + jj);
		}

		/* Try to index to the position in the file and read the
           data. */
		
		if (!fseek(file->fp, computeOffset(file, last) + extra, SEEK_SET))
			if (regRead(file->reg + ii, file->fp)) {
				err = 0;
				continue;
			}

		/* We couldn't read this parameter, so zero it out. */

		regPut(file->reg + ii, 0);
	}
	return !err;
}

/*------------------------------------------------------------------------------
  logLoadRegData

  Loads the specified register with the value that was present when index 'idx'
  occurred.
------------------------------------------------------------------------------*/
int logLoadRegData(HLOGFILE file, unsigned idxReg, unsigned idx)
{
	assert(idxReg < file->nReg);

	if (idxReg < file->nReg) {
		unsigned const last = lastIndex(file->reg[idxReg].period, idx);
		int extra = 0, jj;

		/* Compute the extra space taken up by registers prior to the
		   one we want. */

		for (jj = 0; jj < idxReg; ++jj) {
			assert(file->reg[jj].period != 0);

			if (!(last % file->reg[jj].period))
				extra += regDataSize(file->reg + jj);
		}

		/* Try to index to the position in the file and read the
		   data. */
			
		if (!fseek(file->fp, computeOffset(file, last) + extra, SEEK_SET))
			if (regRead(file->reg + idxReg, file->fp))
				return 1;

		/* We couldn't read this parameter, so zero it out. */

		regPut(file->reg + idxReg, 0);
	}
	return 0;
}

/*------------------------------------------------------------------------------
  logOpen

  Opens the log file for reading.
------------------------------------------------------------------------------*/
HLOGFILE logOpen(char const* name)
{
	HLOGFILE ptr = (HLOGFILE) malloc(sizeof(struct _LogFile));

	if (0 != ptr)
		if (0 == (ptr->fp = fopen(name, "r")) || !logReadHeader(ptr)) {

			/* If the file is opened, close it. */

			if (ptr->fp)
				fclose(ptr->fp);

			/* Reclaim the memory we allocated. */

			free(ptr);
			ptr = 0;
		}
	return ptr;
}

/*------------------------------------------------------------------------------
  logReadHeader

  Reads the header information form the already opened file specified
  in the log file structure.
------------------------------------------------------------------------------*/
static int logReadHeader(HLOGFILE const f)
{
	struct FileHeader fh;

	if (1 == fread(&fh, sizeof(fh), 1, f->fp))
		if (ntohl(fh.signature) == SIGNATURE && ntohs(fh.version) >= VERSION) {
			struct tm gmt;

			/* Transfer the file header stuff into our log file
               object. */

			gmt.tm_year = ntohs(fh.year) - 1900;
			gmt.tm_mon = fh.month;
			gmt.tm_mday = fh.day;
			gmt.tm_hour = fh.hour;
			gmt.tm_min = fh.minute;
			gmt.tm_sec = fh.second;
			f->baseTime = mktime(&gmt);
			strcpy(f->description, fh.description);
			f->tick = 0;
			f->nReg = f->totReg = ntohs(fh.nReg);
			f->mode = modeRead;
			f->saveRate = ntohs(fh.saveRate);

			/* Allocate space for all the registers. If we can't
               allocate the space, we return an error status to the
               caller. */

			f->reg = (HLOGREG) malloc(sizeof(struct _LogRegister) * f->nReg);
			if (f->reg) {
				int ii;

				for (ii = 0; ii < f->nReg; ++ii) {
					struct FileRegister fr;

					/* Try to read in the register's state. If we
                       can't, free up the memory we used and return
                       and error. */

					if (1 == fread(&fr, sizeof(fr), 1, f->fp)) {
						HLOGREG const ptr = f->reg + ii;

						ptr->period = ntohl(fr.period);
						strncpy(ptr->name, fr.name, sizeof(ptr->name) - 1);
						ptr->name[sizeof(ptr->name) - 1] = '\0';
						ptr->type = fr.type;
						strncpy(ptr->descr, fr.descr, sizeof(ptr->descr) - 1);
						ptr->descr[sizeof(ptr->descr) - 1] = '\0';
						regPut(ptr, (u_int32_t) 0);
					} else {
						free(f->reg);
						f->reg = 0;
						return 0;
					}
				}

				/* Now that everything was successful, try to
                   determine what the last index of the file is. */

				f->maxIndex = getAbsoluteLastIndex(f);
				return 1;
			}
		}
	return 0;
}

/*------------------------------------------------------------------------------
  logSaveData

  Saves the data in the registers to the file.
------------------------------------------------------------------------------*/
int logSaveData(HLOGFILE file)
{
	/* If we're in pre-write mode, we have to first initialize the log
       file. */

	if (modePreWrite == file->mode) {

		/* Stamp the time and write the header to the file. If we
           can't write the header to the file, return an error to the
           caller. */

		time(&file->baseTime);
		if (!logWriteHeader(file)) {
			file->mode = modeInvalid;
			return 0;
		}
	}

	/* Only write to the log file if we're in write mode. */

	if (modeWrite == file->mode) {
		int ii;

		for (ii = 0; ii < file->nReg; ++ii) {
			unsigned long const period = file->reg[ii].period;

			assert(period != 0);

			/* If it's this register's turn to write to the file, do
               it. If we can't write to the file, return an error and
               mark the file as invalid. */

			if (!(file->tick % period) && !regWrite(file->reg + ii, file->fp)) {
				file->mode = modeInvalid;
				return 0;
			}
		}
		++file->tick;
		return 1;
	} else
		return 0;
}

/*------------------------------------------------------------------------------
  logSetDescription

  Sets the description associated with the log file. This can only be
  set when the file is in pre-write mode.
------------------------------------------------------------------------------*/
void logSetDescription(HLOGFILE file, char const* descr)
{
	/* Only allow modification if we're in pre-write mode. (Any other
       mode will have the file header already written. */

	if (modePreWrite == file->mode) {
		strncpy(file->description, descr, sizeof(file->description) - 1);
		file->description[sizeof(file->description) - 1] = '\0';
	}
}

/*------------------------------------------------------------------------------
  logWriteHeader

  Writes out the file header to the file.
------------------------------------------------------------------------------*/
static int logWriteHeader(HLOGFILE const file)
{
	/* Only do this if we're in pre-write mode. */

	if (modePreWrite == file->mode && file->fp) {
		struct FileHeader fh;
		struct tm* gmt;

		memset(&fh, 0, sizeof(fh));

		fh.signature = htonl(SIGNATURE);
		fh.version = htons(VERSION);
		fh.saveRate = htons(file->saveRate);
		
		/* Stuff the header with day information. */

		gmt = localtime(&file->baseTime);
		fh.year = htons(gmt->tm_year + 1900);
		fh.month = (unsigned char) gmt->tm_mon;
		fh.day = (unsigned char) gmt->tm_mday;
		fh.hour = (unsigned char) gmt->tm_hour;
		fh.minute = (unsigned char) gmt->tm_min;
		fh.second = (unsigned char) gmt->tm_sec;

		fh.padding0 = 0;
		strcpy(fh.description, file->description);
		fh.nReg = htons(file->nReg);
		fh.padding1 = 0;

		/* Write the header to the file. If we have a problem, change
           the state of the log file and return and error to the
           caller. */

		if (1 == fwrite(&fh, sizeof(fh), 1, file->fp)) {
			int ii;

			/* Loop through the defined registers. */

			for (ii = 0; ii < file->nReg; ++ii) {
				struct FileRegister fr;
				HLOGREG const reg = file->reg + ii;

				/* Transfer the register information into the register
                   layout structure. */

				fr.period = htonl(reg->period);
				strcpy(fr.name, reg->name);
				fr.type = reg->type;
				strcpy(fr.descr, reg->descr);

				/* Write the layout to the disk. If there's an error,
                   we're through. */

				if (1 != fwrite(&fr, sizeof(fr), 1, file->fp)) {
					file->mode = modeInvalid;
					return 0;
				}
			}
			file->mode = modeWrite;
			return 1;
		}
	}
	file->mode = modeInvalid;
	return 0;
}

/*------------------------------------------------------------------------------
  occurances

  Returns the number of times the data point would have been saved at
  the given index, if the data point had the given period.
------------------------------------------------------------------------------*/
static long occurances(unsigned period, unsigned idx)
{
	assert(period > 0);

	return (idx + period - 1) / period;
}
