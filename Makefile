OBJS=main.o tape.o image.o

all: image-tape

image-tape: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(OBJS): defs.h

clean:
	rm -f image-tape $(OBJS)
