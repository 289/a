AR  = ar
CPP = g++
CC  = g++
LD  = g++

SHARED_DIR = $(SOLUTION_ROOT_DIR)/shared

INCLUDES = -I/usr/include -I/usr/local/ssl/include -I$(SOLUTION_ROOT_DIR)

EXTERNAL_SLIB = /usr/local/lib/libprotobuf.a /usr/local/ssl/lib/libcrypto.a /usr/local/lib/liblua.a /usr/local/lib/libz.a

USE_SELF_CLEAN_OPT = false
CLEAN =

ifeq ($(SINGLE_THREAD),true)
	DEFINES = -Wall -D_FILE_OFFSET_BITS=64
	LDFLAGS = -pthread -lrt
	SHARED_STATIC_LIB = $(SHARED_DIR)/net/libnetio_st.a $(SHARED_DIR)/logsys/liblogsys_st.a $(SHARED_DIR)/base/libsharedbase_st.a
else
	DEFINES = -Wall -D_REENTRANT -D_FILE_OFFSET_BITS=64
	LDFLAGS = -pthread -lrt
	SHARED_STATIC_LIB = $(SHARED_DIR)/net/libnetio.a $(SHARED_DIR)/logsys/liblogsys.a $(SHARED_DIR)/base/libsharedbase.a
endif

ifeq ($(DEBUG_VERSION),true)
	DEFINES += -D_DEBUGINFO -D_DEBUG -g -O0
else
	DEFINES += -DNDEBUG
endif

