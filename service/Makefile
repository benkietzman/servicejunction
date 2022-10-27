###########################################
# Service Junction
# -------------------------------------
# file       : Makefile
# author     : Ben Kietzman
# begin      : 2011-12-09
# copyright  : kietzman.org
# email      : ben@kietzman.org
###########################################
# {{{ prep work
MAKE_UNAME=`uname -s`
prefix=/usr/local
all: bin/acorn bin/acronym bin/addrInfo bin/aes bin/bls bin/curl bin/format bin/google bin/jira bin/jwt bin/logger bin/mssql bin/mysql bin/password bin/pdf bin/ping bin/portConcentrator bin/samba bin/transmission bin/warden bin/zoneMinder
install: bin/acorn bin/acronym bin/addrInfo bin/aes bin/bls central.php bin/curl email.php finance.php bin/format functions.php bin/google ircBot.php bin/jira bin/jwt listServices.php bin/logger bin/mssql bin/mysql oracle.php bin/password bin/pdf bin/ping bin/portConcentrator bin/samba soap.php sysInfo.php tcpdf.php bin/transmission bin/warden weather.php bin/zoneMinder
	install --mode=775 bin/acorn $(prefix)/servicejunction/service/
	install --mode=775 bin/acronym $(prefix)/servicejunction/service/
	install --mode=775 bin/addrInfo $(prefix)/servicejunction/service/
	install --mode=775 bin/aes $(prefix)/servicejunction/service/
	install --mode=775 bin/bls $(prefix)/servicejunction/service/
	install --mode=664 central.php $(prefix)/servicejunction/service/
	install --mode=775 bin/curl $(prefix)/servicejunction/service/
	install --mode=664 email.php $(prefix)/servicejunction/service/
	install --mode=664 finance.php $(prefix)/servicejunction/service/
	install --mode=775 bin/format $(prefix)/servicejunction/service/
	install --mode=664 functions.php $(prefix)/servicejunction/service/
	install --mode=775 bin/google $(prefix)/servicejunction/service/
	install --mode=664 ircBot.php $(prefix)/servicejunction/service/
	install --mode=775 bin/jira $(prefix)/servicejunction/service/
	install --mode=775 bin/jwt $(prefix)/servicejunction/service/
	install --mode=664 listServices.php $(prefix)/servicejunction/service/
	install --mode=664 location.php $(prefix)/servicejunction/service/
	install --mode=775 bin/logger $(prefix)/servicejunction/service/
	install --mode=775 bin/mssql $(prefix)/servicejunction/service/
	install --mode=775 bin/mysql $(prefix)/servicejunction/service/
	install --mode=664 oracle.php $(prefix)/servicejunction/service/
	install --mode=775 bin/password $(prefix)/servicejunction/service/
	install --mode=775 bin/pdf $(prefix)/servicejunction/service/
	install --mode=775 bin/ping $(prefix)/servicejunction/service/
	install --mode=775 bin/portConcentrator $(prefix)/servicejunction/service/
	install --mode=775 bin/samba $(prefix)/servicejunction/service/
	install --mode=664 soap.php $(prefix)/servicejunction/service/
	install --mode=664 sysInfo.php $(prefix)/servicejunction/service/
	install --mode=664 tcpdf.php $(prefix)/servicejunction/service/
	install --mode=775 bin/teradata $(prefix)/servicejunction/service/
	install --mode=775 bin/transmission $(prefix)/servicejunction/service/
	install --mode=775 bin/warden $(prefix)/servicejunction/service/
	install --mode=664 weather.php $(prefix)/servicejunction/service/
	install --mode=775 bin/zoneMinder $(prefix)/servicejunction/service/
# }}}

# {{{ acorn
acorn: bin/acorn
	install --mode=775 bin/acorn $(prefix)/servicejunction/service/
bin/acorn: ../../common/libcommon.a obj/functions.o obj/acorn.o bin
	g++ -o bin/acorn obj/acorn.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz
obj/acorn.o: acorn.cpp obj ../../common/Makefile
	g++ -g -Wall -c acorn.cpp -o obj/acorn.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}
# {{{ acronym
acronym: bin/acronym
	install --mode=775 bin/acronym $(prefix)/servicejunction/service/
bin/acronym: ../../common/libcommon.a obj/functions.o obj/acronym.o bin
	g++ -o bin/acronym obj/acronym.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz
obj/acronym.o: acronym.cpp obj ../../common/Makefile
	g++ -g -Wall -c acronym.cpp -o obj/acronym.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}
# {{{ addrInfo
addrInfo: bin/addrInfo
	install --mode=775 bin/addrInfo $(prefix)/servicejunction/service/
bin/addrInfo: ../../common/libcommon.a obj/functions.o obj/addrInfo.o bin
	g++ -o bin/addrInfo obj/addrInfo.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz
obj/addrInfo.o: addrInfo.cpp obj ../../common/Makefile
	g++ -g -Wall -c addrInfo.cpp -o obj/addrInfo.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}
# {{{ aes
aes: bin/aes
	install --mode=775 bin/aes $(prefix)/servicejunction/service/
bin/aes: ../../common/libcommon.a obj/functions.o obj/aes.o bin
	g++ -o bin/aes obj/aes.o obj/functions.o -L/usr/local/lib -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz
obj/aes.o: aes.cpp obj ../../common/Makefile
	g++ -g -Wall -c aes.cpp -o obj/aes.o -I/usr/local/include -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}
# {{{ bls
bls: bin/bls
	install --mode=775 bin/bls $(prefix)/servicejunction/service/
bin/bls: ../../common/libcommon.a obj/functions.o obj/bls.o bin
	g++ -o bin/bls obj/bls.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz
obj/bls.o: bls.cpp obj ../../common/Makefile
	g++ -g -Wall -c bls.cpp -o obj/bls.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}
# {{{ central
central: central.php functions.php
	install --mode=664 central.php $(prefix)/servicejunction/service/
# }}}
# {{{ curl
curl: bin/curl
	install --mode=775 bin/curl $(prefix)/servicejunction/service/
bin/curl: ../../common/libcommon.a obj/functions.o obj/curl.o bin
	g++ -o bin/curl obj/curl.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz
obj/curl.o: curl.cpp obj ../../common/Makefile
	g++ -g -Wall -c curl.cpp -o obj/curl.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}
# {{{ email
email: email.php functions.php
	install --mode=664 email.php $(prefix)/servicejunction/service/
# }}}
# {{{ finance
finance: finance.php functions.php
	install --mode=664 finance.php $(prefix)/servicejunction/service/
# }}}
# {{{ format
format: bin/format
	install --mode=775 bin/format $(prefix)/servicejunction/service/
bin/format: ../../common/libcommon.a obj/functions.o obj/format.o bin
	g++ -o bin/format obj/format.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz
obj/format.o: format.cpp obj ../../common/Makefile
	g++ -g -Wall -c format.cpp -o obj/format.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}
# {{{ functions.php
functions.php:
	install --mode=664 functions.php $(prefix)/servicejunction/service/
# }}}
# {{{ google
google: bin/google
	install --mode=775 bin/google $(prefix)/servicejunction/service/
bin/google: ../../common/libcommon.a obj/functions.o obj/google.o bin
	g++ -o bin/google obj/google.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz
obj/google.o: google.cpp obj ../../common/Makefile
	g++ -g -Wall -c google.cpp -o obj/google.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}
# {{{ ircBot
ircBot: ircBot.php functions.php
	install --mode=664 ircBot.php $(prefix)/servicejunction/service/
# }}}
# {{{ jira
jira: bin/jira
	install --mode=775 bin/jira $(prefix)/servicejunction/service/
bin/jira: ../../common/libcommon.a obj/functions.o obj/jira.o bin
	g++ -o bin/jira obj/jira.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz
obj/jira.o: jira.cpp obj ../../common/Makefile
	g++ -g -Wall -c jira.cpp -o obj/jira.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}
# {{{ jwt
jwt: bin/jwt
	install --mode=775 bin/jwt $(prefix)/servicejunction/service/
bin/jwt: ../../common/libcommon.a obj/functions.o obj/jwt.o bin
	g++ -o bin/jwt obj/jwt.o obj/functions.o -L/usr/local/lib -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -ljwt -lmjson -lnsl -lpthread -lssl -ltar -lz
obj/jwt.o: jwt.cpp obj ../../common/Makefile
	g++ -g -Wall -c jwt.cpp -o obj/jwt.o -I/usr/local/include -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}
# {{{ listServices
listServices: listServices.php functions.php
	install --mode=664 listServices.php $(prefix)/servicejunction/service/
# }}}
# {{{ location
location: location.php functions.php
	install --mode=664 location.php $(prefix)/servicejunction/service/
# }}}
# {{{ logger
logger: bin/logger
	install --mode=775 bin/logger $(prefix)/servicejunction/service/
bin/logger: ../../common/libcommon.a obj/functions.o obj/logger.o bin
	g++ -o bin/logger obj/logger.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz
obj/logger.o: logger.cpp obj ../../common/Makefile
	g++ -g -Wall -c logger.cpp -o obj/logger.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}
# {{{ mssql
mssql: bin/mssql
	install --mode=775 bin/mssql $(prefix)/servicejunction/service/
bin/mssql: ../../common/libcommon.a obj/functions.o obj/mssql.o bin
	g++ -o bin/mssql obj/mssql.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -lsybdb -ltar -lz
obj/mssql.o: mssql.cpp obj ../../common/Makefile
	g++ -g -Wall -c mssql.cpp -o obj/mssql.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}
# {{{ mysql
mysql: bin/mysql
	install --mode=775 bin/mysql $(prefix)/servicejunction/service/
bin/mysql: ../../common/libcommon.a obj/functions.o obj/mysql.o bin
	g++ -o bin/mysql obj/mysql.o obj/functions.o -L/usr/lib64/mysql -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lmysqlclient -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz
obj/mysql.o: mysql.cpp obj ../../common/Makefile
	g++ -g -Wall -c mysql.cpp -o obj/mysql.o -I/usr/local/mysql/include -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}
# {{{ oracle
oracle: oracle.php functions.php
	install --mode=664 oracle.php $(prefix)/servicejunction/service/
# }}}
# {{{ password
password: bin/password
	install --mode=775 bin/password $(prefix)/servicejunction/service/
bin/password: ../../common/libcommon.a obj/functions.o obj/password.o bin
	g++ -o bin/password obj/password.o obj/functions.o -L/usr/lib64/mysql -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lmysqlclient -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz
obj/password.o: password.cpp obj ../../common/Makefile
	g++ -g -Wall -c password.cpp -o obj/password.o -I/usr/local/mysql/include -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}
# {{{ pdf
pdf: bin/pdf
	install --mode=775 bin/pdf $(prefix)/servicejunction/service/
bin/pdf: ../../common/libcommon.a obj/functions.o obj/pdf.o bin
	g++ -o bin/pdf obj/pdf.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpoppler-cpp -lpthread -lssl -ltar -lz
obj/pdf.o: pdf.cpp obj ../../common/Makefile
	g++ -g -Wall -c pdf.cpp -o obj/pdf.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}
# {{{ ping
ping: bin/ping
	install --mode=775 bin/ping $(prefix)/servicejunction/service/
bin/ping: ../../common/libcommon.a obj/functions.o obj/ping.o bin
	g++ -o bin/ping obj/ping.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz
obj/ping.o: ping.cpp obj ../../common/Makefile
	g++ -g -Wall -c ping.cpp -o obj/ping.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}
# {{{ portConcentrator
portConcentrator: bin/portConcentrator
	install --mode=775 bin/portConcentrator $(prefix)/servicejunction/service/
bin/portConcentrator: ../../common/libcommon.a obj/functions.o obj/portConcentrator.o bin
	g++ -o bin/portConcentrator obj/portConcentrator.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz
obj/portConcentrator.o: portConcentrator.cpp obj ../../common/Makefile
	g++ -g -Wall -c portConcentrator.cpp -o obj/portConcentrator.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}
# {{{ samba
samba: bin/samba
	install --mode=775 bin/samba $(prefix)/servicejunction/service/
bin/samba: ../../common/libcommon.a obj/functions.o obj/samba.o bin
	g++ -o bin/samba obj/samba.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lsmbclient -lssl -ltar -lz
obj/samba.o: samba.cpp obj ../../common/Makefile
	g++ -g -Wall -c samba.cpp -o obj/samba.o -I/usr/include/samba-4.0 -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}
# {{{ soap
soap: soap.php functions.php
	install --mode=664 soap.php $(prefix)/servicejunction/service/
# }}}
# {{{ sysInfo
sysInfo: sysInfo.php functions.php
	install --mode=664 sysInfo.php $(prefix)/servicejunction/service/
# }}}
# {{{ tcpdf
tcpdf: tcpdf.php functions.php
	install --mode=664 tcpdf.php $(prefix)/servicejunction/service/
# }}}
# {{{ teradata
teradata: bin/teradata
	install --mode=775 bin/teradata $(prefix)/servicejunction/service/
bin/teradata: ../../common/libcommon.a obj/functions.o obj/teradata.o bin
	g++ -o bin/teradata obj/teradata.o obj/functions.o -L/usr/lib64/teradata -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lodbc -lpthread -lssl -ltar -lz
obj/teradata.o: teradata.cpp obj ../../common/Makefile
	g++ -g -Wall -c teradata.cpp -o obj/teradata.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}
# {{{ transmission
transmission: bin/transmission
	install --mode=775 bin/transmission $(prefix)/servicejunction/service/
bin/transmission: ../../common/libcommon.a obj/functions.o obj/transmission.o bin
	g++ -o bin/transmission obj/transmission.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz
obj/transmission.o: transmission.cpp obj ../../common/Makefile
	g++ -g -Wall -c transmission.cpp -o obj/transmission.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}
# {{{ warden
warden: bin/warden
	install --mode=775 bin/warden $(prefix)/servicejunction/service/
bin/warden: ../../common/libcommon.a obj/functions.o obj/warden.o bin
	g++ -o bin/warden obj/warden.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz
obj/warden.o: warden.cpp obj ../../common/Makefile
	g++ -g -Wall -c warden.cpp -o obj/warden.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}
# {{{ weather
weather: weather.php functions.php
	install --mode=664 weather.php $(prefix)/servicejunction/service/
# }}}
# {{{ zoneMinder
zoneMinder: bin/zoneMinder
	install --mode=775 bin/zoneMinder $(prefix)/servicejunction/service/
bin/zoneMinder: ../../common/libcommon.a obj/functions.o obj/zoneMinder.o bin
	g++ -o bin/zoneMinder obj/zoneMinder.o obj/functions.o -L/data/extras/lib -L../../common -lcommon -lb64 -lcrypto -lcurl -lexpat -lmjson -lnsl -lpthread -lssl -ltar -lz
obj/zoneMinder.o: zoneMinder.cpp obj ../../common/Makefile
	g++ -g -Wall -c zoneMinder.cpp -o obj/zoneMinder.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;
# }}}

# {{{ post work
bin:
	if [ ! -d bin ]; then mkdir bin; fi;
../../common/libcommon.a: ../../common/Makefile
	cd ../../common; make;
../../common/Makefile: ../../common/configure
	cd ../../common; ./configure;
../../common/configure:
	cd ../../; git clone https://github.com/benkietzman/common.git
obj/functions.o: include/functions.cpp obj ../../common/Makefile
	g++ -g -Wall -c include/functions.cpp -o obj/functions.o -I/usr/local/ssl/include -I/data/extras/include -I../../common;
obj:
	if [ ! -d obj ]; then mkdir obj; fi;
clean:
	-rm -fr obj bin
# }}}
