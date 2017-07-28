CPPUTEST_HOME = ../cpputest
CPPFLAGS += -I$(CPPUTEST_HOME)/include -I../ -DUTEST

LD_LIBRARIES = -L$(CPPUTEST_HOME)/lib -lCppUTest -lCppUTestExt

OBJ = AllTests.o XModem.o

all: AllTests

AllTests: $(OBJ)
	g++ $(CPPFLAGS) -o $@ $(OBJ) $(LD_LIBRARIES)

AllTests.o: AllTests.cpp
	g++ $(CPPFLAGS) -o $@ -c $< 

XModem.o: ../XModem.cpp ../XModem.h
	g++ $(CPPFLAGS) -o $@ -c $<

clean:
	rm *.o
	rm AllTests


