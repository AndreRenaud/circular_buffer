#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

struct circular_buffer;

typedef void (*circular_notify)(struct circular_buffer *buf, void *user_data);

struct circular_buffer {
	uint8_t *buffer;
	size_t size;
	size_t head; /* Position of next write */
	size_t tail; /* Position of next read (== head if empty/full) */
	bool full;
	circular_notify notify;
	void *notify_data;
};

int circular_buffer_init(struct circular_buffer *buf, uint8_t *buffer, size_t buffer_size, circular_notify notify, void *notify_data);

int circular_buffer_write(struct circular_buffer *buf, const void *data, size_t data_len, bool allow_overwrite);
size_t circular_buffer_read(struct circular_buffer *buf, void *data, size_t max_len);
size_t circular_buffer_free_space(struct circular_buffer *buf);
size_t circular_buffer_used_space(struct circular_buffer *buf);
size_t circlular_buffer_maximum(struct circular_buffer *buf);

static inline bool circular_buffer_is_full(struct circular_buffer *buf)
{
	return circular_buffer_free_space(buf) == 0;
}

static inline bool circular_buffer_is_empty(struct circular_buffer *buf)
{
	return circular_buffer_used_space(buf) == 0;
}

#endif