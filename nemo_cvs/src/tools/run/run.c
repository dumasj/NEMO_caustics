/*
 *  RUN:   generic edit config file and run a program in a (new) run directory
 *
 *    all arguments after "--" on the commandline are passed onto the editor
 *    which then replaces all "key=val" instances as directed.
 * 
 *    8-may-2011   original?
 *   17-sep-2013   use new run_ interface
 *
 */

#include <nemo.h>
#include <run.h>

string defv[] = {
  "exe=???\n          Executable to run",
  "ini=???\n          Ini file to edit",
  "dir=???\n          Directory in which to run",
  "log=\n             Logfile, if needed (> logfile)",
  "stdin=f\n          If true, read ini from stdin (< inifile)",
  "VERSION=0.2\n      17-sep-2013 PJT",
  NULL,
};

string usage="Generic frontend to edit an ini file and run programs";

string cvsid="$Id: run.c,v 1.2 2013/09/17 12:41:22 pteuben Exp $";


void nemo_main(void);
void edit_file(string inifile, int argc, string *argv);

void nemo_main()
{
  string exefile = getparam("exe");
  string inifile = getparam("ini");
  string dirname = getparam("dir");
  string logfile = getparam("log");
  bool usestdin = getbparam("stdin");
  char dname[256], cmd[256];
  int i, argc;
  string *argv;


  run_mkdir(dirname);

#if 0
  sprintf(dname,"%s/%s",outdir,parfile);
  parstr = stropen(dname,"w");
  fprintf(parstr,"%d\n",getiparam("numbs"));
  fprintf(parstr,"%s\n",getparam("aexpn"));
  fprintf(parstr,"%s\n",getparam("adiv"));
  fprintf(parstr,"%s\n",getparam("om0"));
  fprintf(parstr,"%s\n",getparam("vsca"));
  fprintf(parstr,"%s\n",getparam("hubble"));
  fprintf(parstr,"%d\n",getiparam("mond"));
  fprintf(parstr,"%d\n",getiparam("freq"));
  fprintf(parstr,"%d\n",getiparam("brand"));
  fprintf(parstr,"%s\n",getparam("au0"));
  strclose(parstr);
#endif

  sprintf(cmd,"cp %s %s/%s", inifile, dirname, inifile);
  run_sh(cmd);

  run_cd(dirname);

  argv = getargv(&argc);
  edit_file(inifile, argc, argv);

  sprintf(cmd,"%s ",exefile);
  if (usestdin) {
    strcat(cmd,"<"); 
    strcat(cmd,inifile);
    strcat(cmd," "); 
  }
  if (hasvalue("log")) {
    strcat(cmd,">"); 
    strcat(cmd,logfile);
  }
  run_sh(cmd);

}

void edit_file(string inifile, int argc, string *argv)
{
  int i;

  for (i=1; i<argc; i++) {
    dprintf(0,"EDIT: %s\n",argv[i]);
  }
}
