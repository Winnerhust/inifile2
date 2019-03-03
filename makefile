CC=g++ -Wall -std=c++11
GTEST_DIR = ./googletest
# GTEST_LIB_DIR = ./make
GTEST_LIB_DIR = ./googletest/gtestlib
# Where to find user code.
USER_DIR = ./src
GTEST_CPPFLAGS += -isystem $(GTEST_DIR)/include
CPPFLAGS += -g -Wall -Wextra -pthread -std=c++11 -I$(USER_DIR)
# -Dprotected=public -Dprivate=public
GTEST_LIBS = $(GTEST_LIB_DIR)/libgtest.a $(GTEST_LIB_DIR)/libgtest_main.a
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h
TESTS = inifile_test
.PHONY: all clean
all: testcase mytest

testcase: inifile_test.o inifile.o $(GTEST_LIBS)
	$(CC) $(CPPFLAGS) $(GTEST_CPPFLAGS) -L$(GTEST_LIB_DIR) -lgtest_main -lpthread $^ -o $@
inifile_test.o: $(USER_DIR)/$(TESTS).cpp $(USER_DIR)/inifile.h $(GTEST_HEADERS)
	$(CC) $(CPPFLAGS) -c $(USER_DIR)/$(TESTS).cpp
inifile.o: $(USER_DIR)/inifile.cpp $(USER_DIR)/inifile.h $(GTEST_HEADERS)
	$(CC) $(CPPFLAGS) -c $(USER_DIR)/inifile.cpp
# testcase:
	# $(CC) $(GTEST_CPPFLAGS) $(CPPFLAGS) -g -c src/inifile.cpp
	# $(CC) $(GTEST_CPPFLAGS) $(CPPFLAGS) -g -c test/inifile_test.cpp
	# $(CC) -g -o testcase inifile_test.o inifile.o -L$(GTEST_LIB_DIR) -lgtest_main -lpthread

mytest: inifile.o test/mytest.cpp
	$(CC) $(CPPFLAGS) -g -c test/mytest.cpp
	$(CC) $(CPPFLAGS) -g -o mytest mytest.o inifile.o
clean:
	rm -f *.o testcase mytest
