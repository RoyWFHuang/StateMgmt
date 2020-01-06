
ifneq ("$(wildcard ./STMconfig.mk)", "")
include STMconfig.mk
endif
ifeq ($(CFLAG),)
  SRC_ROOT = $(pwd)/src
endif

ifeq ($(STATEMANGMENT_LIB_NAME),)
  STATEMANGMENT_LIB_NAME = libstmgmt.a
endif

#MD_FILE = src/lib_metadata.c metadata.c
STM_FILE = src/state_mgmt.c src/fsm.c

STM_LIB_FILE = $(STM_FILE)


INCLUDE_DIR = \
-I$(SRC_ROOT) \
-I$(SRC_ROOT)/inc \
-I. \
-I./inc \
-I./h


all:
	$(CC) -c $(STM_LIB_FILE) $(CFLAG) $(INCLUDE_DIR)
	ar -r $(STATEMANGMENT_LIB_NAME) *.o
	rm -rf *.o
	mv $(STATEMANGMENT_LIB_NAME) ./lib/$(STATEMANGMENT_LIB_NAME)

debug:
	$(CC) -c $(STM_LIB_FILE) $(CFLAG) $(INCLUDE_DIR) $(DEBUG_FLAG)
	ar -r $(STATEMANGMENT_LIB_NAME) *.o
	rm -rf *.o
	mv $(STATEMANGMENT_LIB_NAME) ./lib/$(STATEMANGMENT_LIB_NAME)

clean:
	rm -rf *.o ./lib/$(STATEMANGMENT_LIB_NAME)

distclean: clean
	rm -rf stmtest

test :
	$(CC) -c $(STM_LIB_FILE) state_mgmt_test.c $(CFLAG) $(INCLUDE_DIR) $(DEBUG_FLAG) \
-DDEBUG_TEST -DCONSOLE_DEBUG
	$(CC) *.o -lpthread -o stmtest

