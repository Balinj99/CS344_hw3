To compile this program simply type make to use the Makefile I wrote.

In case that doesn't work, compile with gcc --std=gnu99 -o smallsh smallsh.c

The resources I used for this assignment are the code examples provided in modules 4 and 5.

I also found a way to use scanf to get an input with spaces by replacing "%s" with "%[^\n]%*c".
I found this method at https://www.geeksforgeeks.org/taking-string-input-space-c-3-different-methods/

Thank you.
