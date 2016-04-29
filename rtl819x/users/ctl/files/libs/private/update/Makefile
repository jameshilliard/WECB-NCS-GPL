include $(CTL_DIR)/global.def

OBJS += flash_update.o

TARGET = $(INSTALL)/lib/libupdate.a

all: $(OBJS)
	$(AR) -cruv $(TARGET) $(OBJS)
clean:
	rm -f *.o
	rm -f $(TARGET)

.c.o:
	$(CC) -c -o $@ $(CFLAGS) $(IFLAGS) $<
