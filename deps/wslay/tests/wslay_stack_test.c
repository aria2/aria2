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
#include "wslay_stack_test.h"

#include <CUnit/CUnit.h>

#include "wslay_stack.h"

void test_wslay_stack()
{
  int ints[] = { 1, 2, 3, 4, 5 };
  int i;
  struct wslay_stack *stack = wslay_stack_new();
  CU_ASSERT(wslay_stack_empty(stack));
  for(i = 0; i < 5; ++i) {
    wslay_stack_push(stack, &ints[i]);
    CU_ASSERT_EQUAL(ints[i], *(int*)(wslay_stack_top(stack)));
    CU_ASSERT(!wslay_stack_empty(stack));
  }
  for(i = 4; i >= 0; --i) {
    CU_ASSERT_EQUAL(ints[i], *(int*)(wslay_stack_top(stack)));
    wslay_stack_pop(stack);
  }
  CU_ASSERT(wslay_stack_empty(stack));
  wslay_stack_free(stack);
}
