APP=toxrelay
VERSION=0.01
RELEASE=1
ARCH_TYPE=`uname -m`

all:
	gcc -std=gnu99 -DAL_LIBTYPE_STATIC -O3 -o ${APP} -Isrc src/*.c -ltoxcore -lsodium -lncurses -lrt -lpthread
debug:
	gcc -std=gnu99 -DAL_LIBTYPE_STATIC -g -o ${APP} -Isrc src/*.c -ltoxcore -lsodium -lncurses -lrt -lpthread
source:
	tar -cvzf ../${APP}_${VERSION}.orig.tar.gz ../${APP}-${VERSION} --exclude-vcs
install:
	mkdir -p ${DESTDIR}/usr/bin
	install -m 755 --strip ${APP} ${DESTDIR}/usr/bin
	mkdir -m 755 -p ${DESTDIR}/usr/share/man/man1
	install -m 644 man/${APP}.1.gz ${DESTDIR}/usr/share/man/man1
clean:
	rm -f ${APP} \#* \.#*



