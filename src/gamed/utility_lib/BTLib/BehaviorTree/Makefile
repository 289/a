SOLUTION_ROOT_DIR = ../../../..

SINGLE_THREAD = false
DEBUG_VERSION = true

CURRENT_PROJECT_DIR = $(SOLUTION_ROOT_DIR)/gamed/utility_lib/BTLib


include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.defs.mk

SLIB_NAME = libBTs.a.1.0.0

INCLUDES += -I$(CURRENT_PROJECT_DIR)

LIBSRCS =  \
		   BTNode.cpp \
		   BTAssert.cpp \
		   BehaviorTree.cpp \
		   BTDecoratorNode.cpp \
		   BTLinkNode.cpp \
		   BTSelecNode.cpp \
		   BTSeqNode.cpp \
		   ClassLeafNode.cpp \
		   LeafFactory.cpp


all : lib

include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.lib.mk

lib : $(LIB_O)
	cd $(BUILD) && $(AR) crs $(SLIB_NAME) $(addprefix ../,$^)

