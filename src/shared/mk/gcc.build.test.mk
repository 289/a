#
# gcc.build.mk
#
# Should be included after "all", but before any "lib" or "EXE"
#


######## build objects in $(BUILD) directory

all : test

BUILD = ./build
BUILD_O = $(BUILD)/o
CLEAN_BUILD += $(BUILD)

$(BUILD_O) :
	 mkdir -p $@

$(BUILD) : $(BUILD_O)

TEST := 

define TEST_temp
  TAR :=  $(BUILD)/$(notdir $(basename $(1)))
  TEST := $(TEST) $$(TAR)
  $$(TAR) : | $(BUILD)
  $$(TAR) : $(LIBNAME)
  $$(TAR) : ./$(1) 
	cd $(BUILD) && $(CC) $(DEFINES) $(INCLUDES) $(CFLAGS) $(LDFLAGS) -I.. -L. -o $$(notdir $$@) ../$$< $(SHARED_STATIC_LIB) $(SYS_SHARE_LIB)
endef

$(foreach s,$(TESTSRCS),$(eval $(call TEST_temp,$(s))))

test : $(TEST)

   #cd $(BUILD) && $(CC) $(DEFINES) $(INCLUDES) $(CFLAGS) $(LDFLAGS) -I.. -L. -o $$(notdir $$@) ../$$< $(SHARED_STATIC_LIB) $(SYS_SHARE_LIB)

######## clean

.PHONY: ftclean clean distclean
ftclean:
	rm -rf $(CLEAN_BUILD)

clean:
	rm -rf $(CLEAN_BUILD)

distclean:
	rm -rf $(CLEAN_BUILD)

