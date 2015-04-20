
#include "calcPos.h"
#include </usr/include/SpiceUsr.h>
#include </usr/include/SpiceZfc.h>

/*
ilumin_c - finds the illumination angles at a specified surface point of a target body.
phaseq_c - computes the apparent phase angle between the centers of target, observer, and illuminator ephemeris objects.
Brief Example:
The following example computes the illumination angles for a point
specified using planetocentric coordinates, observed by MGS:
 */
SpiceDouble r = 3390.42;
SpiceDouble lon = 175.30;
SpiceDouble lat = -14.59;
SpiceDouble point[3];
SpiceDouble et;
SpiceDouble srfvec[3];
SpiceDouble trgepc;
SpiceDouble phase, solar, emissn;

/*
 load kernels: LSK, PCK, planet/satellite SPK
 and MGS spacecraft SPK
 */
furnsh_c ( "naif0008.tls" );
furnsh_c ( "mars_iau2000_v0.tpc" );
furnsh_c ( "mar063.bsp" );
furnsh_c ( "mgs_ext22.bsp" );
/*
 convert planetocentric r/lon/lat to Cartesian vector
 */
latrec_c( r, lon * rpd_c(), lat * rpd_c(), point );
/*
 convert UTC to ET
 */
str2et_c ( "2006 JAN 31 01:00", &et );
/*
 compute illumination angles
 */
ilumin_c ( "Ellipsoid", "MARS", et, "IAU_MARS",
          "LT+S", "MGS", point,
          &trgepc, srfvec, &phase, &solar, &emissn );