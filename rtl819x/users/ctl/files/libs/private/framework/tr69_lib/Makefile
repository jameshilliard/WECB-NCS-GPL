include $(CTL_DIR)/global.def

CFLAGS += -I../include -I../tr69_core
#CFLAGS += -fno-omit-frame-pointer -fno-optimize-sibling-calls    # for oprofile

OBJS_CLIENT = libtr69_client.o tsl_socket.o
TARGET_CLIENT = $(INSTALL)/lib/libtr69c.a
TARGET_CLIENT_SO = $(INSTALL)/lib/libtr69c_dl.so

OBJS_SERVER = libtr69_server.o tsl_socket.o accesscfg.o
TARGET_SERVER = $(INSTALL)/lib/libtr69s.a

OBJS_MSG = tsl_msg.o tsl_socket.o
TARGET_MSG = $(INSTALL)/lib/libmsg.a

OBJS_PROCESS = tsl_process.o
TARGET_PROCESS = $(INSTALL)/lib/libprocess.a

OBJS_TSLUTIL = tsl_strconv.o
TARGET_TSLUTIL = $(INSTALL)/lib/libtslutil.a
TARGET_TSLUTIL_SO = $(INSTALL)/lib/libtslutil_dl.so

all: $(OBJS_CLIENT) $(OBJS_SERVER) $(OBJS_MSG) $(OBJS_PROCESS) $(OBJS_TSLUTIL)
	$(AR) -cruv $(TARGET_CLIENT) $(OBJS_CLIENT)
	$(AR) -cruv $(TARGET_SERVER) $(OBJS_SERVER)
	$(AR) -cruv $(TARGET_MSG) $(OBJS_MSG)
	$(AR) -cruv $(TARGET_PROCESS) $(OBJS_PROCESS)
	$(AR) -cruv $(TARGET_TSLUTIL) $(OBJS_TSLUTIL)

	$(CC) -shared -o $(TARGET_CLIENT_SO) $(OBJS_CLIENT)
	$(CC) -shared -o $(TARGET_TSLUTIL_SO) $(OBJS_TSLUTIL)

clean:
	rm -f *.o
	rm -f $(TARGET_CLIENT)
	rm -f $(TARGET_CLIENT_SO)
	rm -f $(TARGET_SERVER)
	rm -f $(TARGET_MSG)
	rm -f $(TARGET_PROCESS)	
	rm -f $(TARGET_TSLUTIL)
	rm -f $(TARGET_TSLUTIL_SO)

.c.o:
	$(CC) -c -o $@ $(CFLAGS) $(IFLAGS) -fPIC $<


