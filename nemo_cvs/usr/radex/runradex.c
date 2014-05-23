/*
 *  RUNRADEX:   run the RADEX code
 *
 *    This program only assembles the parameter to write the input parameter
 *    file, copies your input file to the properly named qmics.dat file,
 *    stuffs all of that in a (new) run directory, and runs the program in
 *    that directory.
 *
 */

#include <nemo.h>
#include <run.h>

string defv[] = {
  "mol=???\n             Input molecular data file [hco+.dat, from $RADEXDAT/]",
  "outdir=???\n          Run directory",
  "out=output.txt\n      Output filename",
  "fmin=50\n             Minimum freq [GHz]",
  "fmax=500\n            Maximum freq [GHz]",
  "tkin=20\n             Kinetic temperature [K]",
  "partner=H2\n          Partner(s) names (H2 p-H2 o-H2 e H He H+)",
  "density=1e4\n         Density of partner(s)",
  "tbg=2.73\n            Background temperature [K]",
  "cdmol=1e13\n          Molecular column density [cm^-2]",
  "deltav=1.0\n          Line width [km/s] ?",
  "VERSION=0.2\n         17-sep-2013 PJT",
  NULL,
};

string usage="RADEX frontend; see also http://www.sron.rug.nl/~vdtak/radex/index.shtml";

string cvsid="$Id: runradex.c,v 1.5 2013/09/17 12:36:07 pteuben Exp $";

#define MAXPARTNER   7

void nemo_main()
{
  int n;

  stream parstr, outstr;
  string molfile = getparam("mol");
  string outfile = getparam("out");
  string outdir  = getparam("outdir");
  string exefile = "radex";
  string datfile = "mol.dat";
  string parfile = "input.txt";
  string logfile = "radex.txt";
  char dname[256], cmd[256];

  real  fmin = getdparam("fmin");
  real  fmax = getdparam("fmax");
  real  tkin = getdparam("tkin");

  int   npartner;
  string *partner;
  real  density[MAXPARTNER];

  real  tbg = getdparam("tbg");
  real  cdmol = getdparam("cdmol");
  real  deltav = getdparam("deltav");

  partner = burststring(getparam("partner"),",");
  npartner = xstrlen(partner,sizeof(string))-1;
  if (npartner < 1 || npartner>MAXPARTNER) error("too few/many partners");
  n = nemoinpr(getparam("density"),density,MAXPARTNER);
  if (n != npartner) error("need matching numbers for partner= and density=");

  run_mkdir(outdir);
  sprintf(dname,"%s/%s",outdir,parfile);

  parstr = stropen(dname,"w");

  fprintf(parstr,"%s\n",molfile);
  fprintf(parstr,"%s\n",outfile);
  fprintf(parstr,"%g %g\n",fmin,fmax);
  fprintf(parstr,"%g\n",tkin);
  fprintf(parstr,"%d\n",npartner);
  for (n=0; n<npartner; n++) {
    fprintf(parstr,"%s\n",partner[n]);
    fprintf(parstr,"%g\n",density[n]);
  } 
  fprintf(parstr,"%g\n",tbg);
  fprintf(parstr,"%g\n",cdmol);
  fprintf(parstr,"%g\n",deltav);
  fprintf(parstr,"0\n");

  strclose(parstr);

  run_cd(outdir);
  sprintf(cmd,"%s < %s > %s ", exefile, parfile, logfile);
  run_sh(cmd);
}


