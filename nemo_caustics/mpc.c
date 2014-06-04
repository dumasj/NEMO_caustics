/*
 * mpc.c: procedures for intializing and calculating the forces and
 *             	potential of a combination of 3 potentials:
 *			Miyamoto Nagai Disk
 *			Plummer Sphere
 *			Caustic Ring Halo
 * Version 1
 * 30-may-14
 * jdumas added flags on minimum values for rho and rpar to prevent nans at x, y, z=0
 */

#include <stdinc.h>
#include <potential_float.h>
#include <math.h>
#include <stdio.h>
#include <complex.h>

local double omega = 0.0;		/* pattern speed */
local double miya_ascal = 0.0;
local double miya_bscal = 1.0;
local double miya_mass  = 1.0;
local double plu_rc   = 1.0;
local double plu_mass = 1.0;
local double vhalo = 1.0;
local double q = 1.0;
local double d = 1.0;
local const double G = 1.0;

//Caustic flow number                   1,     2,     3,     4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,   15,   16,   17,   18,   19,   20    
local const double a_n[]    = {1.0,  40.1,  20.1,  13.6,  10.4,  8.4,  7.0,  6.1,  5.3,  4.8,  4.3,  4.0,  3.7,  3.4,  3.2,  3.0,  2.8,  2.7,  2.5,  2.4,  2.3};
local const double V_n[]    = {1.0,   517,   523,   523,   523,  522,  521,  521,  520,  517,  515,  512,  510,  507,  505,  503,  501,  499,  497,  496,  494};
local const double rate_n[] = {1.0,    53,    23,    14,    10,  7.8,  6.3,  5.3,  4.5,  3.9,  3.4,  3.1,  2.8,  2.5,  2.3,  2.1,  2.0,  1.8,  1.7,  1.6,  1.5};
local const double p_n[]    = {1.0,   0.3,   0.3,   1.0,   0.3, 0.15, 0.12,  0.6, 0.23, 0.41, 0.25, 0.19, 0.17, 0.11, 0.09, 0.09, 0.09, 0.09, 0.09, 0.09, 0.09};

local const double ONE_THIRD = 1.0 / 3.0;
local const double complex c1  = (double complex) 1.0;
local const double complex c2  = (double complex) 2.0;
local const double complex c3  = (double complex) 3.0;
local const double complex c4  = (double complex) 4.0;
local const double complex c12 = (double complex) 12.0;
local const double complex c05 = (double complex) 0.5;
local const double II = (double complex) 1.0*I;
local const double TOLERANCE  = 0.00001;
local const double TOLERANCE2 = 0.001;

local const int X = 0;
local const int Y = 1;
local const int Z = 2;

// Now let's compute the values of T at which the various poles cross the real axis
inline double complex f1(double complex x) {
  return (2.0 + 4.0*x) / 3.0;
}

inline double complex f2(double complex x, double complex z) {
  return 4.0 * ((1.0-x) * (1.0-x) - 3.0 * z*z);
} 

inline double complex f3(double complex x, double complex z) {
  return -128.0 * cpow(x-1.0,3.0) - 1728.0 * z*z - 1152.0 * (x-1.0) * z*z;
}

// for convenience define f4, which appears frequently in the roots
double complex f4(double complex x, double complex z) {
  return cpow( (f3(x,z) + csqrt( -256.0*f2(x,z)*f2(x,z)*f2(x,z) + f3(x,z)*f3(x,z) )) / 2.0, ONE_THIRD );
}

// f6 my suspicion is that the gravitational field in r and z constitute the real and complex components of the individual terms in f5; so let's do a test to see if this s right
inline double complex f5(double complex x, double complex z, double complex T) {
  return (csqrt(c2*T - c1 + x - II*z)) / c2;
}


inline double complex T1(double complex x, double complex z) {
  if (creal(x) >= -0.125 && cabs(f4(x,z)) < TOLERANCE2) {
    double complex temp = cpow((-1.0 - 6.0*x + 15.0*x*x -8.0*x*x*x), ONE_THIRD) / 3.0;
    return c05 * (c1 - csqrt(f1(x) / 2.0 + temp) - csqrt( f1(x) - temp - c2*x/csqrt(f1(x) / 2.0 + temp) ));
  } else {
    double complex temp = csqrt(f1(x) / 2.0 + f2(x,z) / (c3 * f4(x,z)) + f4(x,z) / c12);
    return c05 * (c1 - temp - csqrt( f1(x) - f2(x,z) / (c3 * f4(x,z)) - f4(x,z) / c12 - c2 * x / temp ));
  }
}

inline double complex T2(double complex x, double complex z) {
  if (creal(x) >= -0.125 && cabs(f4(x,z)) < TOLERANCE2) {
    double complex temp = cpow(( -1.0 - 6.0 * x + 15.0 * x*x - 8.0 * x*x*x), ONE_THIRD) / 3.0;
    return c05 * (c1 - csqrt(f1(x) / 2.0 + temp) + csqrt( f1(x) - temp - c2 * x / sqrt(f1(x) / 2.0 + temp) ));
  } else {
    double complex temp = csqrt(f1(x) / 2.0 + f2(x,z) / (c3 * f4(x,z)) + f4(x,z) / c12);
    return c05 * (c1 - temp + csqrt( f1(x) - f2(x,z) / (c3 * f4(x,z)) - f4(x,z) / c12 - c2 * x / temp ));
  }
}

inline double complex T3(double complex x, double complex z) {
  if (creal(x) >= -0.125 && cabs(f4(x,z)) < TOLERANCE2) {
    double complex temp = cpow((-1.0 - 6.0 * x + 15.0 * x*x - 8.0 * x*x*x), ONE_THIRD)/3.0;
    return c05 * (c1 + csqrt(f1(x) / 2.0 + temp) - csqrt( f1(x) - temp + c2 * x / csqrt(f1(x) / 2.0 + temp) ));
  } else {
    double complex temp = csqrt(f1(x) / 2.0 + f2(x,z) / (c3 * f4(x,z)) + f4(x,z) / c12);
    return c05*(c1 + temp - csqrt( f1(x) - f2(x,z) / (c3 * f4(x,z)) - f4(x,z) / c12 + c2 * x / temp ));
  }
}

inline double complex T4(double complex x, double complex z) {
  if (creal(x) >= -0.125 && cabs(f4(x,z))<TOLERANCE2) {
    double complex temp = cpow((-1.0 - 6.0 * x + 15.0 * x*x - 8.0 * x*x*x), ONE_THIRD)/3.0;
    return c05 * (c1 + csqrt(f1(x) / 2.0 + temp) + csqrt( f1(x) - temp + c2 * x / csqrt(f1(x) / 2.0 + temp) ));
  } else {
    double complex temp = csqrt(f1(x) / 2.0 + f2(x,z) / (c3 * f4(x,z)) + f4(x,z) / c12);
    return c05 * (c1 + temp + csqrt(f1(x) - f2(x,z) / (c3*f4(x,z)) - f4(x,z) / c12 + c2 * x / temp ));
  }
}

//*************CHANGE NAMES FOR Z, X*******************
//*************CHANGE NAMES FOR Z, X*******************
//*************CHANGE NAMES FOR Z, X*******************
//*************CHANGE NAMES FOR Z, X*******************
//*************CHANGE NAMES FOR Z, X*******************
//*************CHANGE NAMES FOR Z, X*******************
//*************CHANGE NAMES FOR Z, X*******************
//*************CHANGE NAMES FOR Z, X*******************
//*************CHANGE NAMES FOR Z, X*******************
//*************CHANGE NAMES FOR Z, X*******************
//*************CHANGE NAMES FOR Z, X*******************
//*************CHANGE NAMES FOR Z, X*******************
//*************CHANGE NAMES FOR Z, X*******************
//*************CHANGE NAMES FOR Z, X*******************
//*************CHANGE NAMES FOR Z, X*******************
//                      |
//                      |
//                      v
//  if ( (count != 4) && (count != 2) )
void gfield_close(double rho, double z, int n, double *rfield, double *zfield) {

  z = (double complex)(z / p_n[n]);

  double complex x = (double complex)(( rho - a_n[n] ) / p_n[n]);

  // two cases; within the caustic, all four roots are real, so we use T1 for d4 and T2,3,4 for d3
  // outside the caustic there should be two real roots and two complex roots
  // the vector intlimits hold the limits of integration; for the first case, intlimits is of length four; second, length two

  double im[4] = {cabs(cimag(T1(x,z))), cabs(cimag(T2(x,z))), cabs(cimag(T3(x,z))), cabs(cimag(T4(x,z)))};
  double re[4] = {creal(T1(x,z))      , creal(T2(x,z))      , creal(T3(x,z))      , creal(T4(x,z))};

  int count = 0;
  int i, j;
  double temp;

  // get only the reals we want (Julie will make better comments)
  for (i = 0; i < 4; ++i) {
    if (im[i] < TOLERANCE) {
      re[count] = re[i];    
      ++count; // count the roots - there should be 2 or 4
    }
  }

  // sort in ascending order
  for (i = 0; i < count - 1; ++i) {
    for (j = i + 1; j < count; ++j) //LOSING THIS CURLY BRACE A GOOD IDEA?*******
      if (re[i] > re[j]) {
        temp  = re[i];
        re[i] = re[j];
        re[j] = temp;
      }
  }

  // roots stored in re[] and re2[] BAD COMMENT IS CONFUSING******
  double factor = -8.0 * M_PI * G * rate_n[n] * 4498.6589 / (rho * V_n[n] * 1.0226831);

  *rfield += factor * ( creal(f5(x,z,re[1])) - creal(f5(x,z,re[0])) - 0.5 ); // it seems that the answer is just off by 0.5, so we subtract it by hand
  *zfield += factor * ( cimag(f5(x,z,re[1])) - cimag(f5(x,z,re[0])) );

  if (count == 4){
    *rfield += factor * ( creal(f5(x,z,re[3])) - creal(f5(x,z,re[2])) );
    *zfield += factor * ( cimag(f5(x,z,re[3])) - cimag(f5(x,z,re[2])) );	
  }

}

// calculate gfield far away for nth flow
void gfield_far(double rho, double z, int n, double *rfield, double *zfield) {

  // simulation units are kpc, gyr, ms=222288.47*Ms
  double r_squared = rho*rho + z*z;
  double shift = a_n[n] + (p_n[n] / 4.0);
  double A_n = (8.0 * M_PI * G * rate_n[n] * 4498.6589) / (V_n[n] * 1.0226831);  //convert from M_sun/yr*sr to m_sim/gyr*sr with 4498 and km/s to kpc/gyr with 1.0226

  // caustic radius shifted by 0.25 so that g goes to zero beyond a_n[n]
  double s = hypot(r_squared - shift*shift, 2.0 * shift * z);
  double factor = -A_n / ( s * (2.0 * shift*shift + s) );

  // result in kpc/gyr^2
  *rfield += factor * (r_squared * rho - shift * shift * rho);
  *zfield += factor * (r_squared * z   + shift * shift * z);
}


void inipotential (int *npar, double *par, string name) {
  int n;
  n = *npar;
  if (n>0) omega = par[0];
  if (n>1) miya_ascal = par[1];
  if (n>2) miya_bscal = par[2];
  if (n>3) miya_mass  = par[3];
  if (n>4) plu_rc   = par[4];
  if (n>5) plu_mass = par[5];
  if (n>6) vhalo = par[6];
  if (n>7) q = par[7];
  if (n>8) d = par[8];
  if (n>9) warning("mpl: only first 9 parameters recognized");
}

void apply_miyamoto_pot(double *pos, double *acc, double *pot) {
  double qpar, apar, spar;
  qpar = hypot(pos[Z], miya_bscal);
  apar = miya_ascal + qpar;
  spar = pos[X]*pos[X] + pos[Y]*pos[Y] + (miya_ascal + qpar)*(miya_ascal + qpar);

  *pot -= miya_mass / sqrt(spar);

  acc[X] -= miya_mass * pos[X] / pow(spar, 1.5);
  acc[Y] -= miya_mass * pos[Y] / pow(spar, 1.5);
  acc[Z] -= miya_mass * pos[Z] * apar / (qpar * pow(spar, 1.5));
}

void apply_plummer_pot(double *pos, double *acc, double *pot) {
  double ppar, rpar;
  ppar = sqrt(pos[X]*pos[X] + pos[Y]*pos[Y] + pos[Z]*pos[Z]) + plu_rc;
  rpar = ppar - plu_rc;
  if (rpar < 0.000001) // mke sure rpar != zero (causes nan in accx accy accz at origin)
    rpar = 0.000001;

  *pot -= plu_mass / ppar;

  acc[X] -= plu_mass * pos[X] / (rpar * ppar * ppar);
  acc[Y] -= plu_mass * pos[Y] / (rpar * ppar * ppar);
  acc[Z] -= plu_mass * pos[Z] / (rpar * ppar * ppar);
}

void apply_caustic_pot(double *pos, double *acc, double *pot) {
  double rho, z, rfield, zfield;
  double r, l, tr, tl; //EXPLAIN THESE PL0X
  int n;

  rfield = 0.0;
  zfield = 0.0;
  rho = sqrt(pos[X]*pos[X] + pos[Y]*pos[Y]);
  z = pos[Z];

  // rho cannot be zero (causes nan in near field and ax and ay at origin)
  if (rho < 0.000001) {
    rho = 0.000001;
  }
  for (n = 1; n <= 20; ++n) {
    // tricusp
    r  = (3.0 - sqrt( 1.0 + (8.0 / p_n[n]) * (rho - a_n[n]) )) / 4.0;
    l  = (3.0 + sqrt( 1.0 + (8.0 / p_n[n]) * (rho - a_n[n]) )) / 4.0;
    tr = 2.0 * p_n[n] * sqrt(pow(r, 3.0) * (1.0 - r));
    tl = 2.0 * p_n[n] * sqrt(pow(l, 3.0) * (1.0 - l));

    // are we in the region of space where we use the close approximation?
    if ( (z <= tr && z >= 0.0 && rho >= a_n[n] && rho <= a_n[n] + p_n[n])
     || (z >= tl  && z <= tr  && rho >= (a_n[n] - p_n[n] / 8.0) && rho <= a_n[n])
     || (z >= -tr && z <= 0.0 && rho >= a_n[n] && rho <= a_n[n] + p_n[n])
     || (z <= -tl && z >= -tr && rho >= (a_n[n] - p_n[n] / 8.0) && rho <= a_n[n]) )
    {
      gfield_close(rho, z, n, &rfield, &zfield);
    } else {
      gfield_far(rho, z, n, &rfield, &zfield);        
    }
  }
  for (n = 1; n <= 20; ++n) {
    double r_squared = rho*rho + z*z;
    double shift = a_n[n] + p_n[n] / 4.0;  //caustic radius shifted by 0.25 so that g goes to zero beyond a_n[n]
    double A_n = (8.0 * M_PI * G * rate_n[n] * 4498.6589) / (V_n[n] * 1.0226831);  //convert from M_sun/yr*sr to m_sim/gyr*sr with 4498 and km/s to kpc/gyr with 1.0226
    double s = hypot(r_squared - shift*shift, 2.0 * shift *z);
    *pot += (A_n / 2.0) * log(1.0 + ( s / (2.0 * a_n[n]*a_n[n]) ));
  }

  acc[X] += rfield * pos[X] / rho;
  acc[Y] += rfield * pos[Y] / rho;
  acc[Z] += zfield;

}
void potential_double(int *ndim, double *pos, double *acc, double *pot, double *time) {
  *pot = 0;
  acc[X] = 0;
  acc[Y] = 0;
  acc[Z] = 0;
  apply_miyamoto_pot(pos, acc, pot);
  apply_plummer_pot(pos, acc, pot);
  apply_caustic_pot(pos, acc, pot);
}

