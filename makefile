
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
cxxflags = -g -W -O -Wall -Wno-deprecated -Werror -fPIC -std=c++1y

incflags = -I.
incflags += $(shell root-config --cflags | sed -e "s/-I/-isystem/g")
GSL_PREFIX=$(shell gsl-config --prefix)
ifneq ($(GSL_PREFIX),/usr)
	incflags += $(shell gsl-config --cflags | sed -e "s/-I/-isystem/g")
endif
incflags += -I/$(PMTCALIB)/src/

so = $(shell root-config --ld)
soflags = -g -shared -fPIC
libs += $(shell root-config --ldflags --libs) -lMinuit -lMinuit2 -lGeom -lXMLIO
libs += -lfftw3
libs += `gsl-config --libs`

all	: start $(dict).cc $(objs) $(lib) end

start	: 
	@echo ''		
	@echo ' * PMTCalib make ... '
	@echo ''
	@rm -f ./#* ./*~ ./*.*~	
	@rm -f ./src/#* ./src/*~ ./src/*.*~
	@rm -f ./mac/#* ./mac/*~ ./mac/*.*~
	@mkdir -p lib work

$(dict).cc : 
	@rootcling -f $(dict).cc -s $(lib) -rml $(lib) -rmf $(rootmap) $(incflags) -c $(head) LinkDef.h 
	@echo ' * Building ( dic ) :' $@
	@echo ''

%.o	: %.cc	
	@$(cxx) $(cxxflags) $(incflags) -c -o $@ $<
	@echo ' * Building ( obj ) :' $@
	@echo ''

$(lib) 	: $(objs) $(dict).o
	@$(so) $(soflags) $(libs) -o $@ $(objs) $(dict).o
	@echo ' * Building ( lib ) :' $@
	@echo ''

end	:
	@echo ' * PMTCalib done !'
	@echo ''

clean	:	
	@echo ''	
	@echo ' * PMTCalib clean ...'
	@echo ''
	@rm -f ./#* ./*~ ./*.*~	
	@rm -f ./src/#* ./src/*~ ./src/*.*~
	@rm -f ./mac/#* ./mac/*~ ./mac/*.*~
	@rm -f $(dict)*.cc
	@rm -f $(dict)*.o
	@rm -f ./src/*.o
	@rm -f ./$(lib)
	@rm -f ./$(lib:.so=_rdict.pcm)
	@rm -f ./$(lib:.so=.rootmap)
	@make -C examples clean

fresh 	: clean all 

.PHONY: examples
examples:
	@make -C examples all
