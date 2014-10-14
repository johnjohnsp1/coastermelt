BIN := mtflash

OBJS := flasher.o
LIB := ../lib

CCFLAGS := -I$(LIB)
LDFLAGS := -framework CoreFoundation -framework IOKit

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $<

%.o: %.cpp $(LIB)/*.h
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	rm -f $(BIN) $(OBJS)

.PHONY: all clean
