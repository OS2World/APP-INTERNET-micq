# Anyone who knows how to make Makefiles please help out.
# I have no clue but this appears to work fairly well
# be sure to use gmake
CC = gcc
#CFLAGS = -Wall -DUNIX -DOS2 
CFLAGS = -O2 -Wall -DUNIX -DOS2
# Uncomment the below line for humorous messages
#CFLAGS += -DFUNNY_MSGS 
# uncomment the line below for debuging info
#CFLAGS += -g
CLIBS = -lsocket -lbsd -lreadline -ltermcap
#uncomment the line below if your using Solaris
#CLIBS= -lnsl -lsocket
# switch the comments on the lines below if you're using a
# motorola or other bigendian processor
CFLAGS += -DINTEL_END -UMOTOR_END -I/emx/gnureadl -L/emx/gnureadl
#CFLAGS += -UINTEL_END -DMOTOR_END
OBJS =micq.o icq_response.o sendmsg.o util.o ui.o rus_conv.o rl_support.o
HEADERS = datatype.h micq.h rl_support.h ui.h
#
EXE = micq.exe
#

all : $(EXE)

final: all
	emxbind -s $(EXE)
	lxlite /mr2 $(EXE)
	zip -9 micq.zip $(EXE) icq.cmd readme.os2 readme commands.txt icq091.txt todo WISHLIST

re : clean all

$(EXE): ${OBJS} ${HEADERS} 
	$(CC) ${CFLAGS} -o $(EXE) ${OBJS} $(CLIBS) 

${OBJS} : ${HEADERS} makefile

clean :
	rm -f ${OBJS} ${EXE}

