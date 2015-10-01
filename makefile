
CC=g++ -Wall
GTEST_CPPFLAGS=-I${GTEST_DIR}/include -pthread 
GTEST_LDFLAGS=-L${GTEST_DIR}/lib -lgtest -pthread

CPPFLAGS=-Isrc/ -Dprotected=public -Dprivate=public
all: testcase 

testcase:
	$(CC) $(GTEST_CPPFLAGS) $(CPPFLAGS) -g -c src/inifile.cpp
	$(CC) $(GTEST_CPPFLAGS) $(CPPFLAGS) -g -c test/inifile_test.cpp 
	$(CC) -g -o testcase inifile_test.o inifile.o $(GTEST_LDFLAGS) 
clean:	
	rm -f *.o testcase



	
#	g++  $(gtest-config --cppflags --cxxflags) -o $(TARGET).o -c test_main.cpp
#	g++ $(gtest-config --ldflags --libs) -o $(TARGET) $(TARGET).o
