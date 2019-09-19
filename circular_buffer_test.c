#include "acutest.h"
#include "circular_buffer.h"

void test_add_remove(void)
{
	uint8_t buffer[1024];
	struct circular_buffer cb;
	char tmp[3] = {};

	TEST_CHECK(circular_buffer_init(&cb, buffer, sizeof(buffer), NULL, NULL) >= 0);
	TEST_CHECK(circular_buffer_write(&cb, "foo", 3, false) == 3);
	TEST_CHECK(circular_buffer_read(&cb, tmp, 3) == 3);
	TEST_CHECK(memcmp(tmp, "foo", 3) == 0);
}

void test_full_empty(void)
{
	uint8_t buffer[2];
	struct circular_buffer cb;
	char tmp;

	TEST_CHECK(circular_buffer_init(&cb, buffer, sizeof(buffer), NULL, NULL) >= 0);
	TEST_CHECK(!circular_buffer_is_full(&cb));
	TEST_CHECK(circular_buffer_is_empty(&cb));
	TEST_CHECK(circular_buffer_write(&cb, "a", 1, false) == 1);
	TEST_CHECK(!circular_buffer_is_full(&cb));
	TEST_CHECK(circular_buffer_write(&cb, "b", 1, false) == 1);
	TEST_CHECK(circular_buffer_is_full(&cb));
	TEST_CHECK(circular_buffer_read(&cb, &tmp, 1) == 1);
	TEST_CHECK(!circular_buffer_is_full(&cb));
	TEST_CHECK(tmp == 'a');
	TEST_CHECK(circular_buffer_read(&cb, &tmp, 1) == 1);
	TEST_CHECK(tmp == 'b');
	TEST_CHECK(circular_buffer_is_empty(&cb));
}

TEST_LIST = {
	{"simple add remove", test_add_remove},
	{"full/empty", test_full_empty},
	{NULL, NULL},
};