CFLAGS=-g -Wall -pipe -Wextra

default: circular_buffer_test

test: circular_buffer_test
	./circular_buffer_test

circular_buffer_test: circular_buffer.o circular_buffer_test.o
	$(CC) -o $@ circular_buffer.o circular_buffer_test.o $(LFLAGS)

%.o: %.c circular_buffer.h
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o circular_buffer_test