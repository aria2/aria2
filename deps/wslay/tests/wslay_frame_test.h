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
#ifndef WSLAY_FRAME_TEST_H
#define WSLAY_FRAME_TEST_H

void test_wslay_frame_context_init(void);
void test_wslay_frame_recv(void);
void test_wslay_frame_recv_1byte(void);
void test_wslay_frame_recv_fragmented(void);
void test_wslay_frame_recv_interleaved_ctrl_frame(void);
void test_wslay_frame_recv_zero_payloadlen(void);
void test_wslay_frame_recv_too_large_payload(void);
void test_wslay_frame_recv_ctrl_frame_too_large_payload(void);
void test_wslay_frame_recv_minimum_ext_payload16(void);
void test_wslay_frame_recv_minimum_ext_payload64(void);
void test_wslay_frame_send(void);
void test_wslay_frame_send_fragmented(void);
void test_wslay_frame_send_interleaved_ctrl_frame(void);
void test_wslay_frame_send_1byte_masked(void);
void test_wslay_frame_send_zero_payloadlen(void);
void test_wslay_frame_send_too_large_payload(void);
void test_wslay_frame_send_ctrl_frame_too_large_payload(void);
void test_wslay_frame_write(void);
void test_wslay_frame_write_fragmented(void);
void test_wslay_frame_write_interleaved_ctrl_frame(void);
void test_wslay_frame_write_1byte_masked(void);
void test_wslay_frame_write_zero_payloadlen(void);
void test_wslay_frame_write_too_large_payload(void);
void test_wslay_frame_write_ctrl_frame_too_large_payload(void);

#endif /* WSLAY_FRAME_TEST_H */
