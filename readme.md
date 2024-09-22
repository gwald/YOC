# YOC
Yaroze Offset Calculator - Calculates the offsets and generates 2 files for you.
(c) 3/1998 Elliott Lee.

Maintained here: [https://github.com/gwald/YOC](https://github.com/gwald/YOC)

----

## Usage (all parameters are mandatory):

    YOC HexStartAddr  MyProject.yoc YOC
    
The **HexStartAddr** is the base address to be used for offset calculations, it can start with 0x or end in h.

The **MyProject.yoc** filename must be be alphanum only. It is a text file where each non-blank line is in the format: 
**C_DEFINE_constant_name** **data_path_and_filename**  

	**C_DEFINE_constant_name** is the identifier name (#define) to be put in the header file. It can only use underscores and alphanumeric characters.
	**data_path_and_filename** is the path and filename to the file to include. It can only use underscore, dot, forward or back slash and alphanumeric characters.
	The optional executable must be on the last line and without a **constant_name** value, ie just the **file_name**.
	Comment out lines with the first character being "#".

The **YOC** is your project name (single word, recommend using YOC) and will be used for generating output files, see below.
You probably won't need all of them, just ingore them.

----

## The output files created are: 

The **YOC.sio** is created for Net Yaroze Siocons batch loading file or used with [yarexe to create a psx.exe](https://github.com/gwald/Yarexe).
Each line will retain the path you used for the ***MyProject.yoc**. 

The **YOC-vscode.sio** is the created like above but for vscode with PSX.DEV, see Template help in [https://github.com/gwald/psyq_to_netyaroze] (https://github.com/gwald/psyq_to_netyaroze)

The **YOC.dat** is all the files loaded into a single binary file.

The **YOC_dat.sio** is created for Net Yaroze Siocons batch loading the single dat file instead of loading all the files individually.

The **YOC_vscode_dat.sio** is the created like above but for vscode, see Template help in [https://github.com/gwald/psyq_to_netyaroze] (https://github.com/gwald/psyq_to_netyaroze)


The **YOC.h** is the created C header file with #defines to the ***MyProject.yoc**.


	** WARNING! THE FILES ABOVE WILL BE AUTOMATICALLY CREATED - AND OVERWRITTING IF ALREADY EXISTING!! **



## Versions:

**V4 rewrite and added PSX.DEV support  **
https://github.com/gwald/psyq_to_netyaroze

**V3 Fixed bugs, added fcaseopen **
Fixed bugs, added fcaseopen.c by OneSadCookie for linux path and filename DOS support

**V2 ported to TCC/gcc  **
Might have to change _fcloseall to fcloseall to compile in your GCC.
Add more infor to header file and the executable line option, still WIP


**V1 Original 1998 release **
Yaroze Offset Calculator By Elliott Lee 15 March 1998
Original DOS code & win32 executable.
Original source code yoc.c by Elliott Lee 15 March 1998
Written for Borland Turbo C++ 3.0 but executable here was compiled with TCC with -m32 flag.