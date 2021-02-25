###########################################
# Service Junction
# -------------------------------------
# file       : Makefile
# author     : Ben Kietzman
# begin      : 2011-12-09
# copyright  : kietzman.org
# email      : ben@kietzman.org
###########################################

#MAKEFLAGS="-j ${C}"
MAKE_UNAME=`uname -s`

all: bin/acorn bin/acronym bin/addrInfo bin/curl bin/format bin/google bin/jira bin/jwt bin/logger bin/mssql bin/mysql bin/password bin/pdf bin/ping bin/portConcentrator bin/samba bin/transmission bin/zoneMinder

bin/acorn: ../../common/libcommon.a obj/functions.o obj/acorn.o bin
	g++ -o bin/acorn obj/acorn.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz

bin/acronym: ../../common/libcommon.a obj/functions.o obj/acronym.o bin
	g++ -o bin/acronym obj/acronym.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz

bin/addrInfo: ../../common/libcommon.a obj/functions.o obj/addrInfo.o bin
	g++ -o bin/addrInfo obj/addrInfo.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz

bin/curl: ../../common/libcommon.a obj/functions.o obj/curl.o bin
	g++ -o bin/curl obj/curl.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz

bin/format: ../../common/libcommon.a obj/functions.o obj/format.o bin
	g++ -o bin/format obj/format.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz

bin/google: ../../common/libcommon.a obj/functions.o obj/google.o bin
	g++ -o bin/google obj/google.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz

bin/jira: ../../common/libcommon.a obj/functions.o obj/jira.o bin
	g++ -o bin/jira obj/jira.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz

bin/jwt: ../../common/libcommon.a obj/functions.o obj/jwt.o bin
	g++ -o bin/jwt obj/jwt.o obj/functions.o -L/usr/local/lib -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -ljwt -lmjson -lnsl -lpthread -lssl -ltar -lz

bin/logger: ../../common/libcommon.a obj/functions.o obj/logger.o bin
	g++ -o bin/logger obj/logger.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz

bin/mssql: ../../common/libcommon.a obj/functions.o obj/mssql.o bin
	g++ -o bin/mssql obj/mssql.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -lsybdb -ltar -lz

bin/mysql: ../../common/libcommon.a obj/functions.o obj/mysql.o bin
	g++ -o bin/mysql obj/mysql.o obj/functions.o -L/usr/lib64/mysql -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lmysqlclient -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz

bin/password: ../../common/libcommon.a obj/functions.o obj/password.o bin
	g++ -o bin/password obj/password.o obj/functions.o -L/usr/lib64/mysql -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lmysqlclient -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz

bin/pdf: ../../common/libcommon.a obj/functions.o obj/pdf.o bin
	g++ -o bin/pdf obj/pdf.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpoppler-cpp -lpthread -lssl -ltar -lz

bin/ping: ../../common/libcommon.a obj/functions.o obj/ping.o bin
	g++ -o bin/ping obj/ping.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz

bin/portConcentrator: ../../common/libcommon.a obj/functions.o obj/portConcentrator.o bin
	g++ -o bin/portConcentrator obj/portConcentrator.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz

bin/samba: ../../common/libcommon.a obj/functions.o obj/samba.o bin
	g++ -o bin/samba obj/samba.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lsmbclient -lssl -ltar -lz

bin/transmission: ../../common/libcommon.a obj/functions.o obj/transmission.o bin
	g++ -o bin/transmission obj/transmission.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz

bin/zoneMinder: ../../common/libcommon.a obj/functions.o obj/zoneMinder.o bin
	g++ -o bin/zoneMinder obj/zoneMinder.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz

bin:
	if [ ! -d bin ]; then mkdir bin; fi;

../../common/libcommon.a:
	cd ../../common; ./configure; make;

obj/functions.o: include/functions.cpp obj
	g++ -g -Wall -c include/functions.cpp -o obj/functions.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;

obj/acorn.o: acorn.cpp obj
	g++ -g -Wall -c acorn.cpp -o obj/acorn.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;

obj/acronym.o: acronym.cpp obj
	g++ -g -Wall -c acronym.cpp -o obj/acronym.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;

obj/addrInfo.o: addrInfo.cpp obj
	g++ -g -Wall -c addrInfo.cpp -o obj/addrInfo.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;

obj/curl.o: curl.cpp obj
	g++ -g -Wall -c curl.cpp -o obj/curl.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;

obj/format.o: format.cpp obj
	g++ -g -Wall -c format.cpp -o obj/format.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;

obj/google.o: google.cpp obj
	g++ -g -Wall -c google.cpp -o obj/google.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;

obj/jira.o: jira.cpp obj
	g++ -g -Wall -c jira.cpp -o obj/jira.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;

obj/jwt.o: jwt.cpp obj
	g++ -g -Wall -c jwt.cpp -o obj/jwt.o -I/usr/local/include -I/usr/local/ssl/include -I/data/extras/include -I../../common;

obj/logger.o: logger.cpp obj
	g++ -g -Wall -c logger.cpp -o obj/logger.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;

obj/mssql.o: mssql.cpp obj
	g++ -g -Wall -c mssql.cpp -o obj/mssql.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;

obj/mysql.o: mysql.cpp obj
	g++ -g -Wall -c mysql.cpp -o obj/mysql.o -I/usr/local/mysql/include -I/usr/local/ssl/include -I/data/extras/include -I../../common;

obj/password.o: password.cpp obj
	g++ -g -Wall -c password.cpp -o obj/password.o -I/usr/local/mysql/include -I/usr/local/ssl/include -I/data/extras/include -I../../common;

obj/pdf.o: pdf.cpp obj
	g++ -g -Wall -c pdf.cpp -o obj/pdf.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;

obj/ping.o: ping.cpp obj
	g++ -g -Wall -c ping.cpp -o obj/ping.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;

obj/portConcentrator.o: portConcentrator.cpp obj
	g++ -g -Wall -c portConcentrator.cpp -o obj/portConcentrator.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;

obj/samba.o: samba.cpp obj
	g++ -g -Wall -c samba.cpp -o obj/samba.o -I/usr/include/samba-4.0 -I/usr/local/ssl/include -I/data/extras/include -I../../common;

obj/transmission.o: transmission.cpp obj
	g++ -g -Wall -c transmission.cpp -o obj/transmission.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;

obj/zoneMinder.o: zoneMinder.cpp obj
	g++ -g -Wall -c zoneMinder.cpp -o obj/zoneMinder.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;

obj:
	if [ ! -d obj ]; then mkdir obj; fi;

clean:
	-rm -fr obj bin
