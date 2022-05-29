CFLAGS = -O3 -Wall
INCLUDE = -I curl-7.63.0/include
LIB = -L curl-7.63.0/lib/.libs/	
all: 3.o
	gcc ${CFLAGS} ${INCLUDE} ${LIB} 3.c -o 3 -lcurl -lm