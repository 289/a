SOLUTION_ROOT_DIR = ../../..

SINGLE_THREAD = false
DEBUG_VERSION = true

CURRENT_PROJECT_DIR = $(SOLUTION_ROOT_DIR)/gamed/utility_lib/BTLib


include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.defs.mk

SLIB_NAME = libBTs.a.1.0.0

INCLUDES += -I$(CURRENT_PROJECT_DIR) -I$(SOLUTION_ROOT_DIR)/gamed/utility_lib

BEHAVIORTREE_DIR = ./BehaviorTree
INTEGRATION_DIR  = ./Integration

LIBSRCS =  \
		   $(BEHAVIORTREE_DIR)/BTNode.cpp \
		   $(BEHAVIORTREE_DIR)/BTAssert.cpp \
		   $(BEHAVIORTREE_DIR)/BehaviorTree.cpp \
		   $(BEHAVIORTREE_DIR)/BTDecoratorNode.cpp \
		   $(BEHAVIORTREE_DIR)/BTLinkNode.cpp \
		   $(BEHAVIORTREE_DIR)/BTSelecNode.cpp \
		   $(BEHAVIORTREE_DIR)/BTSeqNode.cpp \
		   $(BEHAVIORTREE_DIR)/ClassLeafNode.cpp \
		   $(BEHAVIORTREE_DIR)/LeafFactory.cpp \
		   $(INTEGRATION_DIR)/BehaviorTreeLibrary.cpp \
		   $(INTEGRATION_DIR)/ConditionFailure.cpp \
		   $(INTEGRATION_DIR)/ConditionSuccessful.cpp \
		   $(INTEGRATION_DIR)/LoadBTs.cpp \
		   $(INTEGRATION_DIR)/Nop.cpp


all : lib

include $(SOLUTION_ROOT_DIR)/shared/mk/gcc.build.lib.mk

lib : $(LIB_O)
	cd $(BUILD) && $(AR) crs $(SLIB_NAME) $(addprefix ../,$^)

