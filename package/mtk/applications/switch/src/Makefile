EXEC = switch

SRC=switch_fun.c switch_753x.c switch_ioctl.c switch_netlink.c
SRC+=switch_fun_an8855.c an8855_sdk/api/src/*.c

CFLAGS+=-I./an8855_sdk/api/inc
CFLAGS+=-DCOMPAT_MODE

all: $(EXEC)

switch: $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC) $(LDLIBS) $(LIBS)

romfs:
	$(ROMFSINST) /bin/switch

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o
