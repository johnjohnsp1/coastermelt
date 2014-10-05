LDFLAGS := -framework CoreFoundation -framework IOKit

BIN := flasher
OBJS := flasher.o

all: $(BIN)

flasher: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $<

%.o: %.cpp *.h
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	rm -f $(BIN) $(OBJS)

.PHONY: all clean
