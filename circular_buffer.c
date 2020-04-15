#include <string.h>
#include <assert.h>

#if defined(__APPLE__) || defined(__linux__)
/* For posix-pthread compliant systems */
#include <pthread.h>
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#define disable_irq() pthread_mutex_lock(&mutex)
#define enable_irq() pthread_mutex_unlock(&mutex)
#else
/* For other systems, ie: embedded, these macros should be defined as appropriate */
#define disable_irq()
#define enable_irq()
#endif

#include "circular_buffer.h"

int circular_buffer_init(struct circular_buffer *buf, uint8_t *buffer, size_t buffer_size)
{
	if (buffer_size <= 0)
		return -1;
	buf->buffer = buffer;
	buf->size = buffer_size;
	buf->head = 0;
	buf->tail = 0;
	buf->full = false;
	buf->bytes_written = buf->bytes_read = 0;
	return 0;
}

size_t circular_buffer_write(struct circular_buffer *buf, const void *data, size_t data_len)
{
	size_t to_write;
	if (!data_len || buf->full)
		return 0;
	disable_irq();
	if (buf->head < buf->tail) {
		size_t avail = buf->tail - buf->head;
		if (avail > data_len)
			to_write = data_len;
		else
			to_write = avail;
		memcpy(&buf->buffer[buf->head], data, to_write);
		buf->head = (buf->head + to_write) % buf->size;
	} else {
		size_t avail = buf->size - buf->head;
		if (avail > data_len)
			to_write = data_len;
		else
			to_write = avail;

		memcpy(&buf->buffer[buf->head], data, to_write);
		buf->head = (buf->head + to_write) % buf->size;
		if (to_write < data_len) {
			// buffer has wrapped, so write the next bit
			const uint8_t *d8 = data;
			assert(buf->head == 0);
			avail = buf->tail;
			size_t last_write = data_len - to_write;
			if (avail < last_write)
				last_write = avail;
			memcpy(buf->buffer, &d8[to_write], last_write);
			buf->head = (buf->head + last_write) % buf->size;
			to_write += last_write;
		}
	}
	buf->full = to_write && (buf->head == buf->tail);
	buf->bytes_written += to_write;
	enable_irq();
	return to_write;
}

size_t circular_buffer_read(struct circular_buffer *buf, void *data, size_t max_len)
{
	size_t have_read;
	if (!max_len)
		return 0;
	disable_irq();
	if (!buf->full && buf->head == buf->tail) {
		enable_irq();
		return 0;
	}
	if (buf->head > buf->tail) {
		size_t to_read = buf->head - buf->tail;
		if (to_read < max_len)
			have_read = to_read;
		else
			have_read = max_len;
		memcpy(data, &buf->buffer[buf->tail], have_read);
		buf->tail = (buf->tail + have_read) % buf->size;
	} else {
		size_t to_read = buf->size - buf->tail;
		if (to_read > max_len)
			to_read = max_len;
		memcpy(data, &buf->buffer[buf->tail], to_read);
		buf->tail = (buf->tail + to_read) % buf->size;
		have_read = to_read;
		if (to_read < max_len) {
			assert(buf->tail == 0);
			to_read = max_len - to_read;
			if (to_read > buf->head)
				to_read = buf->head;
			memcpy(&data[have_read], buf->buffer, to_read);
			buf->tail = (buf->tail + to_read) % buf->size;
			have_read += to_read;
		}
	}
	buf->full = buf->full && (have_read == 0);
	buf->bytes_read += have_read;
	enable_irq();
	return have_read;
}

size_t circular_buffer_free_space(struct circular_buffer *buf)
{
	size_t space;
	disable_irq();
	if (buf->full)
		space = 0;
	else if (buf->head < buf->tail)
		space = buf->tail - buf->head;
	else
		space = buf->size - (buf->head - buf->tail);
	enable_irq();
	return space;
}

size_t circular_buffer_used_space(struct circular_buffer *buf)
{
	size_t space;
	if (buf->full)
		space = buf->size;
	else if (buf->head < buf->tail)
		space = buf->size - (buf->head - buf->tail);
	else
		space = buf->head - buf->tail;
	return space;
}

size_t circlular_buffer_maximum(struct circular_buffer *buf)
{
	return buf->size;
}

