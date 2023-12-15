// program:	Yaroze Offset Calculator
// mainfile:	yoc.c
//
// file:	yoc.c
// created:	15 March 1998
// author:	Elliott Lee
// company:	Protocol Software
// e-mail:	tenchi@netmagic.net
// url:		http://www.netmagic.net/~tenchi
//
// copyright:	(C) 1998 Elliott Lee
// copyright:	Portions (C) 1998 Sony Computer Entertainment Inc.
// copyright:   Original idea by Don Yang.
//
// description: Calculates the offsets and generates 2 files for you.
//
// compiler:	Borland Turbo C++ 3.0
//
// notes:	This is a quick-n-dirty script.  Not much documentation
//		is available.

// ......................................................................
// GENERAL INCLUDES
// ......................................................................
#include <conio.h>
#include <dos.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ......................................................................
// GLOBAL VARS / DEFINES
// ......................................................................
#define	ALIGNMENT	(0x10)
#define BOUND_MAX	(0x801efffflu)
#define BOUND_MIN	(0x80090000lu)
#define	NUM_ARGS	4
#define PREFIX_DLOAD	"local dload"
#define PREFIX_HEADER	"#define"
#define PROG_VER	"1.0"
#define WBUF		2048

char	*WorkingBuf;

// ......................................................................
// PROTOTYPES
// ......................................................................
void		CheckArgs( int argc, char *argv[], unsigned long *base,
		  char *flist, char *fauto, char *fh );
char 		*EliminateSpaces( char *s );
void		ExitError( char *msg );
int		FileExists( char *f );
unsigned long	HexStr2ULong( char *str );
int 		IsBlank( char *s );
int 		IsComment( char *s );
int		IsHex( char *num );
void		OverwritePrompt( char *f );
int 		ProcessList( long unsigned base, char *filelist,
		char *fileauto, char *fileheader, char *workbuffer,
		int bufsize );
void 		ShowHelp( void );

// ......................................................................
// in     number of command line args, command line argument pointer,
//	  base, list file name, auto file name, C header file name
// out    .
// desc   checks to make sure this is properly formatted!
// notes  .
// ......................................................................
void CheckArgs( int argc, char *argv[], unsigned long *base,
	char *flist, char *fauto, char *fh )
{
// seeking help?
if( strcmpi(argv[1],"-h")==0 )
  {
  ShowHelp();
  exit(0);
  }  // if( strcmpi(argv[1],"-h") )

// right number of args?
if( argc!=(NUM_ARGS+1) )
  ExitError( "Wrong number of arguments!" );

// is the first one a hex number?
if( !IsHex(argv[1]) )
  ExitError( "First argument should be the <hex start addr>." );

// convert the first string to hex!
*base = HexStr2ULong(argv[1]);
if( *base==0 )
  ExitError( "Only put in hex digits; no \"0x\" or \"h\"!" );

// input exist?
if( !FileExists(flist) )
  ExitError( "List file does not exist!" );

// duplicate names?
if( strcmpi(fauto,flist)==0 )
  ExitError( "The <auto file> cannot be the same as the list file!" );
if( strcmpi(fh,flist)==0 )
  ExitError( "The <header file> cannot be the same as the list file!" );
if( strcmpi(fh,fauto)==0 )
  ExitError( "The <auto file> and <header file> cannot be the same!" );

// overwrite?
OverwritePrompt(fauto);
OverwritePrompt(fh);
}

// ......................................................................
// in     pointer to string
// out    pointer to first non-space character in string
// desc   eliminates preceeding string spaces
// notes  I define a space as characters c<33.  This includes spaces,
//	  tabs, and newlines, BUT not NULLs---because NULLs are string
//	  terminators...
// ......................................................................
char *EliminateSpaces( char *s )
{
while( *s>0 && *s<33 )
  s++;

return(s);
}

// ......................................................................
// in     message string
// out    .
// desc   prints the message, prints the usage, and then quits
// notes  .
// ......................................................................
void ExitError( char *msg )
{
printf( "ERROR: %s\n\n",msg );
printf( "Usage: yoc <hex start addr> <list file> <auto file> " \
	  "<header file>\n\n" \
	"Use \"yoc -h\" for help.\n" );

fcloseall();

exit(0);
}

// ......................................................................
// in     file name
// out    1=exists | 0=doesn't
// desc   test to see if the file exists
// notes  .
// ......................................................................
int FileExists( char *f )
{
FILE	*h;

h=fopen(f,"rt");
if( h==NULL )
  return(0);

fclose(h);
return(1);
}

// ......................................................................
// in     string
// out    1=yes | 0=no
// desc   check if the string is just full of space characters
// notes  .
// ......................................................................
int IsBlank( char *s )
{
while( *s!=0 )
  {
  if( *s>=33 )
    return(0);

  s++;
  }  // while( *s!=0 )

return(1);
}

// ......................................................................
// in     string
// out    1=yes | 0=no
// desc   check if the string is a comment
// notes  .
// ......................................................................
int IsComment( char *s )
{
s=EliminateSpaces(s);

if( *s=='#' )
  return(1);
else
  return(0);
}

// ......................................................................
// in     number string
// out    1=yes | 0=no
// desc   check if the number string is a hexadecimal number
// notes  .
// ......................................................................
int IsHex( char *num )
{
while( *num!=0 )
  {
  if( (*num>='0' && *num<='9') || (*num>='a' && *num<='f') ||
    (*num>='A' && *num<='F') )
    num++;
  else
    return(0);
  }  // while( *num!=0 )

return(1);
}

// ......................................................................
// in     hex number string
// out    unsigned long | 0=non-hex number/or 0 entered
// desc   converts a hex string to an unsigned long
// notes  don't put in a number that's too long or you'll overflow the
//	  buffer.
// ......................................................................
unsigned long HexStr2ULong( char *str )
{
unsigned long buf;

buf=0;
while( *str!=0 )
  {
  // shift the buffer over to make room for the new digit!
  buf=buf<<4;

  if( *str>='0' && *str<='9' )
    {
    // add normal number
    buf += *str-'0';
    }  // if( *str>='0' && *str<='9' )
  else
    {
    if( *str>='a' && *str<='f' )
      {
      // add a hex (lowercase)
      buf += 10+(*str-'a');
      }  // if( *str>='a' && *str<='f' )
    else
      {
      if( *str>='A' && *str<='F' )
	{
	// add a hex (uppercase)
	buf += 10+(*str-'A');
	}  // if( *str>='A' && *str<='F' )
      else
	return(0);
      }  // else if( *str>='a' && *str<='f' )
    }  // else if( *str>='0' && *str<='9' )

  // next character please
  str++;
  }  // while( *str!=0 )

return(buf);
}

// ......................................................................
// in     name of file
// out    .
// desc   puts up an overwrite prompt if the file exists; exits on "No"
// notes  .
// ......................................................................
void OverwritePrompt( char *f )
{
char	c;

if( !FileExists(f) )
  return;

printf( "YOC will overwrite \"%s\".  Continue?  [yN] ",f );
c=getche();
printf( "\n" );

if( c=='y' || c=='Y' )
  return;

exit(0);
}

// ......................................................................
// in     base address, list file, auto file, header file, working
//	  buffer, size of work buffer
// out    1=successful | 0=failed
// desc   processes the list file to create the auto and header files
// notes  .
// ......................................................................
int ProcessList( long unsigned base, char *filelist, char *fileauto,
	char *fileheader, char *workbuffer, int bufsize )
{
FILE	*flist,*fauto,*fheader,*fdata;
long	linestotal=0,
	linesdata=0,
	lineserrors=0;
char	*definename, *filename;
long unsigned start=base,fsize=0,bytestotal=0;

// open our handles!
flist=fopen(filelist,"rt");
if( flist==NULL )
  ExitError( "Can't open list file." );
fauto=fopen(fileauto,"wt+");
if( fauto==NULL )
  ExitError( "Can't write/create auto file." );
fheader=fopen(fileheader,"wt+");
if( fheader==NULL )
  ExitError( "Can't write/create header file." );

// add some heading info
fprintf( fheader,
	"/* These constants automatically calculated with YAC %s  */\n" \
	"/* (c) 3/1998 Elliott Lee (tenchi@netmagic.net).          */\n" \
	"/*                                                        */\n" \
	"/* Include this file in your main code source.  e.g.      */\n" \
	"/*                                                        */\n" \
	"/*      #include \"%s\"        */\n" \
	"/*                                                        */\n" \
	"/* Base begins at 0x%0.8x                              */\n\n",
	PROG_VER,fileheader );

// check bounds
if( base<BOUND_MIN )
  {
  printf( "\n\t*** WARNING ***\n"
	"\tBase address is lower than %lx!\n\n",BOUND_MIN );
  }  // if( base<BOUND_MIN )

while( !feof(flist) )
  {
  // get a line!
  fgets(workbuffer,bufsize,flist);
  linestotal++;

  // make sure we're dealing with a command here...
  workbuffer=EliminateSpaces(workbuffer);
  if( IsBlank(workbuffer) || IsComment(workbuffer) )
    continue;
  linesdata++;

  // okey, now separate this into two parameters!

  // define name
  definename=workbuffer;

  // seek to end of this word
  while( *workbuffer!=0 && *workbuffer>=33 )
    workbuffer++;

  // did we run out of chars?
  if( *workbuffer==0 )
    {
    printf( "Line %ld: Missing file name (2nd parameter).\n",linestotal );
    lineserrors++;
    continue;
    }  // if( *workbuffer==0 )

  // we have landed on the end of this word.  change this character to
  // a null, advance to the next character, and then seek to the next
  // word/end-of-string
  *workbuffer=0;
  workbuffer=EliminateSpaces(workbuffer+1);

  // are we at the end?
  if( *workbuffer==0 )
    {
    printf( "Line %ld: Missing file name (2nd parameter).\n",linestotal );
    lineserrors++;
    continue;
    }  // if( *workbuffer==0 )

  // okay, set the file name
  filename=workbuffer;

  // seek to end of word
  while( *workbuffer!=0 && *workbuffer>=33 )
    workbuffer++;

  // are we at the end of the string?
  if( *workbuffer!=0 )
    {
    // hm... no, we're not.  looks like some straggler characters.
    // let's drop the NULL here to terminate the file name, but keep
    // checking for superfluous characters.
    *workbuffer=0;
    workbuffer=EliminateSpaces(workbuffer+1);

    // is the current character the end of string?  if not, then print
    // a friendly warning message.
    if( *workbuffer>0 )
      printf( "Line %ld: Extra characters on line; ignored.\n",linestotal );
    }  // if( *workbuffer!=0 )

  // okay, now let's get the size of the file!
  if( !FileExists(filename) )
    {
    printf( "Line %ld: File not found: \"%s\".\n",linestotal,filename );
    lineserrors++;
    continue;
    }  // if( !FileExists(filename) )
  else
    {
    // remember to open in BINARY mode!
    fdata=fopen(filename,"rb");
    if( fdata==NULL )
      {
      printf( "Line %ld: Can't read \"%s\".\n",linestotal,filename );
      lineserrors++;
      continue;
      }  // if( fdata==NULL )

    // okay, get the size!
    fseek(fdata,0,SEEK_END);
    fsize=(long unsigned)ftell(fdata);
    fclose(fdata);

    // cool.  now print out the entries to the two files!
    fprintf(fauto,PREFIX_DLOAD "%s\t\t%lx\n",filename,base);
    fprintf(fheader,PREFIX_HEADER "\t%s\t\t(0x%lx)\t/* %lu */ \n",
	definename,base,fsize);

    // advance to the next base
    bytestotal+=fsize;
    base+=fsize-(fsize%ALIGNMENT)+ALIGNMENT;
    }  // else if( !FileExists(filename) )
  }  // while( !feof(flist) )

// check bounds again
if( base>BOUND_MAX )
  {
  printf( "\n\t*** WARNING ***\n"
	"\tLast entry exceeds %lx!\n\n",BOUND_MAX );
  }  // if( base>BOUND_MAX )

// add some closing stuff
fprintf(fheader,"\n/* Total Size: %ld bytes */\n" \
	"/* Next free memory space at 0x%lx */\n\n",
	bytestotal,base);

// close our handles!
fcloseall();

printf( "\n" \
	"Lines in list file:  %ld\n" \
	"Lines processed:     %ld\n" \
	"Lines w/ errors:     %ld\n\n" \
	"Size to download:    %ld bytes\n" \
	"Memory consumed:     0x%lx-0x%lx (%lu bytes)\n" \
	"Next free mem space: 0x%lx\n\n",
	linestotal,linesdata,lineserrors,bytestotal,
	start,base-1,base-start,base );

if( lineserrors )
  printf( "\t*** WARNING ***\n\tSome files had errors.\n" );

return(1);
}

// ......................................................................
// in     .
// out    .
// desc   prints help
// notes  .
// ......................................................................
void ShowHelp( void )
{
printf( "YOC v%s Quick Help:\n" \
	"====================\n" \
	"\n" \
	"  YOC (Yaroze Offset Calculator) is used to calculate the addresses for the\n" \
	"files you have.\n" \
	"  The <hex start addr> is the base address to be used for offset\n" \
	"calculations.  Only supply the number (no leading\"0x\" or trailing \"h\".\n" \
	"  The <list file> is a text file where each non-blank line is in the\n" \
	"format: <constant name> <file name>.  The constant is the identifier name\n" \
	"to be put in the header file, the file is the path to the file to include.\n" \
	"Use whitespace between them.  You can make a line a comment by having the first\n" \
	"character be a \"#\".\n" \
	"  The <auto file> is one output file which you can cut/paste into your Yaroze\n" \
	"batch loading file.  Each line will retain the path you used for the <file\n" \
	"name>.  The <header file> is a C header file with #defines to the <file name>.\n" \
	"\n" \
	"(c) 3/1998 Elliott Lee (tenchi@netmagic.net).\n" \
	"Based off of a program by Don Yang.\n",
	PROG_VER );
}

// ......................................................................
// in     .
// out    .
// desc   main
// notes  .
// ......................................................................
void main( int argc, char *argv[] )
{
char		*filelist,*fileauto,*fileh;
unsigned long	Base;

// map the arguments to meaningful names
filelist=argv[2];
fileauto=argv[3];
fileh=argv[4];

// check our parameters
CheckArgs( argc,argv,&Base,filelist,fileauto,fileh );

// alloc mem
WorkingBuf = malloc(WBUF*sizeof(char));
if( WorkingBuf==0 )
  ExitError( "Can't allocate working buffer memory!" );

// away we go!
ProcessList( Base,filelist,fileauto,fileh,WorkingBuf,WBUF);

// release memory
free( WorkingBuf );
}

// end
