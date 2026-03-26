CC=gcc
FS_SRC?=/usr/src/freeswitch-src
FS_INCLUDE=$(FS_SRC)/src/include
APR_INCLUDE=$(FS_SRC)/libs/apr/include
APU_INCLUDE=$(FS_SRC)/libs/apr-util/include
TELETONE_INCLUDE=$(FS_SRC)/libs/libteletone/src

CFLAGS=-fPIC -Wall -I$(FS_INCLUDE) -I$(APR_INCLUDE) -I$(APU_INCLUDE) -I$(TELETONE_INCLUDE)
LDFLAGS=-shared -lm -lcurl

TARGET=mod_asr_tts.so
SRCS=mod_asr_tts.c \
     core/config.c \
     core/buffer.c \
     core/resample.c \
     core/media_bug.c \
     core/worker.c \
     core/vad.c \
     core/event.c \
     asr/asr_interface.c \
     asr/providers/aliyun_asr.c
OBJS=$(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

install: $(TARGET)
	cp $(TARGET) /usr/local/freeswitch/mod/

clean:
	rm -f $(OBJS) $(TARGET) 2>/dev/null || true

.PHONY: all install clean