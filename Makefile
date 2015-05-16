# Copyright Erik Slagter, GPLv2 is applicable

TARGET	= x86_64
DEBUG	= off
PROGRAM	= msgd
OBJS	= msgd.o device.o textentry.o http_server.o http_page.o syslog.o stringstringmap.o
DEPS	= .msgd.d .device.d .textentry.d .http_server.d .http_page.d .syslog.d .stringstringmap.d

ifeq ($(TARGET), x86_64)
	ENABLE_DEVICE_TTY		= 1
	ENABLE_DEVICE_ESP		= 1
	ENABLE_DEVICE_CURSES	= 1
	ENABLE_DEVICE_GTK		= 0
	ENABLE_DEVICE_MATRIX	= 1
	ENABLE_DEVICE_MUIN		= 1
	ENABLE_DEVICE_CF634		= 1
	ENABLE_DEVICE_CF635		= 1
	ENABLE_DEVICE_SURE		= 1
	ENABLE_DEVICE_DM7000	= 0
	CPPFLAGS				+= -DMHD_mode_multithread
	LDLIBS					+= -lmicrohttpd -lpthread
endif

ifeq ($(TARGET), i386)
	ENABLE_DEVICE_TTY		= 1
	ENABLE_DEVICE_ESP		= 1
	ENABLE_DEVICE_CURSES	= 1
	ENABLE_DEVICE_GTK		= 0
	ENABLE_DEVICE_MATRIX	= 1
	ENABLE_DEVICE_MUIN		= 1
	ENABLE_DEVICE_CF634		= 1
	ENABLE_DEVICE_CF635		= 1
	ENABLE_DEVICE_SURE		= 1
	ENABLE_DEVICE_DM7000	= 0
	CPPFLAGS				+= -DMHD_mode_multithread
	LDLIBS					+= -lmicrohttpd -lpthread
endif

ifeq ($(TARGET), mipsel)
	ENABLE_DEVICE_TTY		= 1
	ENABLE_DEVICE_ESP		= 0
	ENABLE_DEVICE_CURSES	= 0
	ENABLE_DEVICE_GTK		= 0
	ENABLE_DEVICE_MATRIX	= 1
	ENABLE_DEVICE_MUIN		= 1
	ENABLE_DEVICE_CF634		= 1
	ENABLE_DEVICE_CF635		= 1
	ENABLE_DEVICE_SURE		= 1
	ENABLE_DEVICE_DM7000	= 0
	CPPFLAGS				+= -DMHD_mode_multithread
	LDLIBS					+= -Wl,-Bstatic -lmicrohttpd -Wl,-Bdynamic -lpthread
endif

ifeq ($(TARGET), ppc)
	ENABLE_DEVICE_TTY		= 1
	ENABLE_DEVICE_ESP		= 0
	ENABLE_DEVICE_CURSES	= 0
	ENABLE_DEVICE_GTK		= 0
	ENABLE_DEVICE_MATRIX	= 1
	ENABLE_DEVICE_MUIN		= 1
	ENABLE_DEVICE_CF634		= 1
	ENABLE_DEVICE_CF635		= 1
	ENABLE_DEVICE_SURE		= 1
	ENABLE_DEVICE_DM7000	= 1
	ifeq ($(ENABLE_DEVICE_DM7000), 1)
		CPPFLAGS			+= -I/home/erik/src/tuxbox/root/cdkroot/include/freetype2
		CPPFLAGS			+= -I/home/erik/src/libmicrohttpd/ppc/usr/include
		LDLIBS				+= -lfreetype
		LDLIBS				+= -L/home/erik/src/libmicrohttpd/ppc/usr/lib
	endif
	CPPFLAGS				+= -DMHD_mode_singlethread
	LDLIBS					+= -Wl,-Bstatic -lmicrohttpd -Wl,-Bdynamic -lpthread
endif

ifeq ($(ENABLE_DEVICE_TTY),1)
	OBJS		+= device_tty.o
	DEPS		+= .device_tty.d
	CPPFLAGS	+= -DDEVICE_TTY
endif

ifeq ($(ENABLE_DEVICE_ESP),1)
	OBJS		+= device_esp.o
	DEPS		+= .device_esp.d
	CPPFLAGS	+= -DDEVICE_ESP
	LDLIBS		+= -lespif
endif

ifeq ($(ENABLE_DEVICE_CURSES),1)
	OBJS		+= device_curses.o
	DEPS		+= .device_curses.d
	CPPFLAGS	+= -DDEVICE_CURSES
	LDLIBS		+= -lncursesw
endif

ifeq ($(ENABLE_DEVICE_GTK),1)
	OBJS		+= device_gtk.o
	DEPS		+= .device_gtk.d
	CPPFLAGS	+= $(shell pkg-config --cflags gtk+-2.0) -DDEVICE_GTK
	LDLIBS		+= $(shell pkg-config --libs gtk+-2.0)
endif

ifeq ($(ENABLE_DEVICE_MATRIX),1)
	OBJS		+= device_matrix.o device_matrix_common.o
	DEPS		+= .device_matrix.d .device_matrix_common.d
	CPPFLAGS	+= -DDEVICE_MATRIX
endif

ifeq ($(ENABLE_DEVICE_MUIN),1)
	OBJS		+= device_muin.o device_matrix_common.o
	DEPS		+= .device_muin.d .device_matrix_common.d
	CPPFLAGS	+= -DDEVICE_MUIN
endif

ifeq ($(ENABLE_DEVICE_CF634),1)
	OBJS		+= device_cf634.o
	DEPS		+= .device_cf634.d
	CPPFLAGS	+= -DDEVICE_CF634
endif

ifeq ($(ENABLE_DEVICE_CF635),1)
	OBJS		+= device_cf635.o
	DEPS		+= .device_cf635.d
	CPPFLAGS	+= -DDEVICE_CF635
endif

ifeq ($(ENABLE_DEVICE_SURE),1)
	OBJS		+= device_sure.o
	DEPS		+= .device_sure.d
	CPPFLAGS	+= -DDEVICE_SURE
endif

ifeq ($(ENABLE_DEVICE_DM7000),1)
	OBJS		+= device_dm7000.o
	DEPS		+= .device_dm7000.d
	CPPFLAGS	+= -DDEVICE_DM7000
endif

include common.mak
