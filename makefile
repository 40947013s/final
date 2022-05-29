CFLAGS = -O3 -Wall
INCLUDE = -I curl-7.63.0/include
LIB = -L curl-7.63.0/lib/.libs/	

all: 1.o 2.o 3.o
    gcc 1.o -o 1
    gcc 2.o -o 2
    gcc ${CFLAGS} ${INCLUDE} ${LIB} 3.c -o 3 -lcurl -lm

1: 1.c
    gcc -c 1.c -o 1.o
    gcc -shared 1.o -o lib1.so

2: 2.c
    gcc -c 2.c -o 2.o
    gcc -shared 2.o -o lib2.so3.o
	gcc ${CFLAGS} ${INCLUDE} ${LIB} 3.c -o 3 -lcurl -lm