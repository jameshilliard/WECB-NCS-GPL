include $(CTL_DIR)/global.def
#INSTALL=`/bin/pwd`/../tr69fw/target/revj
OBJS += timer.o tr69_oal_strconv.o tr69_strconv.o tsl_strconv.o tr69_oal_network.o tr69_network.o act_timer.o 
TARGET = $(INSTALL)/lib/libctlutil.a

#TARGET = libctlutil.a
all: $(OBJS)
	$(AR) -cruv $(TARGET) $(OBJS)
clean:
	rm -f *.o
	rm -f $(TARGET)

.c.o:
	$(CC) -c -o $@ $(CFLAGS) $(IFLAGS) $<

