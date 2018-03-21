#
# gcc.build.mk
#
# Should be included after "all", but before any "lib" or "EXE"
#


######## build objects in $(BUILD) directory

BUILD = ./build
BUILD_O = $(BUILD)/o
CLEAN_BUILD += $(BUILD)

$(BUILD_O) :
	 mkdir -p $@

LIB_O :=

define BUILD_temp
  TAR :=  $(BUILD_O)/$(notdir $(basename $(1)))
  LIB_O := $(LIB_O) $$(TAR).o
  $$(TAR).o : | $(BUILD_O)
  -include $$(TAR).d
  $$(TAR).o : ./$(1)
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) -o $$@ -MMD $$<
endef

$(foreach s,$(LIBSRCS),$(eval $(call BUILD_temp,$(s))))



######## clean

.PHONY: ftclean clean distclean
ftclean:
	rm -rf $(CLEAN_BUILD)

clean:
	rm -rf $(CLEAN_BUILD) $(CLEAN)

distclean:
	rm -rf $(CLEAN_BUILD)

