CC	:= gcc
CFLAGS	:= -Iinclude -g
LDFLAGS	:= -lcrypto -lssl -lgmp -lz
TARGETS	:= package_finalize make_self_npdrm make_self_sprx make_self_nocrypt

all: make_self.c package_finalize.c
	${CC} package_finalize.c -o bin/package_finalize ${CFLAGS} ${LDFLAGS}
	${CC} make_self.c -o bin/make_self_sprx ${CFLAGS} ${LDFLAGS} -DSPRX
	${CC} make_self.c -o bin/make_self_npdrm ${CFLAGS} ${LDFLAGS} -DNPDRM
	${CC} make_self.c -o bin/make_self_nocrypt ${CFLAGS} ${LDFLAGS} -DNOCRYPT
	echo ... > bin/placeholder

clean:
	rm -rf bin/*

install:
	install -C -m 755 -v bin/* /usr/bin
