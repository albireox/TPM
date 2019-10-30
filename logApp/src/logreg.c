/*==============================================================================

  L O G R E G . C

  This module defines register manipulation routines.

  $Id: logreg.c,v 1.1.1.1 2000/01/27 21:18:18 cvsuser Exp $
==============================================================================*/
#include <string.h>
#include "lf.h"

/*------------------------------------------------------------------------------
  regDataSize

  Returns the size of the register's data.
------------------------------------------------------------------------------*/
int regDataSize(HLOGREG reg)
{
	switch (reg->type) {
	 case REG_UINT1:
	 case REG_INT8:
	 case REG_UINT8:
		 return 1;

	 case REG_INT16:
	 case REG_UINT16:
		 return 2;

	 case REG_INT32:
	 case REG_UINT32:
		 return 4;

	 case REG_FLOAT:
		 return sizeof(double);

	 default:
		 return IS_BLOB(reg) ? reg->type & ~0x80 : 0;
	}
}

/*------------------------------------------------------------------------------
  regGet

  Returns the data associated with the register. The data is returned as a
  32-bit, unsigned number.
------------------------------------------------------------------------------*/
u_int32_t regGet(HLOGREG reg)
{
	switch (reg->type) {
	 case REG_UINT1:
		 return (u_int32_t) (reg->data.ui1 ? 1 : 0);

	 case REG_INT8:
		 return (u_int32_t) reg->data.i8;

	 case REG_UINT8:
		 return (u_int32_t) reg->data.ui8;

	 case REG_INT16:
		 return (u_int32_t) (int32_t) ntohs(reg->data.ui16);

	 case REG_UINT16:
		 return (u_int32_t) ntohs(reg->data.ui16);

	 case REG_INT32:
		 return (u_int32_t) (int32_t) ntohl(reg->data.ui32);

	 case REG_UINT32:
		 return (u_int32_t) ntohl(reg->data.ui32);

	 case REG_FLOAT:
		 return (u_int32_t) reg->data.d;

	 default:
		 return 0;
	}
}

/*------------------------------------------------------------------------------
  regGetBlob

  Retrieves the contents of a blob field and stores it in a specified
  buffer.
------------------------------------------------------------------------------*/
int regGetBlob(HLOGREG reg, void* buf, size_t n)
{
	if (IS_BLOB(reg)) {
		memmove(buf, reg->data.b, BLOB_SIZE(reg) < n ? BLOB_SIZE(reg) : n);
		return 1;
	} else
		return 0;
}

/*------------------------------------------------------------------------------
  regGetDouble

  Returns the data associated with the register. The data is returned as a
  floating point number.
------------------------------------------------------------------------------*/
double regGetDouble(HLOGREG reg)
{
	switch (reg->type) {
	 case REG_UINT1:
		 return (double) (reg->data.ui1 ? 1.0 : 0.0);

	 case REG_INT8:
		 return (double) reg->data.i8;

	 case REG_UINT8:
		 return (double) reg->data.ui8;

	 case REG_INT16:
		 return (double) (int16_t) ntohs(reg->data.ui16);

	 case REG_UINT16:
		 return (double) ntohs(reg->data.ui16);

	 case REG_INT32:
		 return (double) (int32_t) ntohl(reg->data.ui32);

	 case REG_UINT32:
		 return (double) ntohl(reg->data.ui32);

	 case REG_FLOAT:
		 return reg->data.d;

	 default:
		 return 0.0;
	}
}

/*------------------------------------------------------------------------------
  regGetDescription

  Returns the description of the register.
------------------------------------------------------------------------------*/
char const* regGetDescription(HLOGREG reg)
{
	return reg->descr;
}

/*------------------------------------------------------------------------------
  regGetName

  Returns the name of the register.
------------------------------------------------------------------------------*/
char const* regGetName(HLOGREG reg)
{
	return reg->name;
}

/*------------------------------------------------------------------------------
  regGetPeriod

  Returns the name of the register.
------------------------------------------------------------------------------*/
unsigned long regGetPeriod(HLOGREG reg)
{
	return reg->period;
}

/*------------------------------------------------------------------------------
  regGetType

  Returns the data type of the register.
------------------------------------------------------------------------------*/
RegType regGetType(HLOGREG reg)
{
	return reg->type;
}

/*------------------------------------------------------------------------------
  regPut

  Sets the value of the register.
------------------------------------------------------------------------------*/
int regPut(HLOGREG reg, u_int32_t data)
{
	switch (reg->type) {
	 case REG_UINT1:
		 reg->data.ui1 = (u_int8_t) (data ? 1 : 0);
		 break;

	 case REG_INT8:
		 reg->data.i8 = (int8_t) data;
		 break;

	 case REG_UINT8:
		 reg->data.ui8 = (u_int8_t) data;
		 break;

	 case REG_INT16:
		 reg->data.i16 = htons((u_int16_t) data);
		 break;

	 case REG_UINT16:
		 reg->data.ui16 = htons((u_int16_t) data);
		 break;

	 case REG_INT32:
		 reg->data.i32 = htonl((u_int32_t) data);
		 break;

	 case REG_UINT32:
		 reg->data.ui32 = htonl((u_int32_t) data);
		 break;

	 case REG_FLOAT:
		 reg->data.d = (double) data;
		 break;

	 default:
		 return 0;
	}
	return 1;
}

/*------------------------------------------------------------------------------
  regPutBlob

  Updates the contents of a blob field from the contents of a
  specified buffer.
------------------------------------------------------------------------------*/
int regPutBlob(HLOGREG reg, void const* buf, size_t n)
{
	if (IS_BLOB(reg)) {
		memmove(reg->data.b, buf, BLOB_SIZE(reg) < n ? BLOB_SIZE(reg) : n);
		return 1;
	} else
		return 0;
}

/*------------------------------------------------------------------------------
  regPutDouble

  Sets the value of the register.
------------------------------------------------------------------------------*/
int regPutDouble(HLOGREG reg, double data)
{
	switch (reg->type) {
	 case REG_UINT1:
		 reg->data.ui1 = (u_int8_t) (data != 0.0 ? 1 : 0);
		 break;

	 case REG_INT8:
		 reg->data.i8 = (int8_t) data;
		 break;

	 case REG_UINT8:
		 reg->data.ui8 = (u_int8_t) data;
		 break;

	 case REG_INT16:
		 reg->data.i16 = htons((u_int16_t) data);
		 break;

	 case REG_UINT16:
		 reg->data.ui16 = htons((u_int16_t) data);
		 break;

	 case REG_INT32:
		 reg->data.i32 = htonl((u_int32_t) data);
		 break;

	 case REG_UINT32:
		 reg->data.ui32 = htonl((u_int32_t) data);
		 break;

	 case REG_FLOAT:
		 reg->data.d = data;
		 break;

	 default:
		 return 0;
	}
	return 1;
}

/*------------------------------------------------------------------------------
  regRead
------------------------------------------------------------------------------*/
int regRead(HLOGREG reg, FILE* fp)
{
	switch (reg->type) {
	 case REG_UINT1:
	 case REG_INT8:
	 case REG_UINT8:
	 case REG_INT16:
	 case REG_UINT16:
	 case REG_INT32:
	 case REG_UINT32:
	 case REG_FLOAT:
		 return 1 == fread(&reg->data, regDataSize(reg), 1, fp);

	 default:
		 if (IS_BLOB(reg))
			 return 1 == fread(reg->data.b, regDataSize(reg), 1, fp);
		 else
			 return 0;
	}
}

/*------------------------------------------------------------------------------
  regWrite
------------------------------------------------------------------------------*/
int regWrite(HLOGREG reg, FILE* fp)
{
	switch (reg->type) {
	 case REG_UINT1:
	 case REG_INT8:
	 case REG_UINT8:
	 case REG_INT16:
	 case REG_UINT16:
	 case REG_INT32:
	 case REG_UINT32:
	 case REG_FLOAT:
		 return 1 == fwrite(&reg->data, regDataSize(reg), 1, fp);

	 default:
		 if (IS_BLOB(reg))
			 return 1 == fwrite(reg->data.b, regDataSize(reg), 1, fp);
		 else
			 return 0;
	}
}
