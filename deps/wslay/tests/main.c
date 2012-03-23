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
#include <stdio.h>
#include <string.h>
#include <CUnit/Basic.h>
/* include test cases' include files here */
#include "wslay_frame_test.h"
#include "wslay_event_test.h"
#include "wslay_queue_test.h"

static int init_suite1(void)
{
  return 0;
}

static int clean_suite1(void)
{
  return 0;
}


int main(void)
{
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("libwslay_TestSuite", init_suite1, clean_suite1);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if(!CU_add_test(pSuite, "wslay_frame_context_init",
                   test_wslay_frame_context_init) ||
      !CU_add_test(pSuite, "wslay_frame_recv", test_wslay_frame_recv) ||
      !CU_add_test(pSuite, "wslay_frame_recv_1byte",
                   test_wslay_frame_recv_1byte) ||
      !CU_add_test(pSuite, "wslay_frame_recv_fragmented",
                   test_wslay_frame_recv_fragmented) ||
      !CU_add_test(pSuite, "wslay_frame_recv_interleaved_ctrl_frame",
                   test_wslay_frame_recv_interleaved_ctrl_frame) ||
      !CU_add_test(pSuite, "wslay_frame_recv_zero_payloadlen",
                   test_wslay_frame_recv_zero_payloadlen) ||
      !CU_add_test(pSuite, "wslay_frame_recv_too_large_payload",
                   test_wslay_frame_recv_too_large_payload) ||
      !CU_add_test(pSuite, "wslay_frame_recv_ctrl_too_large_payload",
                   test_wslay_frame_recv_ctrl_frame_too_large_payload) ||
      !CU_add_test(pSuite, "wslay_frame_recv_minimum_ext_payload16",
                   test_wslay_frame_recv_minimum_ext_payload16) ||
      !CU_add_test(pSuite, "wslay_frame_recv_minimum_ext_payload64",
                   test_wslay_frame_recv_minimum_ext_payload64) ||
      !CU_add_test(pSuite, "wslay_frame_send", test_wslay_frame_send) ||
      !CU_add_test(pSuite, "wslay_frame_send_fragmented",
                   test_wslay_frame_send_fragmented) ||
      !CU_add_test(pSuite, "wslay_frame_send_interleaved_ctrl_frame",
                   test_wslay_frame_send_interleaved_ctrl_frame) ||
      !CU_add_test(pSuite, "wslay_frame_send_1byte_masked",
                   test_wslay_frame_send_1byte_masked) ||
      !CU_add_test(pSuite, "wslay_frame_send_zero_payloadlen",
                   test_wslay_frame_send_zero_payloadlen) ||
      !CU_add_test(pSuite, "wslay_frame_send_too_large_payload",
                   test_wslay_frame_send_too_large_payload) ||
      !CU_add_test(pSuite, "wslay_frame_send_ctrl_frame_too_large_payload",
                   test_wslay_frame_send_ctrl_frame_too_large_payload) ||
      !CU_add_test(pSuite, "wslay_event_send_fragmented_msg",
                   test_wslay_event_send_fragmented_msg) ||
      !CU_add_test(pSuite, "wslay_event_send_fragmented_msg_with_ctrl",
                   test_wslay_event_send_fragmented_msg_with_ctrl) ||
      !CU_add_test(pSuite, "wslay_event_send_ctrl_msg_first",
                   test_wslay_event_send_ctrl_msg_first) ||
      !CU_add_test(pSuite, "wslay_event_queue_close",
                   test_wslay_event_queue_close) ||
      !CU_add_test(pSuite, "wslay_event_queue_close_without_code",
                   test_wslay_event_queue_close_without_code) ||
      !CU_add_test(pSuite, "wslay_event_recv_close_without_code",
                   test_wslay_event_recv_close_without_code) ||
      !CU_add_test(pSuite, "wslay_event_reply_close",
                   test_wslay_event_reply_close) ||
      !CU_add_test(pSuite, "wslay_event_no_more_msg",
                   test_wslay_event_no_more_msg) ||
      !CU_add_test(pSuite, "wslay_event_callback_failure",
                   test_wslay_event_callback_failure) ||
      !CU_add_test(pSuite, "wslay_event_no_buffering",
                   test_wslay_event_no_buffering) ||
      !CU_add_test(pSuite, "wslay_event_frame_too_big",
                   test_wslay_event_frame_too_big) ||
      !CU_add_test(pSuite, "wslay_event_message_too_big",
                   test_wslay_event_message_too_big) ||
      !CU_add_test(pSuite, "wslay_queue", test_wslay_queue)) {
     CU_cleanup_registry();
     return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}
