#! /usr/bin/perl
#
#  this is a section of perl code to setup perl (e.g. cgi-bin) scripts
#  to know the absolute location of NEMO and define all needed variables
# 
#  CAVEAT:  if you relocate the NEMO directory tree, you will need to 
#	    edit this file (or re-install in the new location)
#
#

$nemop1 ="/home/adam/NEMO/nemo_cvs";
$ENV{'NEMO'}="$nemop1";
$nemo_path = "${nemop1}/bin/";

$ENV{'PATH'} = "/usr/bin:/bin:${nemo_path}";   #Needed?
$ENV{'NEMOOBJ'}= "${nemop1}/obj";
$ENV{'NEMOLIB'}= "${nemop1}/lib";
$ENV{'NEMOINC'}= "${nemop1}/inc";
$ENV{'NEMODAT'}= "${nemop1}/data";

$ENV{'BTRPATH'}= "${nemop1}/obj/bodytrans";
$ENV{'POTPATH'}= "${nemop1}/obj/potential";

## $ENV{'PGPLOT_FONT'} = "/usr/local/astromake/opt/pgplot/grfont.dat";
## $ENV{'LD_LIBRARY_PATH'} = "${nemop1}/lib/:/usr/local/astromake/opt/pgplot";$ENV{'PGPLOT_BACKGROUND'} = "white";$ENV{'PGPLOT_FOREGROUND'} = "black";


