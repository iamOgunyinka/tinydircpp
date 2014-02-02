all:main

CPPFLAGS+=-std=c++0x -Wall -pedantic -Wextra
CPPFLAGS+=-g -O2

main:main.cpp filehandler.cpp tinydircpp.cpp
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)
