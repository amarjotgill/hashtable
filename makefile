CXX = g++
CXXFLAGS = -Wall

test: file.o hash.o mytest.cpp
	$(CXX) $(CXXFLAGS) file.o hash.o mytest.cpp -o test

hash.o: file.o hash.cpp hash.h
	$(CXX) $(CXXFLAGS) -c hash.cpp

file.o: file.cpp file.h
	$(CXX) $(CXXFLAGS) -c file.cpp

clean:
	rm *.o*
	rm *~ 
run:
	./test
val:
	valgrind ./test
