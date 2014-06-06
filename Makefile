CC= clang
CFLAGS= -Wall -O0 -g
LIBS= -lpci

INCLUDES= -I.

SRCS= intelmetool.c \
	  me_status.c \
	  me.c \
	  mmap.c

OBJS = $(SRCS:.c=.o)

TARGET= intelmetool


.PHONY: depend clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	rm $(TARGET) *.o
