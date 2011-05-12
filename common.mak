WARNINGS		= -Wall -Wextra -Wshadow -Wundef -Wformat=2 -Winit-self -Wunused -Werror -Wpointer-arith -Wcast-qual -Wmultichar

ifneq ($(DEBUG), on)
CPPFLAGS		+= -O2 $(WARNINGS)
LDFLAGS			+= -s
else
CPPFLAGS		+= -O0 -g $(WARNINGS)
LDFLAGS			+= -g
endif

ifeq ($(TARGET), x86_64)
	CPPFLAGS	+= -DTARGET=x86_64
endif

ifeq ($(TARGET), i386)
	CPPFLAGS	+= -m32 -DTARGET=i386
	LDFLAGS		+= -m32
endif

ifeq ($(TARGET), mipsel)
	EXECPREFIX	= /home/erik/src/openpli/build-dm8000/tmp/cross/mipsel-linux/bin/
	CPPFLAGS	+= -I/home/erik/src/openpli/build-dm8000/tmp/cross/mipsel-linux/include
	CPPFLAGS	+= -I/home/erik/src/libmicrohttpd/mipsel/usr/include
	CPPFLAGS	+= -DTARGET=mipsel
	LDFLAGS		+= -L/home/erik/src/libmicrohttpd/mipsel/usr/lib
endif

ifeq ($(TARGET), ppc)
	EXECPREFIX	= /home/erik/src/tuxbox/root/cdk/bin/powerpc-tuxbox-linux-gnu-
	CPPFLAGS	+= -I/home/erik/src/tuxbox/root/cdkroot/include
	CPPFLAGS	+= -DTARGET=ppc
endif

CC	= $(EXECPREFIX)gcc
CPP	= $(EXECPREFIX)g++

.PHONY:		all depend clean install

all:		depend $(AOUT)

-include .deps

depend:		.deps

.deps:		$(DEPS)
			@cat $^ > $@

.%.d:		%.cpp
			@$(CPP) $(CPPFLAGS) -M $^ -o $@

.%.d:		%.c
			@$(CPP) $(CPPFLAGS) -M $^ -o $@

%.o:		%.cpp
			@echo "CPP $< -> $@"
			@$(CPP) $(CPPFLAGS) -c $< -o $@

$(AOUT):	$(OBJS)
			$(CPP) $(LDFLAGS) $^ $(LDLIBS) -o $@ 

install:	$(AOUT)
			@echo "INSTALL $(AOUT)"
			sudo install -o root -g root -m 755 -s $(AOUT) /usr/local/bin

clean:
			@echo "CLEAN $(OBJS) $(DEPS) $(AOUT)"
			@rm $(OBJS) $(DEPS) $(AOUT) .deps 2> /dev/null || true

