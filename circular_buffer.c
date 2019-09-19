#include <string.h>

#include "circular_buffer.h"

int circular_buffer_init(struct circular_buffer *buf, uint8_t *buffer, size_t buffer_size, circular_notify notify, void *notify_data)
{
	if (buffer_size <= 0)
		return -1;
	buf->buffer = buffer;
	buf->size = buffer_size;;
	buf->head = 0;
	buf->tail = 0;
	buf->full = false;
	buf->notify = notify;
	buf->notify_data = notify_data;
	return 0;
}

int circular_buffer_write(struct circular_buffer *buf, const void *data, size_t data_len, bool allow_overwrite)
{
	if (buf->full)
		return 0;
	if (buf->head < buf->tail) {
		size_t avail = buf->tail - buf->head;
		if (avail < data_len)
			data_len = avail;
		memcpy(&buf->buffer[buf->head], data, data_len);
		buf->head = (buf->head + data_len) % buf->size;
		buf->full = (buf->head == buf->tail);

		return data_len;
	} else {
		size_t to_write = buf->size - buf->head;
		if (to_write > data_len)
			to_write = data_len;
		memcpy(&buf->buffer[buf->head], data, to_write);
		buf->head = (buf->head + to_write) % buf->size;
		if (to_write < data_len) {
			// buffer has wrapped, so write the next bit
			memcpy(buf->buffer, &data[to_write], data_len - to_write);
			buf->head = (buf->head + data_len - to_write) % buf->size;
			to_write += data_len - to_write;
		}
		buf->full = (buf->head == buf->tail);

		return to_write;
	}
}

size_t circular_buffer_read(struct circular_buffer *buf, void *data, size_t max_len)
{
	if (!max_len)
		return 0;
	buf->full = false;
	if (buf->head > buf->tail) {
		size_t to_read = buf->head - buf->tail;
		if (to_read > max_len)
			to_read = max_len;
		memcpy(data, &buf->buffer[buf->tail], to_read);
		buf->tail = (buf->tail + to_read) % buf->size;
		return to_read;
	} else {
		size_t to_read = buf->size - buf->tail;
		if (to_read > max_len)
			to_read = max_len;
		memcpy(data, &buf->buffer[buf->tail], to_read);
		buf->tail = (buf->tail + to_read) % buf->size;
		// TODO: wrap around
		return to_read;
	}
}

size_t circular_buffer_free_space(struct circular_buffer *buf)
{
	if (buf->full)
		return 0;
	if (buf->head < buf->tail)
		return buf->tail - buf->head;
	return buf->size - (buf->head - buf->tail);
}

size_t circular_buffer_used_space(struct circular_buffer *buf)
{
	if (buf->full)
		return buf->size;
	if (buf->head < buf->tail)
		return buf->size - (buf->head - buf->tail);
	return buf->head - buf->tail;
}

size_t circlular_buffer_maximum(struct circular_buffer *buf)
{
	return buf->size;
}

