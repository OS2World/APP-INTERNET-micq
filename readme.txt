Based on 25. Sept. 98 release of micq

If you upgrade from the old version you should remove the old [.]micqrc
and reenter your UIN.

Additional features: Can play wav files (or execute arbitrary commands for that matter)

If you are updating recreate the micqrc file to support the new commands (its "micqrc" now, fat-compatible). 

Make sure that you have a meaningful HOME environment variable, the data is stored in %HOME/micqrc

If you want to recompile it get the OS/2 port of GNU readline (eg. from 
www.leo.org) and replace input.c (see readline_change.zip inside source.zip))

If you are looking for the source code, its in the source.zip archive :-)

-------------------------------------------------

Detailed Install instructions:
There are actually two possibilites. One way is to add "set home=d:\online\icq" (replace
the path by the path where you want to save your micq-data, this directory must exist) 
to your config.sys (x:\config.sys where x is the boot drive) and then reboot and run it.

Or run the program from a cmd-script that sets the variable (preferred way, doesnt need
reboot). It should look like this: (replace the path in "set home=d:\online\icq" as above).
Note that it also switches to 50 rows ("mode 80,50") instead of the default of 25, which 
is nice. 
====== start of micq.cmd ========
rem @echo off
mode 80,50
set home=d:\online\icq
cls
micq.exe
===== end of file micq.cmd ======


Have Fun!  (Volker = UIN# 15647736 = volker@mail.utexas.edu)

