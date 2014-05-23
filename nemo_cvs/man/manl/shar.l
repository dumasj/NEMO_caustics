.TH SHAR 1l
.\" $Header: /home/cvsroot/nemo/man/manl/shar.l,v 1.1.1.1 1988/06/09 01:33:34 teuben Exp $
.SH NAME
shar \- create shell archive file for extraction by /bin/sh
.SH SYNOPSIS
.B shar
[
.B \-b
] [
.BI \-i\| input_file
] [
.BI \-n\| seq_number
] [
.BI \-e\| seq_end
] [
.BI \-o\| output_file
] [
.BI \-t\| final_text
] [file...]
.SH DESCRIPTION
.I Shar
takes a list of files, and generates a
.IR /bin/sh
script that, when executed, will re-create those files in a different
directory or on a different machine.
The resultant script will use
.IR wc (1)
to do a mild error-check, and will warn about possibly-omitted
control characters.
.PP
.I Shar
generates scripts that will make directories and plain files.
It will not try to generate intermediate filenames, however, so
.RS
shar foo/bar/file
.RE
will not work.  Do
.RS
shar foo foo/bar foo/bar/file
.RE
instead.
.PP
The script is normally sent to standard output; the ``\-o'' option may be
used to specify an output filename.
This is designed to prevent filling up the disk if
.RS
shar * >SHAR
.RE
command is done; do
.RS
shar -o SHAR *
.RE
instead.
.PP
The list of files is normally specified on the command line; the ''\-i''
option may be used instead, to specify a file that contains the list
of files to pack up, one per line.
If the file name is ``-'' the standard input is read.
.PP
The ``\-b'' option says that all leading directory names should be stripped
from the file when they are packed into the archive.
For example,
.RS
shar -b /etc/termcap
.RE
creates an archive that, when executed, creates a file named
``termcap'' in the current directory, rather than overwrite the
host system file.
Note, however, that the scripts generated by
.I shar
normally refuse to overwrite pre-existing files.
.SS "Multi\-part Archives"
Most larger software packages are usually sent out in two or more shell
archives.
The ``\-n,'' ``\-e,'' and ``\-t'' options are used to make an archive
that is part of a series.
The individual archives are often called ``kits'' when this is done.
The ``\-n'' option specifies the archive number; the ``\-e'' option species
the highest number in the series.
When executed, the generated archives will then echo messages like
.RS
shar: End of archive 3 of 9.
.RE
at their end.
.PP
In addition, each shar will generate a file named
.IR ark X isdone .
Each script will contain a loop to check for the presence of these
files, and indicate to the recipient which archives still need to be
executed.
The ``\-t'' option may be used to give starting instructions to the recipient.
When the scripts determine that all the archives have been unpacked,
the text specified with this flag is displayed.
For example,
.RS
shar -n1 -k9 -t "Now do 'sh ./Configure'" *.c >SHAR
.RE
Adds commands to output the following when all the archives have been unpacked:
.RS
.nf
You have run all 9 archives.
Now do 'sh ./Configure'
.fi
.RE
.SH "SEE ALSO"
echo(1), findsrc(1l), makekit(1l), mkdir(1), sh(1), test(1), unshar(1l),
wc(1).