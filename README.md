# YOC
Yaroze Offset Calculator - Calculates the offsets and generates 2 files for you.
(c) 3/1998 Elliott Lee.

Maintained here: https://github.com/gwald/YOC


    

## Usage (all parameters are mandatory):

    YOC    hex_start_addr    list_file    siocons_file    header_file
    
The **hex_start_addr** is the base address to be used for offset calculations, it can start with 0x or end in h.

The **list_file** is a text file where each non-blank line is in the format: 
**constant_name** **file_name**  

**constant_name** is the identifier name (#define) to be put in the header file. It can only use underscores and alphanumeric characters.
**file_name** is the path and filename to the file to include. It can only use underscore, dot, forward or back slash and alphanumeric characters.
The optional executable must be on the last line and without a **constant_name** value, ie just the **file_name**.
You can make a line a comment by having the first character be a "#".

The **siocons_file** is created for Net Yaroze Siocons batch loading file or used with [yarexe to create a psx.exe](https://github.com/gwald/Yarexe).
Each line will retain the path you used for the **file_name**. 

The **header_file** is the created C header file with #defines to the **file_name**.
