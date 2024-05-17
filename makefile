
#
# PMTCalib makefile
# author : Leonidas N. Kalousis
# e-mail : leonidas.kalousis@iphc.cnrs.be
#

dict = PMTCalibDict
lib = lib/libPMTCalib.so
rootmap = $(lib:.so=.rootmap)

srcs = $(wildcard src/*.cc)
head = $(wildcard src/*.h)
objs = $(srcs:.cc=.o)

cxx = $(shell root-config --cxx)
cxxflags = -g -W -march=native -O -Wall -Wno-deprecated -Werror -fPIC -std=c++1y

incflags = -I.
incflags += $(shell root-config --cflags | sed -e "s/-I/-isystem/g")
GSL_PREFIX=$(shell gsl-config --prefix)
ifneq ($(GSL_PREFIX),/usr)
	incflags += $(shell gsl-config --cflags | sed -e "s/-I/-isystem/g")
endif

so = $(shell root-config --ld)
soflags = -g -shared -fPIC
libs += $(shell root-config --ldflags --libs) -lMinuit -lMinuit2 -lGeom -lXMLIO
libs += -lfftw3
libs += $(shell gsl-config --libs)

all	: start $(dict).cc $(objs) $(lib)

start	:
	mkdir -p lib work

$(dict).cc :
	rootcling -f $(dict).cc -s $(lib) -rml $(lib) -rmf $(rootmap) -c $(head) LinkDef.h

%.o	: %.cc
	$(cxx) $(cxxflags) $(incflags) -c -o $@ $<

$(lib) 	: $(objs) $(dict).o
	$(so) $(soflags) $(libs) -o $@ $(objs) $(dict).o

clean	:
	rm -f ./#* ./*~ ./*.*~	
	rm -f ./src/#* ./src/*~ ./src/*.*~
	rm -f ./mac/#* ./mac/*~ ./mac/*.*~
	rm -f $(dict)*.cc
	rm -f $(dict)*.o
	rm -f ./src/*.o
	rm -f ./$(lib)
	rm -f ./$(lib:.so=_rdict.pcm)
	rm -f ./$(lib:.so=.rootmap)
	make -C examples clean

fresh 	: clean all

.PHONY: examples
examples: all
	make -C examples all
