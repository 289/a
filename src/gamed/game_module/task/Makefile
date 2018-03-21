SOLUTION_ROOT_DIR = ../../..

SINGLE_THREAD = false
DEBUG_VERSION = true

CURRENT_PROJECT_DIR = $(SOLUTION_ROOT_DIR)/gamed/game_module/task

include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.defs.mk

EXES_NAME = libtask.so
BIN_DIR = $(CURRENT_PROJECT_DIR)/bin
EXES = $(BIN_DIR)/$(EXES_NAME)

SLIB_NAME = libtask.a

INCLUDE_DIR = $(CURRENT_PROJECT_DIR)/include
TASK_DIR = $(CURRENT_PROJECT_DIR)/task
TEMPL_DIR = $(CURRENT_PROJECT_DIR)/template
UTIL_DIR = $(CURRENT_PROJECT_DIR)/utility

INCLUDES += -I./ -I$(INCLUDE_DIR) -I$(TASK_DIR) -I$(TEMPL_DIR) -I$(UTIL_DIR)

EXESRCS = \
		  $(UTIL_DIR)/cond_util.cpp \
		  $(UTIL_DIR)/entry_util.cpp \
		  $(UTIL_DIR)/notify_util.cpp \
		  $(UTIL_DIR)/op_util.cpp \
		  $(UTIL_DIR)/storage_util.cpp \
		  $(UTIL_DIR)/util.cpp \
		  $(TASK_DIR)/task.cpp \
		  $(TASK_DIR)/task_data.cpp \
		  $(TASK_DIR)/task_msg.cpp \
		  $(TASK_DIR)/task_wrapper.cpp \
		  $(TASK_DIR)/task_types.cpp \
		  $(TEMPL_DIR)/base_task_templ.cpp \
		  $(TEMPL_DIR)/task_templ.cpp \
		  $(TEMPL_DIR)/storage_templ.cpp \
		  $(TEMPL_DIR)/monster_package_templ.cpp \
		  $(TEMPL_DIR)/ratio_table_templ.cpp \
		  $(TEMPL_DIR)/task_manager.cpp \

all : SLIB

include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.exe.mk

$(EXES) : $(EXE_O)
	$(LD) $(LDFLAGS) -fPIC -shared -o $@ $(addprefix ./,$^) $(SHARED_STATIC_LIB) $(SYS_SHARE_LIB)

SLIB : $(EXE_O)
	cd $(BUILD) && $(AR) crs $(SLIB_NAME) $(addprefix ../,$^)

