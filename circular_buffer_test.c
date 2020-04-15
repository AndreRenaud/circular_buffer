#include <pthread.h>
#include <stdlib.h>

#include "acutest.h"
#include "circular_buffer.h"

static void test_add_remove(void)
{
	uint8_t buffer[1024];
	struct circular_buffer cb;
	char tmp[3] = {};

	TEST_ASSERT(circular_buffer_init(&cb, buffer, sizeof(buffer)) >= 0);
	TEST_ASSERT(circular_buffer_write(&cb, "foo", 3) == 3);
	TEST_ASSERT(circular_buffer_read(&cb, tmp, 3) == 3);
	TEST_ASSERT(memcmp(tmp, "foo", 3) == 0);
}

static void test_full_empty(void)
{
	uint8_t buffer[2];
	struct circular_buffer cb;
	char tmp;

	TEST_ASSERT(circular_buffer_init(&cb, buffer, sizeof(buffer)) >= 0);
	TEST_ASSERT(!circular_buffer_is_full(&cb));
	TEST_ASSERT(circular_buffer_is_empty(&cb));
	TEST_ASSERT(circular_buffer_write(&cb, "a", 1) == 1);
	TEST_ASSERT(!circular_buffer_is_full(&cb));
	TEST_ASSERT(circular_buffer_write(&cb, "b", 1) == 1);
	TEST_ASSERT(circular_buffer_is_full(&cb));
	TEST_ASSERT(circular_buffer_read(&cb, &tmp, 1) == 1);
	TEST_ASSERT(!circular_buffer_is_full(&cb));
	TEST_ASSERT(tmp == 'a');
	TEST_ASSERT(circular_buffer_read(&cb, &tmp, 1) == 1);
	TEST_ASSERT(tmp == 'b');
	TEST_ASSERT(circular_buffer_is_empty(&cb));
}

static volatile bool test_running = true;
static void *cb_reader(void *data)
{
	struct circular_buffer *cb = data;

	while (test_running) {
		uint8_t buffer[128];
		size_t r;
		r = circular_buffer_read(cb, buffer, sizeof(buffer));
		if (!r) {
			usleep(1);
			continue;
		}
		TEST_ASSERT(r == sizeof(buffer));
		for (unsigned int i = 0; i < sizeof(buffer); i++) {
			if (!TEST_CHECK(buffer[i] == (uint8_t)(buffer[0] + i))) {
				TEST_MSG("%d: %d != %d (0=%d)\n", i, buffer[i], buffer[0] + i, buffer[0]);
				TEST_ASSERT(0);
			}
		}
		usleep(1);
	}

	return NULL;
}

static void *cb_writer(void *data)
{
	struct circular_buffer *cb = data;

	while (test_running) {
		int start = rand();
		char buffer[128];
		for (unsigned int i = 0; i < sizeof(buffer); i++) {
			buffer[i] = start + i;
		}
		size_t len = circular_buffer_write(cb, buffer, sizeof(buffer));
		TEST_ASSERT(len == 0 || len == sizeof(buffer));
		usleep(1);
	}
	return NULL;
}

static void test_threaded(void)
{
	struct circular_buffer cb;
	uint8_t buffer[1024];
	pthread_t r_threads[10], w_threads[10];
	int nthreads = 10;

	test_running = true;
	TEST_ASSERT(circular_buffer_init(&cb, buffer, sizeof(buffer)) >= 0);
	for (int i = 0; i < nthreads; i++) {
		pthread_create(&r_threads[i], NULL, cb_reader, &cb);
		pthread_create(&w_threads[i], NULL, cb_writer, &cb);
	}

	sleep(1);
	test_running = false;
	for (int i = 0; i < nthreads; i++) {
		pthread_join(w_threads[i], NULL);
		pthread_join(r_threads[i], NULL);
	}
	printf("%zu/%zu bytes read/written\n", (size_t)cb.bytes_read, (size_t)cb.bytes_written);
}

static void *cb_reader_var(void *data)
{
	struct circular_buffer *cb = data;
	int pos = 0;

	while (test_running) {
		uint8_t buffer[128];
		size_t r;
		r = circular_buffer_read(cb, buffer, rand() % sizeof(buffer));
		if (!r) {
			usleep(1);
			continue;
		}
		for (unsigned int i = 0; i < r; i++) {
			if (!TEST_CHECK(buffer[i] == (uint8_t)(pos + i))) {
				TEST_MSG("%d: %d != %d (pos=%d)\n", i, buffer[i], pos + i, pos);
				TEST_ASSERT(0);
			}
		}
		pos += r;
		usleep(1);
	}

	return NULL;
}

static void *cb_writer_var(void *data)
{
	struct circular_buffer *cb = data;
	int pos = 0;

	while (test_running) {
		char buffer[128];
		for (unsigned int i = 0; i < sizeof(buffer); i++) {
			buffer[i] = pos + i;
		}
		size_t len = circular_buffer_write(cb, buffer, rand() % sizeof(buffer));
		pos += len;
		usleep(1);
	}
	return NULL;
}

static void test_threaded_var(void)
{
	struct circular_buffer cb;
	uint8_t buffer[9493]; // weird number
	pthread_t r_thread, w_thread;

	test_running = true;
	TEST_ASSERT(circular_buffer_init(&cb, buffer, sizeof(buffer)) >= 0);
	pthread_create(&r_thread, NULL, cb_reader_var, &cb);
	pthread_create(&w_thread, NULL, cb_writer_var, &cb);

	sleep(5);
	test_running = false;
	pthread_join(w_thread, NULL);
	pthread_join(r_thread, NULL);
	printf("%zu/%zu bytes read/written\n", (size_t)cb.bytes_read, (size_t)cb.bytes_written);
}

TEST_LIST = {
	{"simple add remove", test_add_remove},
	{"full/empty", test_full_empty},
	{"threaded", test_threaded},
	{"threaded variable length", test_threaded_var},
	{NULL, NULL},
};