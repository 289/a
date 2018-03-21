SOLUTION_ROOT_DIR = ../../..

SINGLE_THREAD = false
DEBUG_VERSION = true

CURRENT_PROJECT_DIR = $(SOLUTION_ROOT_DIR)/gamed/game_module/achieve

include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.defs.mk

EXES_NAME = libachieve.so
BIN_DIR = $(CURRENT_PROJECT_DIR)/bin
EXES = $(BIN_DIR)/$(EXES_NAME)

SLIB_NAME = libachieve.a

INCLUDE_DIR = $(CURRENT_PROJECT_DIR)/include
ACHIEVE_DIR = $(CURRENT_PROJECT_DIR)/achieve
TEMPL_DIR = $(CURRENT_PROJECT_DIR)/template
UTIL_DIR = $(CURRENT_PROJECT_DIR)/utility

INCLUDES += -I./ -I$(INCLUDE_DIR) -I$(ACHIEVE_DIR) -I$(TEMPL_DIR) -I$(UTIL_DIR)

EXESRCS = \
		  $(ACHIEVE_DIR)/achieve_types.cpp \
		  $(ACHIEVE_DIR)/achieve_data.cpp \
		  $(ACHIEVE_DIR)/achieve_msg.cpp \
		  $(ACHIEVE_DIR)/achieve.cpp \
		  $(ACHIEVE_DIR)/achieve_wrapper.cpp \
		  $(TEMPL_DIR)/base_achieve_templ.cpp \
		  $(TEMPL_DIR)/achieve_templ.cpp \
		  $(TEMPL_DIR)/achieve_manager.cpp \
		  $(UTIL_DIR)/expression.cpp \
		  $(UTIL_DIR)/achieve_expr.cpp \
		  $(UTIL_DIR)/achieve_notify.cpp \

all : SLIB

include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.exe.mk

$(EXES) : $(EXE_O)
	$(LD) $(LDFLAGS) -fPIC -shared -o $@ $(addprefix ./,$^) $(SHARED_STATIC_LIB) $(SYS_SHARE_LIB)

SLIB : $(EXE_O)
	cd $(BUILD) && $(AR) crs $(SLIB_NAME) $(addprefix ../,$^)

