==============Freelancer Save Manipulator v0.1==============

This program is currently designed to do one thing - blindly append text to all save files in a specified folder.  In the future this program will likely incorporate all of the features offered by Freelancer Save Converter (written by Accushot) and more.

Currently, one function is supported: Append.  It has the format of Append("<text to append>").  The escape sequence for " is \".  Literal linebreaks are supported in the string.
The script files have a structure similar to that of C/C++/Java programs.  Whitespace is ignored, but linebreaks are not.  Each function call should be superseded by a semicolon (;).  Anything after the semicolon is ignored until the start of the next line.  The file extension of the script files is flm.