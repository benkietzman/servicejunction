###########################################
# Service Junction
# -------------------------------------
# file       : Makefile
# author     : Ben Kietzman
# begin      : 2011-04-13
# copyright  : kietzman.org
# email      : ben@kietzman.org
###########################################

all: bin/junction

bin/junction: ../common/libcommon.a obj/junction.o
	-if [ ! -d bin ]; then mkdir bin; fi;
	g++ -o bin/junction obj/junction.o -L/data/extras/lib -L../common -lcommon -lb64 -lcrypto -lexpat -lmjson -lnsl -lpthread -lrt -lssl -ltar -lz

../common/libcommon.a:
	cd ../common; ./configure; make;

obj/junction.o: junction.cpp
	-if [ ! -d obj ]; then mkdir obj; fi;
	g++ -fPIC -g -std=c++14 -Wall -c junction.cpp -o obj/junction.o -I/data/extras/include -I../common

install: bin/junction
	-if [ ! -d /usr/local/servicejunction ]; then mkdir /usr/local/servicejunction; fi;
	-if [ ! -d /usr/local/servicejunction/service ]; then mkdir /usr/local/servicejunction/service; fi;
	install --mode=777 bin/junction /usr/local/servicejunction/junction_preload
	if [ ! -f /lib/systemd/system/junction.service ]; then install --mode=644 junction.service /lib/systemd/system/; fi

clean:
	-rm -fr obj bin

uninstall:
	-rm -fr /usr/local/servicejunction
