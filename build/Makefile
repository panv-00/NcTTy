CPP=g++
PKGS=
CFLAGS=-Wall -O3 -g -lssl -lcrypto
LIBS=
VPATH=../src

OBJECTS= \
	NcTTyUi.o \
	NcTTyNet.o \

DEPLIST= \

default: $(OBJECTS)
	$(CPP) -o ../nctty ../src/NcTTy.cpp \
		$(OBJECTS) \
		$(CFLAGS) \
		$(LIBS)

%.o: %.cpp %.h $(DEPLIST)
	$(CPP) -c $< -o $*.o $(CFLAGS)
