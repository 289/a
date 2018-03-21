#
# gcc.rules.mk
#
# Should be included last, but before any rules overrides
#

######## Common targets

#all:

#.PHONY: kdocs

#kdocs:
#	-mkdir kdoc
#	-kdoc -d kdoc -n coldstore --strip-h-path *.hh

# the following format is not supported by FreeBSD.

#ifeq ($(SINGLE_THREAD),true)
#$(SHAREOBJ): %.o: %.cpp
#	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@
#$(LOGOBJ): %.o: %.cpp
#	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@
#$(LOGSTUB): %.o: %.cxx
#	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@
#else
#$(SHAREOBJ): %_m.o: %.cpp
#	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@
#$(LOGOBJ): %_m.o: %.cpp
#	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@
#$(LOGSTUB): %_m.o: %.cxx
#	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@
#endif

.c.o:
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@

.cpp.o:
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@

.s.o:
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@

.cxx.o:
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@

%.o : %.cxx
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) $< -o $@

# rule for making executables
%: %.o
	$(LD) $(LDFLAGS) -rdynamic -o $@ $^ $(LDLIBS)

%.so :
	$(LD) $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)

%.a :
	ar rsv $@ $^


######## clean

.PHONY: ftclean clean distclean
ftclean:
	rm -rf $(OBJS) $(EXES) $(CLEAN)

clean:
	rm -rf $(LIBOBJS) $(OBJS) $(EXES) $(CLEAN)

distclean:
	rm -rf $(SHAREOBJ) $(OUTEROBJS) $(LIBOBJS) $(OBJS) $(EXES) $(CLEAN) $(DISTCLEAN)

