BIN := mtflash

OBJS := flasher.o

LDFLAGS := -framework CoreFoundation -framework IOKit

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $<

%.o: %.cpp *.h
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	rm -f $(BIN) $(OBJS)

.PHONY: all clean
