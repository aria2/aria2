/*
 * Wslay - The WebSocket Library
 *
 * Copyright (c) 2011, 2012 Tatsuhiro Tsujikawa
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "wslay_queue_test.h"

#include <CUnit/CUnit.h>

#include "wslay_queue.h"

void test_wslay_queue(void)
{
  int ints[] = { 1, 2, 3, 4, 5 };
  int i;
  struct wslay_queue *queue = wslay_queue_new();
  CU_ASSERT(wslay_queue_empty(queue));
  for(i = 0; i < 5; ++i) {
    wslay_queue_push(queue, &ints[i]);
    CU_ASSERT_EQUAL(ints[0], *(int*)(wslay_queue_top(queue)));
    CU_ASSERT(!wslay_queue_empty(queue));
  }
  for(i = 0; i < 5; ++i) {
    CU_ASSERT_EQUAL(ints[i], *(int*)(wslay_queue_top(queue)));
    wslay_queue_pop(queue);
  }
  CU_ASSERT(wslay_queue_empty(queue));

  for(i = 0; i < 5; ++i) {
    wslay_queue_push_front(queue, &ints[i]);
    CU_ASSERT_EQUAL(ints[i], *(int*)(wslay_queue_top(queue)));
    CU_ASSERT(!wslay_queue_empty(queue));
  }
  for(i = 4; i >= 0; --i) {
    CU_ASSERT_EQUAL(ints[i], *(int*)(wslay_queue_top(queue)));
    wslay_queue_pop(queue);
  }
  CU_ASSERT(wslay_queue_empty(queue));
  wslay_queue_free(queue);
}
