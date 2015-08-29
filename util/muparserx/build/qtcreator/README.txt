Debugging a console application will not work out of the box on linux with gnome. If you experience 
problems try applying the following fixes:

####################################################################################################

1.) QTCreator - run app in terminal with debugger on Linux:
http://stackoverflow.com/questions/11343826/qtcreator-run-app-in-terminal-with-debugger-on-linux

Summary:
$ sudo su -
$ echo 0 > /proc/sys/kernel/yama/ptrace_scope

####################################################################################################

2.) Ubuntu Bug #367954: gnome-terminal does not honor x-terminal-emulator -e option:
 - https://bugs.launchpad.net/ubuntu/+source/qtcreator/+bug/566387

Summary:
In the qt creator under tools-options-environment-general, change terminal value from 
"x-terminal-emulator -e" to "/usr/bin/xterm -e"

