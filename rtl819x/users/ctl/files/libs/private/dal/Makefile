include $(CTL_DIR)/global.def
include $(CTL_DIR)/common.def
#INSTALL=`/bin/pwd`/../tr69fw/target/revj

OBJS += dal_comm.o
OBJS += dal_system_setting.o
OBJS += dal_upgrade.o
OBJS += aei_flashupgrade.o


TARGET = $(INSTALL)/lib/libdal.a
TARGET_SO = $(INSTALL)/lib/libdal.so

IFLAGS += -I$(CTL_DIR)/include/ -I$(CTL_DIR)/libs/private/framework/tr69_func/ -I$(CTL_DIR)/libs/gpl/libxml/libxml2-2.7.7/include/

LD := ld
LDFLAGS := -shared -fpic
DEF_LIBPATH := $(TARGET_DIR)/lib/
DEF_SO_LIB := $(DEF_LIBPATH)/libxml2.so


#TARGET = libdalbridge.a
all: $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET_SO) $(OBJS) $(DEF_SO_LIB)

clean:
	rm -f *.o
	rm -f $(TARGET)
	rm -f $(TARGET_SO)

.c.o:
	$(CC) -c -o $@ $(CFLAGS) $(IFLAGS) ${COMMON_CFLAGS} $<
