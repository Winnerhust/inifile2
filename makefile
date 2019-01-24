
CC=g++ -Wall -std=c++11
GTEST_CPPFLAGS=-I${GTEST_DIR}/include -pthread -std=c++11
GTEST_LDFLAGS=-L${GTEST_DIR}/lib -lgtest -pthread

CPPFLAGS=-Isrc/ -Dprotected=public -Dprivate=public
all: testcase mytest

testcase:
	$(CC) $(GTEST_CPPFLAGS) $(CPPFLAGS) -g -c src/inifile.cpp
	# $(CC) $(GTEST_CPPFLAGS) $(CPPFLAGS) -g -c test/inifile_test.cpp 
	# $(CC) -g -o testcase inifile_test.o inifile.o $(GTEST_LDFLAGS) 
mytest: src/inifile.cpp test/mytest.cpp test/inifile_test.cpp 
	$(CC) $(GTEST_CPPFLAGS) $(CPPFLAGS) -g -c src/inifile.cpp
	$(CC) $(GTEST_CPPFLAGS) $(CPPFLAGS) -g -c test/mytest.cpp  
	$(CC) $(GTEST_CPPFLAGS) $(CPPFLAGS) -g -c test/error.cpp  
	$(CC) -g -o mytest mytest.o inifile.o $(GTEST_LDFLAGS) 
	$(CC) -g -o error error.o inifile.o $(GTEST_LDFLAGS) 
clean:	
	rm -f *.o testcase



	
#	g++  $(gtest-config --cppflags --cxxflags) -o $(TARGET).o -c test_main.cpp
#	g++ $(gtest-config --ldflags --libs) -o $(TARGET) $(TARGET).o
