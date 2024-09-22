// program:	Yaroze Offset Calculator
// mainfile:	yoc.c
//
// file:	yoc.c
// created:	15 March 1998
// author:	Elliott Lee
// company:	Protocol Software
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


#define PROG_VER	"v4.0 - More YOC outputs - Sept-2024"


// sometimes the _ is required!!!
#ifdef __linux__
#define  FCLOSEALL_FUNCTION fcloseall();
#else
#define  FCLOSEALL_FUNCTION _fcloseall();
#endif


/*   NOTES:
Maintained here: https://github.com/gwald/YOC

Build:
gcc -static-libgcc -static-libstdc++ -static -O3  YOC.c -o YOC
tcc -m32 YOC.c -oYOC.exe

History:

v3: fixed bugs, added fcaseopen.c by OneSadCookie

Version 2:
Replaced old DOS stuff
leading "0x" or trailing "h" support on hex init RAM address.
Last line can be an executable if it has no #define value, ie only the filename is defined.
Added Exec load address to #define file.
 */



// ......................................................................
// GENERAL INCLUDES
// ......................................................................
//#include <conio.h>
//#include <dos.h>
//#include <io.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>



// ......................................................................
// GLOBAL VARS / DEFINES
// ......................................................................
#define	ALIGNMENT	(0x10)
#define BOUND_MAX	(0x801efffflu)
#define BOUND_MIN	(0x80090000lu)
#define	NUM_ARGS	4
#define PREFIX_DLOAD	"local dload"
#define PREFIX_LOAD		"local load"
#define GO_LOAD          "GO"



#define NEXT_SPACE_HEADER "FREE_SPACE"
#define EXE_NAME_HEADER "EXE"
#define PREFIX_HEADER	"#define"
#define WBUF		4096


/**** YAR EXE ****/

// ecoff a.out format from siocon src
typedef struct filehdr {
	unsigned short	f_magic;	//00 -02 /* magic number */
	unsigned short	f_nscns;	//02 -04 /* number of sections */
	unsigned int		f_timdat;	// 04 -08 /* time & date stamp */
	unsigned int		f_symptr;	// 08 -12 /* file pointer to symbolic header */
	unsigned int		f_nsyms;	// 12 -16 /* sizeof(symbolic hdr) */
	unsigned short	f_opthdr; // 16 -18		/* sizeof(optional hdr) */
	unsigned short	f_flags; // 18 -20		/* flags */
}FILEHDR;

typedef	struct aouthdr {
	unsigned short	magic;   // 20 -22		 00 -02 0x62 and 0x01
	unsigned short	vstamp;  // 22 -24
	unsigned int	tsize;  // 24 -28
	unsigned int	dsize;  //28 -32
	unsigned int	bsize;  //32 -36
	unsigned int	entry; //36 -40
	unsigned int	text_start;  //40 -44
	unsigned int	data_start;	//44 -48
	unsigned int	bss_start; //48 -52
	unsigned int	gprmask; //52 -56
	unsigned int	cprmask[4]; //56 -72
	unsigned int	gp_value; // 72 -76
} AOUTHDR;


typedef struct scnhdr { // 40bytes
	char		s_name[8];		/* 76  - 84 section name */
	unsigned int		s_paddr;		/* 84  - 88 physical address, aliased s_nlib */
	unsigned int		s_vaddr;		/* 88  - 92 virtual address */
	unsigned int		s_size;			/* 92  - 96 section size */
	unsigned int		s_scnptr;		/* 96  - 100 file ptr to raw data for section */
	unsigned int		s_relptr;		/* 100 - 104 file ptr to relocation */
	unsigned int		s_lnnoptr;		/* 104 - 108 file ptr to gp histogram */
	unsigned short	s_nreloc;	/* 108 - 110 number of relocation entries */
	unsigned short	s_nlnno;	/* 110 - 112 number of gp histogram entries */
	unsigned int		s_flags;		/* 112 - 116 flags */
}SCNHDR;



typedef	struct allEcoffHdr{
	FILEHDR fileHDR;
	AOUTHDR ecoHDR;
	SCNHDR  sections[10];
} ECOHDRS;
/**** YAR EXE ****/



//https://github.com/OneSadCookie/fcaseopen/blob/master/fcaseopen.c
FILE *fcaseopen(char const *path, char const *mode);


// ......................................................................
// PROTOTYPES
// ......................................................................
void		CheckArgs( int argc, char *argv[], unsigned long *base,
		char *flist );
char 		*EliminateSpaces( char *s );
void		ExitError( char *msg );
int		FileExists( char *f );
unsigned long	HexStr2ULong( char *str );
int 		IsBlank( char *s );
int 		IsComment( char *s );
int		IsHex( char *num );
void		OverwritePrompt( char *f );
int 		ProcessList( long unsigned base, char *filelist, char *project);
void 		ShowHelp( void );




// https://stackoverflow.com/questions/656542/trim-a-string-in-c
char *ltrim(char *s)
{
	while(isspace(*s)) s++;
	return s;
}

char *rtrim(char *s)
{
	char* back = s + strlen(s);
	while(isspace(*--back));
	*(back+1) = '\0';
	return s;
}

char *trim(char *s)
{
	return rtrim(ltrim(s));
}




// ......................................................................
// in     number of command line args, command line argument pointer,
//	  base, list file name, auto file name, C header file name
// out    .
// desc   checks to make sure this is properly formatted!
// notes  .
// ......................................................................
void CheckArgs( int argc, char *argv[], unsigned long *base,
		char *flist )
{


	// seeking help?
	if( strcmp(argv[1],"-h")==0 ||  strcmp(argv[1],"-H")==0 )
	{
		ShowHelp();
		exit(0);
	}  // if( strcmpi(argv[1],"-h") )



	// map the arguments to meaningful names



	// right number of args?
	if( argc!=(NUM_ARGS) )
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

	/*
	// duplicate names?
	if( strcmp(fauto,flist) == 0)
		ExitError( "The <auto file> cannot be the same as the list file!" );
	if( strcmp(fh,flist) == 0 )
		ExitError( "The <header file> cannot be the same as the list file!" );
	if( strcmp(fh,fauto) == 0 )
		ExitError( "The <auto file> and <header file> cannot be the same!" );
	 */


#if 0 //removed prompts
	// overwrite?
	OverwritePrompt(fauto);
	OverwritePrompt(fh);
#endif
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

	return s;
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
	printf( "Usage: yoc <hex start addr> <project.yoc> YOC\n" );
	ShowHelp();
	FCLOSEALL_FUNCTION;

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

	h=fcaseopen(f,"rt");
	if( h==NULL )
	{
		return(0);
	}

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
	s=trim(s);

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

	if(num[0]=='x' || num[0]=='X') //skip x X
		num = num+1;
	else if(num[1]=='x' || num[1]=='X') //skip 0x 0X
		num = num+2;


	while( *num!=0 )
	{
		if(*num=='h' || *num=='H') //skip ending h H
		{
			num++;
			continue;
		}


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

	if(str[0]=='x' || str[0]=='X') //skip x X
		str = str+1;
	else if(str[1]=='x' || str[1]=='X') //skip 0x 0X
		str = str+2;

	while( *str!=0 )
	{

		if(*str =='h' || *str=='H') //skip ending h H
		{
			str++;
			continue;
		}

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
	c=getchar();
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
int ProcessList( long unsigned base, char *filelist, char *projectname )
{
	FILE	*flist,*fauto,*fautodata,*fauto_vscode,*fheader,*fdata, *fbindat, *fauto_vscodedat;
	long	linestotal=0,
			linesdata=0,
			lineserrors=0,
			is_exec;
	char	*definename, *filename;
	char fileheader[256]={0},fileauto[256]={0}, filevscode[256]={0}, filevscodedat[256]={0}, fileautodat[256]={0}, filebindat[256]={0};
	long unsigned start=base,fsize=0,bytestotal=0;

	char workbuffer[WBUF], *line_p;

	sprintf( fileheader, "%s.h", projectname);
	sprintf( fileauto, "%s.sio", projectname);
	sprintf( fileautodat, "%s_dat.sio", projectname);
	sprintf( filevscode, "%s_vscode.sio", projectname);
	sprintf( filebindat, "%s.dat", projectname);
	sprintf( filevscodedat, "%s_vscode_dat.sio", projectname);

	// printf(" filelist: %s\n", filelist);
	// printf(" fileauto: %s\n", fileauto);
	// printf(" fileheader: %s\n", fileheader);

	// open our handles!
	flist=fcaseopen(filelist,"rt");
	if( flist==NULL )
	{
		ExitError( "Can't open YOC list file." );
	}

	fauto=fcaseopen(fileauto,"wt+");
	if( fauto==NULL )
	{
		ExitError( "Can't write/create project.sio file." );
	}


	fprintf( fauto,
			"# These constants automatically calculated with YOC %s  " \
			"\n# (c) 3/1998 Elliott Lee.                               " \
			"\n# Maintained here: https://github.com/gwald/YOC         " \
			"\n#                                                       " \
			"\n#                                                    " \
			"\n# Base begins at 0x%8x                             " \
			"\n#                                                    "
			"\n#                                                    "
			"\n",
			PROG_VER,(unsigned int)start  );


	fautodata=fcaseopen(fileautodat,"wt+");
	if( fautodata==NULL )
	{
		ExitError( "Can't write/create project-dat.sio file." );
	}


	fprintf( fautodata,
			"# These constants automatically calculated with YOC %s  " \
			"\n# (c) 3/1998 Elliott Lee.                               " \
			"\n# Maintained here: https://github.com/gwald/YOC         " \
			"\n#                                                       " \
			"\n#                                                    " \
			"\n# Base begins at 0x%8x                             " \
			"\n#                                                    "
			"\n#                                                    "
			"\n%s \t%s\t\t0x%lx \n",
			PROG_VER,(unsigned int)start
			,PREFIX_DLOAD,filebindat ,(unsigned int)start  );


	fauto_vscode=fcaseopen(filevscode,"wt+");
	if( fauto==NULL )
	{
		ExitError( "Can't write/create project_vscode.sio file." );
	}



	fprintf( fauto_vscode,
			"# These constants automatically calculated with YOC %s  " \
			"\n# (c) 3/1998 Elliott Lee.                               " \
			"\n# Maintained here: https://github.com/gwald/YOC         " \
			"\n#                                                       " \
			"\n# Include this file in your main code source.  e.g.     " \
			"\n#                                                       " \
			"\n#      #include \"%s\"        " \
			"\n#                                                    " \
			"\n# Base begins at 0x%8x                             ",
			PROG_VER,fileheader,(unsigned int)start  );


	//extra info
	fprintf(fauto_vscode,
			"\n#\n# VSCode (https://www.psx.dev/getting-started) this siocons file is loaded like this in your .vscode/launch.json: \n#\n#\t\t \"autorun\": [\n#   \"monitor reset shellhalt\",\n#\t\t   \"source %s\",\n#\t\t   \"load ${workspaceRootFolderName}.elf\",\n#\t\t   \"tbreak main\",\n#\t\t   \"continue\"\n#\t\t]\n#\n# See https://github.com/gwald/psyq_to_netyaroze for more info.\n\n"
			,filevscode);







	fauto_vscodedat=fcaseopen(filevscodedat,"wt+");
	if( fauto_vscodedat==NULL )
	{
		ExitError( "Can't write/create project_vscode_dat.sio file." );
	}


	fprintf( fauto_vscodedat,
			"# These constants automatically calculated with YOC %s  " \
			"\n# (c) 3/1998 Elliott Lee.                               " \
			"\n# Maintained here: https://github.com/gwald/YOC         " \
			"\n#                                                       " \
			"\n#                                                    " \
			"\n# Base begins at 0x%8x                             ",
			PROG_VER,(unsigned int)start,filebindat ,(unsigned int)start  );



	//extra info
	fprintf(fauto_vscodedat,
			"\n# VSCode (https://www.psx.dev/getting-started) this siocons file is loaded like this in your .vscode/launch.json: \n#\n#\t\t \"autorun\": [\n#\t\t   \"monitor reset shellhalt\",\n#\t\t   \"source %s\",\n#\t\t   \"load ${workspaceRootFolderName}.elf\",\n#\t\t   \"tbreak main\",\n#\t\t   \"continue\"\n#\t\t]\n#\n# See https://github.com/gwald/psyq_to_netyaroze for more info.\n"
			,filevscodedat);

	fprintf(fauto_vscodedat,"\nrestore  %s  binary  0x%lx\n",filebindat,base);


	fheader=fcaseopen(fileheader,"wt+");
	if( fheader==NULL )
	{
		ExitError( "Can't write/create header file." );

	}
	// add some heading info
	fprintf( fheader,
			"/* These constants automatically calculated with YOC %s  \n" \
			" (c) 3/1998 Elliott Lee.                               \n" \
			"Maintained here: https://github.com/gwald/YOC         \n" \
			"                                                      \n" \
			" Include this file in your main code source.  e.g.     \n" \
			"                                                       \n" \
			"      #include \"%s\"        \n" \
			"                                                       \n" \
			" Base begins at 0x%8x                             \n*/\n\n",
			PROG_VER,fileheader,(unsigned int)start  );


	fbindat=fcaseopen(filebindat,"wb+");
	if( fauto==NULL )
	{
		ExitError( "Can't write/create project.dat file." );
	}



	// check bounds
	if( base<BOUND_MIN )
	{
		printf( "\n\t*** WARNING ***\n"
				"\tBase address is lower than %lx!\n\n",BOUND_MIN );
	}  // if( base<BOUND_MIN )

	while( !feof(flist) )
	{
		is_exec=0; // default is data load

		// get a line!
		fgets(workbuffer,WBUF,flist);
		linestotal++;
		printf(workbuffer,"\n");

		// make sure we're dealing with a command here...
		line_p=trim(workbuffer);
		if( line_p[0]==0 || IsBlank(line_p) || IsComment(line_p) )
			continue;

		linesdata++;

		// okey, now separate this into two parameters!

		// define name
		definename=line_p;

		// seek to end of this word
		while( *line_p!=0 && *line_p>=33 )
			line_p++;

		// did we run out of chars? next char's should be filename, if not it's a single command like NY exe
		if( *line_p==0 )
		{
#if 0
			if(linestotal != linesdata)
			{
				printf( "Line %ld: Missing file name (2nd parameter).\n",linestotal );
				//lineserrors++;
				continue;
			}
			else // last line is exe
#endif
			{
				is_exec = 1;
				filename = definename;
			}

		}  // if( *line_p==0 )

		if(!is_exec) //process the rest of the line
		{
			// we have landed on the end of this word.  change this character to
			// a null, advance to the next character, and then seek to the next
			// word/end-of-string
			*line_p=0;
			line_p=trim(line_p+1);

			// are we at the end?
			if( *line_p==0 )
			{
				printf( "Line %ld: Missing file name (2nd parameter).\n",linestotal );
				printf(workbuffer,"\n");
				lineserrors++;
				continue;
			}  // if( *line_p==0 )

			// okay, set the file name
			filename=line_p;

			// seek to end of word
			while( *line_p!=0 && *line_p>=33 )
				line_p++;

			// are we at the end of the string?
			if( *line_p!=0 )
			{
				// hm... no, we're not.  looks like some straggler characters.
				// let's drop the NULL here to terminate the file name, but keep
				// checking for superfluous characters.
				*line_p=0;
				line_p=trim(line_p+1);

				// is the current character the end of string?  if not, then print
				// a friendly warning message.
				if( *line_p>0 )
					printf( "Line %ld: Extra characters on line; ignored.\n",linestotal );
			}  // if( *line_p!=0 )
		}// if(!is_exec)


		// okay, now let's get the size of the file!
		if( !FileExists(filename) )
		{
			char buff[128]={0};

			sprintf(buff, "Line %ld: File not found: \"%s\".\n",linestotal,filename );

			if(!is_exec) // data needs to exist!
				ExitError(buff );
			else
			{
				// missing executable is a warning only
				printf(buff);
				lineserrors++;
			}
		}  // if( !FileExists(filename) )


		{
			// remember to open in BINARY mode!
			fdata=fcaseopen(filename,"rb");
			if( fdata!=NULL )
			{
				// okay, get the size!
				fseek(fdata,0,SEEK_END);
				fsize=(long unsigned)ftell(fdata);
			}
			else
				fsize = 0;

			if(is_exec)
			{
				ECOHDRS exe;

				if( fdata!=NULL )
				{
					// okay, get the size!

					rewind(fdata);
					fread(&exe, 1, sizeof(exe), fdata);
					fclose(fdata);
				}
				else
					fsize = 0;

				// End space
				fprintf(fheader,PREFIX_HEADER "\t%s\t\t(0x%lx) /* Address of last asset + size */ \n",
						NEXT_SPACE_HEADER,(long unsigned int)base );

				// add some closing stuff
				fprintf(fheader,"\n/* Total Asset size: 0x%lx (%ld) bytes */\n\n",
						bytestotal,(long unsigned int) bytestotal);


				// cool.  now print out the entries to the two files!
				fprintf(fauto,PREFIX_LOAD "  \t%s\n",filename);
				fprintf(fauto,GO_LOAD " \n");

				fprintf(fautodata,PREFIX_LOAD "  \t%s\n",filename);
				fprintf(fautodata,GO_LOAD " \n");





				fprintf(fheader,PREFIX_HEADER "\t%s_LOAD_ADDR\t\t(0x%lx)\n",
						(EXE_NAME_HEADER), (long unsigned int)exe.sections[0].s_paddr);

				fprintf(fheader,PREFIX_HEADER "\t%s_SIZE\t\t (%lu)\t/* %lu bytes */\n\n",
						(EXE_NAME_HEADER),fsize,fsize);

				fsize = 0; //exe size doesn't count
			}
			else
			{
				char *filebindata_buff_p;
				int fixed_size;




				fprintf(fauto,PREFIX_DLOAD "\t%s\t\t0x%lx\n",filename,base);
				fprintf(fauto_vscode,"restore  %s  binary  0x%lx\n",filename,base);



				fprintf(fheader,PREFIX_HEADER "\t%s\t\t(0x%lx)\n",
						definename,(long unsigned int)base);
				fprintf(fheader,PREFIX_HEADER "\t%s_SIZE\t\t(0x%lx)\t/* %lu bytes */\n\n",
						definename,(long unsigned int)fsize,fsize);




				// okay, get the size of this file!
				fseek(fdata,0,SEEK_END);
				fsize=(long unsigned)ftell(fdata);
				fseek(fdata,0,SEEK_SET);

				// align with the base and fsize
				if(ALIGNMENT-(base+fsize)%ALIGNMENT != ALIGNMENT)
					fixed_size =fsize + (ALIGNMENT-(base+fsize)%ALIGNMENT);
				else
					fixed_size =fsize;


				filebindata_buff_p = calloc(1, fixed_size );

				fread(filebindata_buff_p,  sizeof(char), fsize,fdata );
				fwrite(filebindata_buff_p,  sizeof(char),fixed_size, fbindat ); //write size with the same alignment

				free(filebindata_buff_p);

				fclose(fdata);

			}

			//base+=fsize-(fsize%ALIGNMENT)+ALIGNMENT; // this adds an extra 16bytes

			// printf(" base:%d + fsize:%d  = %x (base+fsize) ALIGNMENT:%d \n",base, fsize, base+ fsize, ALIGNMENT-(base+fsize)%ALIGNMENT);
			// fflush(stdout);


			if(ALIGNMENT-(base+fsize)%ALIGNMENT != ALIGNMENT)
				fsize =fsize + (ALIGNMENT-(base+fsize)%ALIGNMENT);


			base+=fsize;

			// advance to the next base
			bytestotal+=fsize;

			fclose(fdata);

			if(is_exec)
				break;


		}  // else if( !FileExists(filename) )

		//		if( *line_p==0 )
		//			break;
	}  // while( !feof(flist) )




	// check bounds again
	if( base>BOUND_MAX )
	{
		printf( "\n\t*** WARNING ***\n"
				"\tLast entry exceeds %lx!\n\n",BOUND_MAX );

		fprintf(fheader, "\n\t/*** WARNING ***\n"
				"\tLast entry exceeds %lx!\n\n"
				"\n\t*** WARNING ***/\n",BOUND_MAX
		);


	}  // if( base>BOUND_MAX )









	// close our handles!
	FCLOSEALL_FUNCTION;

	printf( "\n" \
			"Lines in list file:  %ld\n" \
			"Lines processed:     %ld\n" \
			"Lines w/ warnings:   %ld\n\n" \
			"Size to download:    %ld bytes\n" \
			"Memory consumed:     0x%lx-0x%lx (%lu bytes)\n" \
			"Next free mem space: 0x%lx\n\n",
			linestotal,linesdata,lineserrors,bytestotal,
			start,base-1,base-start,base );

	if( lineserrors )
	{
		printf( "\t*** WARNING ***\n\tSome files had warnings.\n" );
		return(1);
	}

	return(0);
}

// ......................................................................
// in     .
// out    .
// desc   prints help
// notes  .
// ......................................................................
void ShowHelp( void )
{
	printf( "YOC %s \n" \
			"====================\n" \
			"\n" \
			"  YOC (Yaroze Offset Calculator) is used to calculate the addresses for the\n" \
			"files you have.\n\n" \
			" YOC HexStartAddr  MyProject.yoc YOC \n" \
			"  The <HexStartAddr> is the base address to be used for offset\n" \
			"calculations.\n" \
			"  The <MyProject.yoc> is a text file where each non-blank line is in the\n" \
			"format: <constant name> <file name>.  The constant is the identifier name\n" \
			"to be put in the header file, the file is the path to the file to include.\n" \
			"Use whitespace between them.  You can make a line a comment by having the first\n" \
			"character be a \"#\".\n" \
			"The optional executable must be on the last line and without a constant_name value\n" \
			"  The <YOC> is the name for all output files, you will get:\n"\
			"The main output files are \n" \
			" 1) The <YOC.sio> is one output file which you can cut/paste into your Yaroze\n" \
			"batch loading file.  Each line will retain the path you used for the <file\n" \
			"name>.  \n"
			"2) The <YOC_vscode.sio> like the a siocons file but for vscode, see readme.md.\n" \

			"3) The <YOC.h> is a C header file with #defines to the <file name>.\n" \
			"\n\n" \
			"(c) 3/1998 Elliott Lee.\nMaintained here: https://github.com/gwald/YOC.\n  " \
			"Based off of a program by Don Yang.\n",
			PROG_VER );
}

// ......................................................................
// in     .
// out    .
// desc   main
// notes  .
// ......................................................................
int main( int argc, char *argv[] )
{
	char		*filelist,*fileauto,*fileh;
	unsigned long	Base;
	int	ret;


	if( argc ==1 || argc != NUM_ARGS )
	{
		ShowHelp();
		exit(0);
	}  // if( strcmpi(argv[1],"-h") )



	filelist=trim(argv[2]);

	// check our parameters
	CheckArgs( argc,argv,&Base,filelist );


	// away we go!
	ret = ProcessList( Base,filelist,argv[3]);


	return ret;
}

// end




//https://github.com/OneSadCookie/fcaseopen/blob/master/fcaseopen.c

#include <unistd.h>
void casechdir(char const *path)
{
#if !defined(_WIN32)
	char *r = alloca(strlen(path) + 2);
	if (casepath(path, r))
	{
		chdir(r);
	}
	else
	{
		errno = ENOENT;
	}
#else
	chdir(path);
#endif
}


#if !defined(_WIN32)
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <errno.h>
#include <unistd.h>

// r must have strlen(path) + 2 bytes
static int casepath(char const *path, char *r)
{
	size_t l = strlen(path);
	char *p = alloca(l + 1);
	strcpy(p, path);
	size_t rl = 0;

	DIR *d;
	if (p[0] == '/')
	{
		d = opendir("/");
		p = p + 1;
	}
	else
	{
		d = opendir(".");
		r[0] = '.';
		r[1] = 0;
		rl = 1;
	}

	int last = 0;
	char *c = strsep(&p, "/");
	while (c)
	{
		if (!d)
		{
			return 0;
		}

		if (last)
		{
			closedir(d);
			return 0;
		}

		r[rl] = '/';
		rl += 1;
		r[rl] = 0;

		struct dirent *e = readdir(d);
		while (e)
		{
			if (strcasecmp(c, e->d_name) == 0)
			{
				strcpy(r + rl, e->d_name);
				rl += strlen(e->d_name);

				closedir(d);
				d = opendir(r);

				break;
			}

			e = readdir(d);
		}

		if (!e)
		{
			strcpy(r + rl, c);
			rl += strlen(c);
			last = 1;
		}

		c = strsep(&p, "/");
	}

	if (d) closedir(d);
	return 1;
}
#endif

FILE *fcaseopen(char const *path, char const *mode)
{
	FILE *f = fopen(path, mode);
#if !defined(_WIN32)
	if (!f)
	{
		char *r = alloca(strlen(path) + 2);
		if (casepath(path, r))
		{
			f = fopen(r, mode);
		}
	}
#endif
	return f;
}
