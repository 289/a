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

EXE_O :=

define EXE_temp
  TAR :=  $(BUILD_O)/$(notdir $(basename $(1)))
  EXE_O := $(EXE_O) $$(TAR).o
  $$(TAR).o : | $(BUILD_O)
  -include $$(TAR).d
  $$(TAR).o : ./$(1)
	$(CC) -c $(DEFINES) $(INCLUDES) $(CFLAGS) -o $$@ -MMD $$<
endef

$(foreach s,$(EXESRCS),$(eval $(call EXE_temp,$(s))))


######## clean

.PHONY: ftclean clean distclean
ftclean:
	rm -rf $(CLEAN_BUILD)

ifeq ($(USE_SELF_CLEAN_OPT),false)
clean:
	rm -rf $(CLEAN_BUILD) $(EXES) $(CLEAN)
endif

distclean:
	rm -rf $(CLEAN_BUILD)

