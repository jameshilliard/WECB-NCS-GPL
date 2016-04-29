include $(CTL_DIR)/global.def
include Auto_Build_Files
include Manual_Build_Files
uuidlibpath = ../../../../../../libuuid
tr69cpath = ../../../../apps/private/tr69_1.0/

CFLAGS += -I../include -I../../../../../../boa/apmib/ -I../../../../../../../linux-2.6.30/drivers/net/wireless/rtl8192cd/
CFLAGS += -I$(uuidlibpath)
CFLAGS += -I$(tr69cpath)
CFLAGS += -Werror

TARGET = $(INSTALL)/lib/libtr69_func.a

all: $(OBJS) apmib
	$(AR) -cruv $(TARGET) $(OBJS) ../../../../../../boa/apmib/apmib.o ../../../../../../boa/apmib/mibtbl.o
clean:
	rm -f *.o
	rm -f $(TARGET)

#.c.o:../../../../include/ctl.h
%.o:%.c ../include/OID.h ../include/tr69_func.h ../include/tr69_inst.h ../include/tr69_st.h
	$(CC) -c -o $@ $(CFLAGS) $(IFLAGS) $<
apmib: tr69c
	cd ../../../../../../boa/ && $(MAKE)
tr69c:
	cd ../tr69_lib/ && $(MAKE)
