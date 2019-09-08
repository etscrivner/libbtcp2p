.PHONY=clean

CFLAGS=-Wall -Werror -I.
LDFLAGS=-lssl -lcrypto

# Pick one of:
#   linux
#   macos
#   bsd
#   win

UNAME=$(shell uname)
ifeq ($(UNAME),Darwin)
  OS=macos
else ifeq ($(UNAME),Linux)
  OS=linux
else ifeq ($(UNAME),FreeBSD)
  OS=bsd
else ifeq ($(UNAME),OpenBSD)
  OS=bsd
else ifneq (,$(findstring MINGW32_NT,$(UNAME)))
  OS=win
else
  $(error unknown os $(UNAME))
endif

# make DEBUG=1 for debugging
ifeq ($(DEBUG),1)
	CFLAGS+=-g
endif

OFILES=libbtcp2p/log.o \
	libbtcp2p/timer.o \
	libbtcp2p/checked_buffer.o \
	libbtcp2p/pack.o \
	libbtcp2p/vartypes.o \
	libbtcp2p/connection.o

btcp2p: btcp2p_example.c libbtcp2p.a
	$(CC) $(CFLAGS) -o btcp2p_example btcp2p_example.c -L. -lbtcp2p $(LDFLAGS)

libbtcp2p.a: $(OFILES)
	$(AR) rcs libbtcp2p.a $(OFILES)

libbtcp2p/log.o: libbtcp2p/log.c libbtcp2p/log.h
	$(CC) $(CFLAGS) -c -o libbtcp2p/log.o libbtcp2p/log.c $(LDFLAGS)

libbtcp2p/timer.o: libbtcp2p/timer.c libbtcp2p/timer.h
	$(CC) $(CFLAGS) -c -o libbtcp2p/timer.o libbtcp2p/timer.c $(LDFLAGS)

libbtcp2p/checked_buffer.o: libbtcp2p/checked_buffer.c libbtcp2p/checked_buffer.h
	$(CC) $(CFLAGS) -c -o libbtcp2p/checked_buffer.o libbtcp2p/checked_buffer.c $(LDFLAGS)

libbtcp2p/pack.o: libbtcp2p/pack.h libbtcp2p/pack.c
	$(CC) $(CFLAGS) -c -o libbtcp2p/pack.o libbtcp2p/pack.c $(LDFLAGS)

libbtcp2p/vartypes.o: libbtcp2p/vartypes.h libbtcp2p/vartypes.c
	$(CC) $(CFLAGS) -c -o libbtcp2p/vartypes.o libbtcp2p/vartypes.c $(LDFLAGS)

libbtcp2p/connection.o: libbtcp2p/connection.h libbtcp2p/connection.c
	$(CC) $(CFLAGS) -c -o libbtcp2p/connection.o libbtcp2p/connection.c $(LDFLAGS)

tests/test_checked_buffer: libbtcp2p.a tests/test_checked_buffer.c
	$(CC) $(CFLAGS) tests/test_checked_buffer.c -o tests/test_checked_buffer -L. -lbtcp2p

check: tests/test_checked_buffer
	@echo "[Unit Tests]"
	@tests/runner.sh tests/test_checked_buffer

clean:
	rm -rf *~
	rm -rf libbtcp2p/*.o
	rm -rf btcp2p
	rm -rf libbtcp2p.a
