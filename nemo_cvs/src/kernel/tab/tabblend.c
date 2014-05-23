/*
 * TABBLEND: create (blended) lines, grid them, output table(s)
 *
 *
 *      21-dec-2011 V0.1    Created - shortest day of the year
 *      30-dec-2011 V0.6    proper definition of cell centered grid g=
 *      31-dec-2011 V0.7    bin=f implementing integration of function
 *
 */

#include <stdinc.h>	
#include <getparam.h>
#include <moment.h>
#include <grid.h>


#define MAXL       100
#define MAXP    100000

string defv[] = {                /* DEFAULT INPUT PARAMETERS */
  "x=0\n		 X coordinates of the line(s)",
  "a=1\n                 Amplitudes",
  "d=1\n                 FWHM of the lines (FWHM=2.355*sigma)",
  "s=-16:16:0.01\n       Points to sample",
  "g=-16:16:1\n          Points to grid",
  "fwhm=0\n              If non-zero, convolve points with this beam",
  "bin=f\n               Bin data (t) or Integrate (f) raw into the bins",
  "hanning=f\n           Optional hanning",
  "fft=f\n               Use FFT to compute spectrum (not implemented)", 
  "rms=0\n               Add gaussian noise",
  "seed=0\n              seed for random number generator",
  "mode=1\n              0 = output raw   1=output final",
  "VERSION=0.8\n	 1-jan-2012 PJT",
  NULL
};

string usage = "create (blended) gaussian profiles and grid them";

string cvsid="$Id: tabblend.c,v 1.8 2012/01/01 15:44:04 pteuben Exp $";


void integrate_bin(int ns, real *x, real *y, int ng, real *g, real *z);

nemo_main()
{
  real x[MAXL], a[MAXL], d[MAXL], d1[MAXL];
  real s[MAXP], g[MAXP], y[MAXP], z[MAXP], z1[MAXP], b[MAXP];
  real fwhm;
  Grid G;
  int i, j, il, ir, nx, na, nd, ns, ng;
  real arg, ds, dg;
  real fac1 = 2.3548;   /* 2sqrt(2ln2): fwhm to sigma factor */
  real fac2 = 1.6651;   /* 2sqrt(ln2)                        */
  bool Qraw;
  bool Qfft = getbparam("fft");
  bool Qhan = getbparam("hanning");
  bool Qbin = getbparam("bin");
  int  mode = getiparam("mode");
  int  seed = init_xrandom(getparam("seed"));
  real  rms = getdparam("rms");

  nx = nemoinpr(getparam("x"),x,MAXL);
  if (nx < 1) error("Error parsing x=%s",getparam("x"));

  na = nemoinpr(getparam("a"),a,MAXL);
  if (na < 1) error("Error parsing a=%s",getparam("a"));
  if (na > nx) error("Too many a=%s",getparam("a"));
  for (i=na; i<nx; i++) a[i] = a[i-1];
  
  nd = nemoinpr(getparam("d"),d,MAXL);
  if (nd < 1) error("Error parsing d=%s",getparam("d"));
  if (nd > nx) error("Too many d=%s",getparam("d"));
  for (i=nd; i<nx; i++) d[i] = d[i-1];
  for (i=0; i<nx; i++) d1[i] = fac2 / d[i];

  ns = nemoinpr(getparam("s"),s,MAXP);
  if (ns < 1) error("Error parsing s=%s",getparam("s"));
  ds = s[1]-s[0];  /* better be a uniform grid */

  ng = nemoinpr(getparam("g"),g,MAXP);
  if (ng < 1) error("Error parsing g=%s",getparam("g"));
  dg = g[1]-g[0];  /* better be a uniform grid */
  inil_grid(&G, ng, g[0]-0.5*dg, g[ng-1]+0.5*dg);

  /*  initialize half a gaussian as smoothing kernel */

  fwhm = getdparam("fwhm");
  if (fwhm > 0.0) {
    for (i=0; i<ns; i++) {
      arg = i * ds*fac2/fwhm;
      arg = arg*arg;
      if (arg < 100) 
	b[i] = exp(-arg);
      else
	b[i] = 0;
    }
  } 

  /* write out a header, report which lines used */

  printf("#  X    A   D\n");
  for (i=0; i<nx; i++) {
    printf("#  %g %g %g\n",x[i], a[i], d[i]);
  }
  printf("# fwhm=%g  rms=%g \n",fwhm,rms);
  if (Qhan) printf("# with hanning\n");
  if (Qbin) printf("# simple binning\n");
  if (mode==0) printf("# raw spectrum\n");
  if (mode==1) printf("# gridded spectrum\n");

  /* compute raw sampled spectrum */

  for (i=0; i<ns; i++) {     /* loop over all sample points */
    y[i] = 0;
    for (j=0; j<nx; j++) {   /* add in all lines */
      arg = (s[i]-x[j])*d1[j];
      arg = arg*arg;
      if (arg > 100) continue;   
      y[i] += a[j]*exp(-arg);
    }
  }

  /* smooth the raw spectrum with fwhm */

  if (fwhm > 0.0) {
    warning("Smoothing raw");
    for (i=0; i<ns; i++) z[i] = z1[i] = 0.0;
    for (i=0; i<ns; i++) {
      z[i]  += y[i];
      z1[i] += 1.0;
      for (j=1; j<ns; j++) {
	if (b[j] == 0.0) break;
	if (i+j<ns) {
	  z[i]  += y[i+j]*b[j];
	  z1[i] += b[j];
	}
	if (i-j>=0) {
	  z[i]  += y[i-j]*b[j];
	  z1[i] += b[j];
	}
      }
    }
    for (i=0; i<ns; i++) y[i] = z[i]/z1[i];
  }

  /* use an FFT engine ?  */

  if (Qfft) {
    warning("fft not implemented yet");
  }

  /* print out sampled (raw) spectrum  */
  /* optionally add noise here as well since this is the last step */
  /* one could argue the rms should be done before smoothing */
  /* but that also changes the actual rms */

  if (mode==0) {
    if (rms > 0.0) {        
      for (i=0; i<ns; i++)
	y[i] += grandom(0.0,rms);
    }
    for (i=0; i<ns; i++) 
      printf("%g  %g\n",s[i],y[i]);
  }

  /* grid the 's' to 'g' grid */
  if (Qbin && ds < dg) {
    // s[] is the input grid, on which y[] is defined
    for (i=0; i<ng; i++) z[i] = z1[i] = 0.0;
    for (i=0; i<ns; i++) {
      j = index_grid(&G, s[i]);
      if (j<0) continue;
      dprintf(2,"bin s->g: %d %g -> %d\n",i,s[i],j);
      z[j]  += y[i];
      z1[j] += 1.0;
    }
    for (i=0; i<ng; i++) {
      if (z1[i] > 0.0) z[i] /= z1[i];
    }
    // g[] is the output grid, on which z[] is now defined
  } else {
    integrate_bin(ns,s,y,ng,g,z);
  }


  /* Hanning */
  if (Qhan) {
    warning("Hanning");
    for (i=0; i<ng; i++) {
      il = MAX(i-1,0);
      ir = MIN(i+1,ng-1);
      z1[i] = 0.25 * z[il] + 0.5 * z[i] + 0.25 * z[ir];
    }
    for (i=0; i<ng; i++) z[i] = z1[i];
  } 


  /* add noise */
  if (rms > 0.0) {
    for (i=0; i<ng; i++)
      z[i] += grandom(0.0,rms);
  }

  if (mode==1) {
    for (i=0; i<ng; i++)
      printf("%g  %g\n",g[i],z[i]);
  }

  
}

/* 
 * some of this code relies on:
 *   - x and g increasing 
 *   - x and g are linear, each delta-x and delta-g, the same
 */


void integrate_bin(int ns, real *x, real *y, int ng, real *g, real *z)
{
  real x0,x1,g0,g1, dg,dx,fac;
  int i,j;
  
  dx = x[1]-x[0];
  dg = g[1]-g[0];
  g0 = g[0] - 0.5*dg;
  g1 = g[0] + 0.5*dg;

  if (g1 < x[0]) error("grid not inside the sample: %g < %g",g1,x[0]);

  /* set up integration and find first sample to contribute */

  i = j = 0;
  if (g0 < x[0] && g1 > x[1]) {     /* first grid has partial sample */
    x0 = x[0];
    x1 = x[1];
    dx = x1-x0;
    z[0] = (y[0]+y[1])*dx*0.5;
    i++;
  } else {                           /* first grid fully inside sample */
    // find first sample 
    while (x[i]<g0) i++;
    x0 = x[i-1];
    x1 = x[i];
    z[0] = (y[i-1]+y[i])*dx*0.5;
  }

  /* loop over all sample points (assumes ordered) integrate them into grid */

  while (i<ns) {
    x0 = x[i];
    x1 = x[i+1];
    dx = x1-x0;

    if (x0<g1 && x1>g1) {   /* interval split */
      dx = g1-x0;
      z[j] += 0.5*dx*(y[i+1]+y[i]);
      dprintf(2,"int-1a s->g: %d %g -> %d  (%g,%g)\n",i,x[i],j,g0,g1);
      j++;
      if (j==ng)  break;
      dx = x1-g1;
      g0 = g[j] - 0.5*dg;
      g1 = g[j] + 0.5*dg;
      z[j] = 0.0;
      dprintf(2,"int-1b s->g: %d %g -> %d  (%g,%g)\n",i,x[i],j,g0,g1);
    } else if (x0>=g1) {    /* full new interval */
      j++;
      if (j==ng)  break;
      g0 = g[j] - 0.5*dg;
      g1 = g[j] + 0.5*dg;
      z[j] = 0.0;
      dprintf(2,"int-2  s->g: %d %g -> %d  (%g,%g)\n",i,x[i],j,g0,g1);
    } else
      dprintf(2,"int-3  s->g: %d %g -> %d  (%g,%g)\n",i,x[i],j,g0,g1);

    z[j] += 0.5*dx*(y[i+1]+y[i]);
    i++;
  } 

  /* normalize */

  for (j=0; j<ng; j++)  z[j] /= dg;
}
