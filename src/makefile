

GCC=gcc
GXX=g++

#CXXFLAGS= -msse -msse2 -Wall \
#		  -pthread \
#		  -Wno-sign-compare -Wno-unused-local-typedefs -Winit-self \
#		  -rdynamic -DHAVE_POSIX_MEMALIGN -Wno-sign-compare \
#		  -Wno-unused-local-typedefs -Winit-self \
#		  -DHAVE_EXECINFO_H=1 -DHAVE_CXXABI_H -std=c++0x -frepo \
#		  -g -I.
#-MT front_tree.o -MD -MP -MF 1.Td
#
#CXXFLAGS=-msse -msse2 -Wall -I.. -pthread \
#		 -Wno-sign-compare -Wno-unused-local-typedefs -Winit-self \
#		 -rdynamic -std=c++0x \
#		 -g
CXXFLAGS=-Wno-unused-but-set-variable -msse -msse2 -Wall -I.. -pthread \
		 -std=c++0x -fPIC \
		 -O2 #-gdwarf-2 #-DDEBUG #-g -DDEBUG #-pg #-DDEBUG #-DDEBUGGRAPH

#CXXFLAGS += -fPIC

LDLIBS = -lm -lpthread -ldl

SRC=mkgraph.cpp
OBJ=mkgraph.o complie_grammar.o
BIN=complie_grammar parsedecode deepparsedecode graph_deep_decode libparse.a testapi


all:$(BIN)

complie_grammar:$(OBJ)
	$(GXX) -o $@ $^ 

parsedecode:parsedecode.o main_decode.o mkgraph.o convert.o
	$(GXX) -o $@ $^

deepparsedecode: main_deep_decode.o parse_deep_search_decode.o mkgraph.o convert.o
	$(GXX) -o $@ $^

graph_deep_decode:graph_deep_decode.o parse_deep_search_decode.o mkgraph.o convert.o
	$(GXX) -o $@ $^

libparse.a:parse_deep_search_decode.o mkgraph.o convert.o Api.o
	ar -rc $@ $^

testapi:testapi.o libparse.a
	$(GCC) $(CXXFLAGS) -lstdc++  -o $@ $^

%.o:%.c
	$(GXX) $(CXXFLAGS) -c  -o $@ $^

%.o:%.cpp
	$(GXX) $(CXXFLAGS) -c  -o $@ $^ 


.PHONY:

clean:
	rm -f *.o $(BIN)
