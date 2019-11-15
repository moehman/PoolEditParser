The program executable in this folder has been compiled in Oracle
Linux 8 (x86_64). To run the program, you need to have the Expat
library installed. The easiest way to do this is to use the package
manager:

$ dnf install expat

... or

$ yum install expat

... depending on your system.


To compile the code, you also need the header files which are in a
separate package:

$ dnf install expat-devel
