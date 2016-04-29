include $(CTL_DIR)/global.def
COPY = cp
COPY_FLAGS = -a
CFG_CUSTOM_SUFFIX=.custom

all:
	gcc -I../include -I../../../../include -I../include/libxml2 $(ACTION_TEC_DFLAGS) tr69_build.c -o tr69_build -D_LINUX  -L$(CTL_DIR)/target/x86/lib -lxml2 -g -DSUPPORT_PROTYPE0
	gcc -I../include -I../../../../include -I../include/libxml2 $(ACTION_TEC_DFLAGS) tr69_build_whole.c -o tr69_build_whole -D_LINUX  -L$(CTL_DIR)/target/x86/lib -lxml2 -g -DSUPPORT_PROTYPE0 -DGEN_PROTYPE
	$(COPY) $(COPY_FLAGS) auto/tr69_func.h ../include/
	$(COPY) $(COPY_FLAGS) auto/tr69_st.h ../include/
	$(COPY) $(COPY_FLAGS) auto/tr69_inst.h ../include/
	$(COPY) $(COPY_FLAGS) auto/OID.h ../include/
	$(COPY) $(COPY_FLAGS) auto/tr69_func.h $(CTL_DIR)/include/
	$(COPY) $(COPY_FLAGS) auto/tr69_st.h $(CTL_DIR)/include
	$(COPY) $(COPY_FLAGS) auto/tr69_inst.h $(CTL_DIR)/include/
	$(COPY) $(COPY_FLAGS) auto/OID.h $(CTL_DIR)/include/

ifeq (protype.xml$(CFG_CUSTOM_SUFFIX), $(wildcard protype.xml$(CFG_CUSTOM_SUFFIX)))
	$(COPY) $(COPY_FLAGS) protype.xml$(CFG_CUSTOM_SUFFIX) $(INSTALL)/etc/protype.xml
else
	$(COPY) $(COPY_FLAGS) protype.xml $(INSTALL)/etc/protype.xml
endif
ifeq (cfg.xml$(CFG_CUSTOM_SUFFIX), $(wildcard cfg.xml$(CFG_CUSTOM_SUFFIX)))
	$(COPY) $(COPY_FLAGS) cfg.xml$(CFG_CUSTOM_SUFFIX) $(INSTALL)/etc/cfg.xml
else
	$(COPY) $(COPY_FLAGS) cfg.xml $(INSTALL)/etc/cfg.xml
endif

	# Generate instoid.lst based on protype.xml.custom when build image
	./tr69_build -I

	$(COPY) $(COPY_FLAGS) instoid.lst $(INSTALL)/etc/instoid.lst
	mkdir -p checktool/auto
	$(COPY) $(COPY_FLAGS) tr69_build_whole ./checktool/

clean:
	rm -f tr69_build
	rm -f $(INSTALL)/etc/protype.xml
	rm -f $(INSTALL)/etc/cfg.xml
	rm -f $(INSTALL)/etc/instoid.lst

