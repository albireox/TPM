#include "slalib.h"
#include "slamac.h"
void slaPlante ( double date, double elong, double phi, int jform,
                 double epoch, double orbinc, double anode, double perih,
                 double aorq, double e, double aorl, double dm,
                 double* ra, double* dec, double* r, int* jstat )
/*
**  - - - - - - - - - -
**   s l a P l a n t e
**  - - - - - - - - - -
**
**  Topocentric apparent RA,Dec of a Solar-System object whose
**  heliocentric orbital elements are known.
**
**  Given:
**     date    double   MJD of observation (JD - 2400000.5)
**     elong   double   observer's east longitude (radians)
**     phi     double   observer's geodetic latitude (radians)
**     jform   int      choice of element set (1-3, see Note 2, below)
**     epoch   double   epoch of elements (TT MJD)
**     orbinc  double   inclination (radians)
**     anode   double   longitude of the ascending node (radians)
**     perih   double   longitude or argument of perihelion (radians)
**     aorq    double   mean distance or perihelion distance (AU)
**     e       double   eccentricity
**     aorl    double   mean anomaly or longitude (radians, jform=1,2 only)
**     dm      double   daily motion (radians, jform=1 only )
**
**  Returned:
**     ra,dec  double   RA, Dec (topocentric apparent, radians)
**     r       double   distance from observer (AU)
**     jstat   int      status:  0 = OK
**                              -1 = illegal jform
**                              -2 = illegal e
**                              -3 = illegal aorq
**                              -4 = illegal dm
**                              -5 = failed to converge
**
**  Notes:
**
**  1  date is the instant for which the prediction is required.
**     It is in the TT timescale (formerly Ephemeris Time, ET)
**     and is a Modified Julian Date (JD-2400000.5).
**
**  2  The longitude and latitude allow correction for geocentric
**     parallax.  This is usually a small effect, but can become
**     important for Earth-crossing asteroids.  Geocentric positions
**     can be generated by appropriate use of the routines slaEvp and
**     slaPlanel.
**
**  3  The elements are with respect to the J2000 ecliptic and equinox.
**
**  4  Three different element-format options are available:
**
**     Option jform=1, suitable for the major planets:
**
**     epoch  = epoch of elements (TT MJD)
**     orbinc = inclination i (radians)
**     anode  = longitude of the ascending node, big omega (radians)
**     perih  = longitude of perihelion, curly pi (radians)
**     aorq   = mean distance, a (AU)
**     e      = eccentricity, e
**     aorl   = mean longitude L (radians)
**     dm     = daily motion (radians)
**
**     Option jform=2, suitable for minor planets:
**
**     epoch  = epoch of elements (TT MJD)
**     orbinc = inclination i (radians)
**     anode  = longitude of the ascending node, big omega (radians)
**     perih  = argument of perihelion, little omega (radians)
**     aorq   = mean distance, a (AU)
**     e      = eccentricity, e
**     aorl   = mean anomaly M (radians)
**
**     Option jform=3, suitable for comets:
**
**     epoch  = epoch of perihelion (TT MJD)
**     orbinc = inclination i (radians)
**     anode  = longitude of the ascending node, big omega (radians)
**     perih  = argument of perihelion, little omega (radians)
**     aorq   = perihelion distance, q (AU)
**     e      = eccentricity, e
**
**  5  Unused elements (dm for jform=2, aorl and dm for jform=3) are
**     not accessed.
**
**  Called: slaGmst,  slaDt,  slaEpj,  slaPvobs,  slaPrenut,
**          slaPlanel,  slaDmxv,  slaDcc2s,  slaDranrm
**
**  Last revision:   25 June 1997
**
**  Copyright P.T.Wallace.  All rights reserved.
*/

/* Light time for unit distance (sec) */
#define TAU 499.004782

{
   int i;
   double dvb[3], dpb[3], vsg[6], vsp[6], v[6],
          rmat[3][3], vgp[6], stl, vgo[6],
          dx, dy, dz, tl;


/* Sun to geocentre (J2000). */
   slaEvp( date, 2000.0, dvb, dpb, &vsg[3], &vsg[0] );

/* Sun to planet (J2000). */
   slaPlanel ( date, jform, epoch, orbinc, anode, perih, aorq,
               e, aorl, dm, vsp, jstat );

/* Geocentre to planet (J2000). */
   for ( i = 0; i < 6; i++ ) {
      v[i] = vsp[i] - vsg[i];
   }

/* Precession and nutation to date. */
   slaPrenut ( 2000.0, date, rmat );
   slaDmxv ( rmat, v, vgp );
   slaDmxv ( rmat, &v[3], &vgp[3] );

/* Geocentre to observer (date). */
   stl = slaGmst ( date - slaDt ( slaEpj ( date ) ) / 86400.0 ) + elong;
   slaPvobs ( phi, 0.0, stl, vgo );

/* Observer to planet (date). */
   for ( i = 0; i < 6; i++ ) {
      v[i] = vgp[i] - vgo[i];
   }

/* Geometric distance (AU). */
   dx = v[0];
   dy = v[1];
   dz = v[2];
   *r = sqrt ( dx * dx + dy * dy + dz * dz );

/* Light time (sec). */
   tl = *r * TAU;

/* Correct position for planetary aberration. */
   for ( i = 0; i < 3; i++ ) {
      v[i] = v[i] - tl * v[i+3];
   }

/* To RA,Dec. */
   slaDcc2s ( v, ra, dec );
   *ra = slaDranrm ( *ra );
   return;
}
