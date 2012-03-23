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
#ifndef WSLAY_EVENT_TEST_H
#define WSLAY_EVENT_TEST_H

void test_wslay_event_send_fragmented_msg(void);
void test_wslay_event_send_fragmented_msg_with_ctrl(void);
void test_wslay_event_send_ctrl_msg_first(void);
void test_wslay_event_queue_close(void);
void test_wslay_event_queue_close_without_code(void);
void test_wslay_event_recv_close_without_code(void);
void test_wslay_event_reply_close(void);
void test_wslay_event_no_more_msg(void);
void test_wslay_event_callback_failure(void);
void test_wslay_event_no_buffering(void);
void test_wslay_event_frame_too_big(void);
void test_wslay_event_message_too_big(void);

#endif /* WSLAY_EVENT_TEST_H */
