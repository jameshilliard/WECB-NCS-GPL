include $(CTL_DIR)/global.def

OBJS = tr69_tree.o
TARGET = $(INSTALL)/lib/libtr69_core.a
CFLAGS += -I../include -I../include/libxml2 -DSUPPORT_PROTYPE0

all: $(OBJS)
	$(AR) -cruv $(TARGET) $(OBJS)
ifneq ($(strip $(RCC)),)
	$(CC) tr69_cfg_main.c tr69_cfg.c $(IFLAGS) $(CFLAGS) -c
else
	$(CC) -o $(INSTALL)/bin/tr69_cfg tr69_cfg_main.c tr69_cfg.c $(IFLAGS) $(CFLAGS) $(LFLAGS) -lxml2 -ltslutil
endif

clean:
	rm -f *.o
	rm -f $(TARGET)
	rm -f $(INSTALL)/bin/tr69_cfg

.c.o:
	$(CC) -c -o $@ $(CFLAGS) $(IFLAGS) $<
