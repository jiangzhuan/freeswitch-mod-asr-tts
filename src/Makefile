CC=gcc
CFLAGS=-fPIC -Wall -I/usr/local/freeswitch/include/freeswitch
LDFLAGS=-shared -lfreeswitch -lm

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
	rm -f $(OBJS) $(TARGET)

.PHONY: all install clean