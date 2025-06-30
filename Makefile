###########################################
# Service Junction
# -------------------------------------
# file       : Makefile
# author     : Ben Kietzman
# begin      : 2011-04-13
# copyright  : Ben Kietzman
# email      : ben@kietzman.org
###########################################

all: bin/centralmon bin/junction

bin/centralmon: ../common/libcommon.a obj/centralmon.o
	-if [ ! -d bin ]; then mkdir bin; fi;
	g++ -o bin/centralmon obj/centralmon.o -L/data/extras/lib -L../common -lcommon -lexpat -lmjson

bin/junction: ../common/libcommon.a obj/junction.o
	-if [ ! -d bin ]; then mkdir bin; fi;
	g++ -o bin/junction obj/junction.o -L/data/extras/lib -L../common -lcommon -lb64 -lcrypto -lexpat -lmjson -lpthread -lrt -lssl -ltar -lz

../common/libcommon.a: ../common/Makefile
	cd ../common; ./configure; make;

../common/Makefile: ../common/configure
	cd ../common; ./configure;

../common/configure:
	cd ../; git clone https://github.com/benkietzman/common.git

obj/centralmon.o: centralmon.cpp ../common/Makefile
	-if [ ! -d obj ]; then mkdir obj; fi;
	g++ -g -Wall -c centralmon.cpp -o obj/centralmon.o -I/data/extras/include -I../common

obj/junction.o: junction.cpp ../common/Makefile
	-if [ ! -d obj ]; then mkdir obj; fi;
	g++ -fPIC -g -Wall -c junction.cpp -o obj/junction.o -I/data/extras/include -I../common

install: bin/centralmon bin/junction
	-if [ ! -d /usr/local/servicejunction ]; then mkdir /usr/local/servicejunction; fi;
	-if [ ! -d /usr/local/servicejunction/service ]; then mkdir /usr/local/servicejunction/service; fi;
	install --mode=777 bin/centralmon /usr/local/servicejunction/
	install --mode=777 bin/junction /usr/local/servicejunction/junction_preload
	if [ ! -f /lib/systemd/system/junction.service ]; then install --mode=644 junction.service /lib/systemd/system/; fi

clean:
	-rm -fr obj bin

uninstall:
	-rm -fr /usr/local/servicejunction
