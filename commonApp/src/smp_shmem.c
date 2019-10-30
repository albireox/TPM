/*==============================================================================

  S M P _ F U N C . C

  This file contains all the sampling functions for the registers.

  $Id: smp_shmem.c,v 1.2 2000/03/02 20:07:23 peregrin Exp $
==============================================================================*/
#include <vxWorks.h>
#include <errnoLib.h>
#include <nfsDrv.h>
#include <nfsLib.h>
#include <usrLib.h>
#include <stdio.h>
#include "errlogLib.h"
#include "timer.h"
#include "data_collection.h"

#define SHREAD1(fname, field_name) \
unsigned long shmem_fname() { \
	return data->field_name; \
}	

#define SHREAD2(fname, field1_name, field2_name) \
unsigned long shmem_fname() { \
	return (data->field1_name << 16) + data->field2_name); \
}

SHREAD2(ALCPOS, axis[1].position_hi, axis[1].position_lo)
SHREAD(ALCVOLT, axis[1].voltage)
SHREAD2(ALENC1, axis[1].actual_position_hi, axis[1].actual_position_lo)
SHREAD2(ALENC2, axis[1].actual_position2_hi, axis[1].actual_position2_lo)
SHREAD(ALLWC, status.i6.il0.s_wind_stop)
SHREAD(ALMCPPOS, data->pvt[0].position)
SHREAD(ALMCPVEL, data->pvt[0].velocity)
SHREAD(ALMPCTIM, data->pvt[0].time)
SHREAD(ALMTRC1,  data->status.i3.alt_1_current)
SHREAD(ALMTRC2, data->status.i3.alt_2_current)

/*------------------------------------------------------------------------------
  read_ALMTRV1

  Reads the first motor voltage.
------------------------------------------------------------------------------*/
void read_ALMTRV1(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i3.alt_1_voltage);
}

/*------------------------------------------------------------------------------
  read_ALMTRV2

  Reads the second motor voltage.
------------------------------------------------------------------------------*/
void read_ALMTRV2(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i3.alt_2_voltage);
}

/*------------------------------------------------------------------------------
  read_ALP100

  Reads the altitude 100 deg limit switch.
------------------------------------------------------------------------------*/
void read_ALP100(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i7.il0.alt_les_90d5_limit);
}

/*------------------------------------------------------------------------------
  read_ALP20

  Reads the altitude 20 deg limit switch.
------------------------------------------------------------------------------*/
void read_ALP20(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i7.il0.alt_grt_18d6_limit_1);
}

/*------------------------------------------------------------------------------
  read_ALP_2

  Reads the altitude -2 deg limit switch.
------------------------------------------------------------------------------*/
void read_ALP_2(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i7.il0.alt_grt_0d3_limit);
}

/*------------------------------------------------------------------------------
  read_ALRWC

  Reads altitude windscreen collision when raising.
------------------------------------------------------------------------------*/
void read_ALRWC(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i6.il0.n_wind_stop);
}

/*------------------------------------------------------------------------------
  read_ALSTATE

  Reads the state variable for altitude.
------------------------------------------------------------------------------*/
void read_ALSTATE(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->axis_state[1]);
}

/*------------------------------------------------------------------------------
  read_ALSTOW

  Reads the "stow" interlock on the altitude axis.
------------------------------------------------------------------------------*/
void read_ALSTOW(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i7.il0.alt_locking_pin_out);
}

/*------------------------------------------------------------------------------
  read_ALTCCPOS

  Reads the TCC interpretation of position PVT for altitude.
------------------------------------------------------------------------------*/
void read_ALTCCPOS(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->tccmove[0].position);
}

/*------------------------------------------------------------------------------
  read_ALTCCVEL

  Reads the TCC interpretation of velocity PVT for altitude.
------------------------------------------------------------------------------*/
void read_ALTCCVEL(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->tccmove[0].velocity);
}

/*------------------------------------------------------------------------------
  read_ALTCCTIM

  Reads the TCC interpretation of time PVT for altitude.
------------------------------------------------------------------------------*/
void read_ALTCCTIM(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->tccmove[0].time);
}

/*------------------------------------------------------------------------------
  read_ALWSPOS

  Reads the LDVT windscreen position.
------------------------------------------------------------------------------*/
void read_ALWSPOS(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i2.alt_lvdt_error);
}

/*------------------------------------------------------------------------------
  read_AZCPOS

  Reads the position "command".
------------------------------------------------------------------------------*/
void read_AZCPOS(HLOGREG reg)
{
	unsigned long value;

	assert(reg);
	value = (data->axis[0].position_hi << 16) + data->axis[0].position_lo;
	regPut(reg, value);
}

/*------------------------------------------------------------------------------
  read_AZCVOLT

  Reads the voltage "command".
------------------------------------------------------------------------------*/
void read_AZCVOLT(HLOGREG reg)
{
	assert(reg);

	regPut(reg, data->axis[0].voltage);
}

/*------------------------------------------------------------------------------
  read_AZCCWC

  Reads the limit switch that detects CCW collisions with the
  windscreen.
------------------------------------------------------------------------------*/
void read_AZCCWC(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i1.il0.az_bump_ccw);
}

/*------------------------------------------------------------------------------
  read_AZCCWHL

  Reads the CCW hard limit switch.
------------------------------------------------------------------------------*/
void read_AZCCWHL(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i7.il0.az_pos_410b_ccw);
}

/*------------------------------------------------------------------------------
  read_AZCCWSL

  Reads the CCW hard limit switch.
------------------------------------------------------------------------------*/
void read_AZCCWSL(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i7.il0.az_pos_410a_ccw);
}

/*------------------------------------------------------------------------------
  read_AZCWC

  Reads the limit switch that detects CW collisions with the
  windscreen.
------------------------------------------------------------------------------*/
void read_AZCWC(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i1.il0.az_bump_cw);
}

/*------------------------------------------------------------------------------
  read_AZCWHL

  Reads the CW hard limit switch.
------------------------------------------------------------------------------*/
void read_AZCWHL(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i7.il0.az_neg_170b_cw);
}

/*------------------------------------------------------------------------------
  read_AZCWSL

  Reads the CW hard limit switch.
------------------------------------------------------------------------------*/
void read_AZCWSL(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i7.il0.az_neg_170a_cw);
}

/*------------------------------------------------------------------------------
  read_AZENC1

  Reads the first azimuth encoder.
------------------------------------------------------------------------------*/
void read_AZENC1(HLOGREG reg)
{
	unsigned long value;

	assert(reg);
	value = (data->axis[0].actual_position_hi << 16) +
		data->axis[0].actual_position_lo;
	regPut(reg, value);
}

/*------------------------------------------------------------------------------
  read_AZENC2

  Reads the second azimuth encoder.
------------------------------------------------------------------------------*/
void read_AZENC2(HLOGREG reg)
{
	unsigned long value;

	assert(reg);
	value = (data->axis[0].actual_position2_hi << 16) +
		data->axis[0].actual_position2_lo;
	regPut(reg, value);
}

/*------------------------------------------------------------------------------
  read_AZMCPPOS

  Reads the MCP interpretation of position PVT for azimuth.
------------------------------------------------------------------------------*/
void read_AZMCPPOS(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->pvt[1].position);
}

/*------------------------------------------------------------------------------
  read_AZMCPVEL

  Reads the MCP interpretation of velocity PVT for azimuth.
------------------------------------------------------------------------------*/
void read_AZMCPVEL(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->pvt[1].velocity);
}

/*------------------------------------------------------------------------------
  read_AZMCPTIM

  Reads the MCP interpretation of time PVT for azimuth.
------------------------------------------------------------------------------*/
void read_AZMCPTIM(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->pvt[1].time);
}

/*------------------------------------------------------------------------------
  read_AZMTRC1

  Reads the azimuth motor current.
------------------------------------------------------------------------------*/
void read_AZMTRC1(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i3.az_1_current);
}

/*------------------------------------------------------------------------------
  read_AZMTRC2

  Reads the azimuth motor current.
------------------------------------------------------------------------------*/
void read_AZMTRC2(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i3.az_2_current);
}

/*------------------------------------------------------------------------------
  read_AZMTRV1

  Reads the azimuth motor voltage.
------------------------------------------------------------------------------*/
void read_AZMTRV1(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i3.az_1_voltage);
}

/*------------------------------------------------------------------------------
  read_AZMTRV2

  Reads the azimuth motor voltage.
------------------------------------------------------------------------------*/
void read_AZMTRV2(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i3.az_2_voltage);
}

/*------------------------------------------------------------------------------
  read_AZSTATE

  Reads the state variable for azimuth.
------------------------------------------------------------------------------*/
void read_AZSTATE(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->axis_state[0]);
}

/*------------------------------------------------------------------------------
  read_AZTCCPOS

  Reads the TCC interpretation of position PVT for azimuth.
------------------------------------------------------------------------------*/
void read_AZTCCPOS(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->tccmove[1].position);
}

/*------------------------------------------------------------------------------
  read_AZTCCVEL

  Reads the TCC interpretation of velocity PVT for azimuth.
------------------------------------------------------------------------------*/
void read_AZTCCVEL(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->tccmove[1].velocity);
}

/*------------------------------------------------------------------------------
  read_AZTCCTIM

  Reads the TCC interpretation of time PVT for azimuth.
------------------------------------------------------------------------------*/
void read_AZTCCTIM(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->tccmove[1].time);
}

/*------------------------------------------------------------------------------
  read_ROCPOS

  Reads the position "command".
------------------------------------------------------------------------------*/
void read_ROCPOS(HLOGREG reg)
{
	unsigned long value;

	assert(reg);
	value = (data->axis[2].position_hi << 16) + data->axis[2].position_lo;
	regPut(reg, value);
}

/*------------------------------------------------------------------------------
  read_ROCVOLT

  Reads the voltage "command".
------------------------------------------------------------------------------*/
void read_ROCVOLT(HLOGREG reg)
{
	assert(reg);

	regPut(reg, data->axis[2].voltage);
}

/*------------------------------------------------------------------------------
  read_ROCCWHL

  Reads the rotator's ccw hard limit.
------------------------------------------------------------------------------*/
void read_ROCCWHL(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i8.il0.rot_pos_380b_ccw);
}

/*------------------------------------------------------------------------------
  read_ROCCWSL

  Reads the rotator's ccw soft limit.
------------------------------------------------------------------------------*/
void read_ROCCWSL(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i8.il0.rot_pos_380a_ccw);
}

/*------------------------------------------------------------------------------
  read_ROCWHL

  Reads the rotator's cw hard limit.
------------------------------------------------------------------------------*/
void read_ROCWHL(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i8.il0.rot_neg_200b_cw);
}

/*------------------------------------------------------------------------------
  read_ROCWSL

  Reads the rotator's cw soft limit.
------------------------------------------------------------------------------*/
void read_ROCWSL(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i8.il0.rot_neg_200a_cw);
}

/*------------------------------------------------------------------------------
  read_ROICHG

  Reads the rotator's instrument change.
------------------------------------------------------------------------------*/
void read_ROICHG(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i1.il4.rot_inst_chg_a);
}

/*------------------------------------------------------------------------------
  read_ROMCPPOS

  Reads the MCP interpretation of position PVT for the rotator.
------------------------------------------------------------------------------*/
void read_ROMCPPOS(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->pvt[2].position);
}

/*------------------------------------------------------------------------------
  read_ROMCPVEL

  Reads the MCP interpretation of velocity PVT for the rotator.
------------------------------------------------------------------------------*/
void read_ROMCPVEL(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->pvt[2].velocity);
}

/*------------------------------------------------------------------------------
  read_ROMCPTIM

  Reads the MCP interpretation of time PVT for rotator.
------------------------------------------------------------------------------*/
void read_ROMCPTIM(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->pvt[2].time);
}

/*------------------------------------------------------------------------------
  read_ROMTRC

  Reads the rotator's motor current.
------------------------------------------------------------------------------*/
void read_ROMTRC(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i4.rot_1_current);
}

/*------------------------------------------------------------------------------
  read_ROMTRV

  Reads the rotator's motor voltage.
------------------------------------------------------------------------------*/
void read_ROMTRV(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->status.i4.rot_1_voltage);
}

/*------------------------------------------------------------------------------
  read_ROSTATE

  Reads the state variable for the rotator.
------------------------------------------------------------------------------*/
void read_ROSTATE(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->axis_state[2]);
}

/*------------------------------------------------------------------------------
  read_ROTCCPOS

  Reads the TCC interpretation of position PVT for the rotator.
------------------------------------------------------------------------------*/
void read_ROTCCPOS(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->tccmove[2].position);
}

/*------------------------------------------------------------------------------
  read_ROTCCVEL

  Reads the TCC interpretation of velocity PVT for the rotator.
------------------------------------------------------------------------------*/
void read_ROTCCVEL(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->tccmove[2].velocity);
}

/*------------------------------------------------------------------------------
  read_ROTCCTIM

  Reads the TCC interpretation of time PVT for rotator.
------------------------------------------------------------------------------*/
void read_ROTCCTIM(HLOGREG reg)
{
	assert(reg);
	regPut(reg, data->tccmove[2].time);
}
