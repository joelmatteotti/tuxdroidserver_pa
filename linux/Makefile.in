#
# TuxDroidInterface Linux Makefile
#
# Joel Matteotti <joel_DOT_matteotti_AT_free_DOT_fr>
#
# TODO: Rendre le Makefile un peut plus propre :)
#

EXEC=TuxDroidServer
SRCDIR=../src/
OBJDIR=../obj/
INCDIR=../include/

CC=gcc

ARCH=${shell getconf LONG_BIT}

LIBAUDIO=-lportaudio -lsndfile -lsamplerate

LIBXML=-lxml2
LIBCURL=-lcurl

AUDIOLIB=-DUSE_PORTAUDIO=1

CFLAGS=-I${INCDIR} -I/usr/include/libxml2 -ansi -pedantic --std=c99 -Wall -pipe -O2 -fno-stack-protector -pthread -D_REENTRANT -D_FORTIFY_SOURCE=0 ${AUDIOLIB}
LDFLAGS=${LIBAUDIO} ${LIBXML} ${LIBCURL}

.PHONY: clean mrproper


all: ${EXEC}


${EXEC}: TuxServer.o TuxDownloader.c TuxUtils.o TuxLogger.o TuxAttitune.o TuxStrings.o TuxDriver.o TuxAudio.o TuxTime.o
	${CC} ${CFLAGS} -o ${EXEC} ${OBJDIR}*.o ${LDFLAGS} -lm -ldl -lpthread
	



TuxServer.o: ${SRCDIR}TuxServer.c
	mkdir -p $(OBJDIR)	
	${CC} ${CFLAGS} -o ${OBJDIR}TuxServer.o -c ${SRCDIR}TuxServer.c


TuxDownloader.c: ${SRCDIR}TuxDownloader.c
	mkdir -p ${OBJDIR}
	${CC} ${CFLAGS} -o ${OBJDIR}TuxDownloader.o -c ${SRCDIR}TuxDownloader.c

TuxUtils.o: ${SRCDIR}TuxUtils.c
	mkdir -p ${OBJDIR}
	${CC} ${CFLAGS} -o ${OBJDIR}TuxUtils.o -c ${SRCDIR}TuxUtils.c

TuxAttitune.o: ${SRCDIR}TuxAttitune.c
	mkdir -p ${OBJDIR}
	${CC} ${CFLAGS} -o ${OBJDIR}TuxAttitune.o -c ${SRCDIR}TuxAttitune.c

TuxStrings.o: ${SRCDIR}TuxStrings.c
	mkdir -p ${OBJDIR}
	${CC} ${CFLAGS} -o ${OBJDIR}TuxStrings.o -c ${SRCDIR}TuxStrings.c

TuxAudio.o: ${SRCDIR}TuxAudio.c
	mkdir -p ${OBJDIR}
	${CC} ${CFLAGS} -o ${OBJDIR}TuxAudio.o -c ${SRCDIR}TuxAudio.c

TuxLogger.o: ${SRCDIR}TuxLogger.c
	mkdir -p ${OBJDIR}
	${CC} ${CFLAGS} -o ${OBJDIR}TuxLogger.o -c ${SRCDIR}TuxLogger.c

TuxDriver.o: ${SRCDIR}TuxDriver.c
	mkdir -p ${OBJDIR}
	${CC} ${CFLAGS} -o ${OBJDIR}TuxDriver.o -c ${SRCDIR}TuxDriver.c

TuxTime.o: ${SRCDIR}TuxTime.c
	mkdir -p ${OBJDIR}
	${CC} ${CFLAGS} -o ${OBJDIR}TuxTime.o -c ${SRCDIR}TuxTime.c


install:
	cp udev/rules.d/45-tuxdroid.rules /etc/udev/rules.d/
	service udev restart
	mkdir -p /etc/tuxdroidserver/
	mkdir -p /var/log/tuxdroidserver/

uninstall:
	rm -f /etc/udev/rules.d/45-tuxdroid.rules
	rm -Rf /etc/tuxdroidserver/
	service udev restart

clean:
	rm -f ${OBJDIR}*

mrproper: clean
	rm -Rf ${OBJDIR}	
	rm -f ${EXEC}
	rm -f config.log	
	rm -f config.h
	rm -f Makefile
		

	

