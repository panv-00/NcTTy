CPP=g++
PKGS=
CFLAGS=-Wall -O3 -g -lssl -lcrypto
LIBS=
VPATH=../src

OBJECTS= \
	NrUI.o \
	NrIO.o \
	NrNet.o \
	NrScreen.o \

DEPLIST= \
	NrInclude.h \

default: $(OBJECTS)
	$(CPP) -o ../ncrev ../src/NcRev.cpp \
		$(OBJECTS) \
		$(CFLAGS) \
		$(LIBS)

%.o: %.cpp %.h $(DEPLIST)
	$(CPP) -c $< -o $*.o $(CFLAGS)
