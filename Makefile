CC = gcc
CFLAGS = -Wall -I./swisseph-master
LDFLAGS = -L./swisseph-master -lswe -lm

TARGET = placidus_chart
SRCS = placidus_chart.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean libswe

all: libswe $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

libswe:
	cd swisseph-master && $(MAKE) -f Makefile.win

clean:
	del $(TARGET) *.o
	cd swisseph-master && $(MAKE) -f Makefile.win clean 