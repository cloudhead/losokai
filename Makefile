CC      := clang
CFLAGS  := -msse4.1 -Wall -Wno-missing-braces -fstrict-aliasing -pedantic -std=c11 -O0 -g
LDFLAGS := -DGLEW_STATIC -lGL -lGLEW -lglfw -lm
INCS    := -I../
CSRC    := $(wildcard *.c)
SSRC    := $(wildcard *.s)
OBJ     := $(CSRC:.c=.o) $(SSRC:.s=.o)
TARGET  := lourland
TOOLS   := tools/lconvert

all: $(TARGET) $(TOOLS)

$(TOOLS): $(wildcard tools/*.c)
	$(MAKE) -C tools

%.o: %.c
	$(CC) -c $(CFLAGS) $(INCS) -o $@ $<

%.o: %.s
	as -o $@ $<

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $(TARGET)

%.d: %.c
	$(CC) $(CFLAGS) $(INCS) -MM -MG -MT "$*.o $*.d" $*.c >$@

-include $(OBJ:.o=.d)

clean:
	rm -f $(OBJ) *.d
	[ -f $(TARGET) ] && rm $(TARGET)
