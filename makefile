
CC=g++
GTEST_CPPFLAGS=-I/usr/include -pthread 
GTEST_LDFLAGS=-L/usr/lib64 -lgtest -pthread

CPPFLAGS=-Isrc/ -Dprotected=public -Dprivate=public
all: testcase 

testcase:
	$(CC) $(GTEST_CPPFLAGS) $(CPPFLAGS) -g -c src/inifile.cpp
	$(CC) $(GTEST_CPPFLAGS) $(CPPFLAGS) -g -c src/stringutil.cpp
	$(CC) $(GTEST_CPPFLAGS) $(CPPFLAGS) -g -c test/inifile_test.cpp
	$(CC) $(GTEST_LDFLAGS) -g -o testcase inifile_test.o inifile.o stringutil.o
clean:	
	rm -f *.o testcase



	
#	g++  $(gtest-config --cppflags --cxxflags) -o $(TARGET).o -c test_main.cpp
#	g++ $(gtest-config --ldflags --libs) -o $(TARGET) $(TARGET).o
