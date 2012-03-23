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
#include "wslay_frame_test.h"

#include <assert.h>

#include <CUnit/CUnit.h>

#include "wslay_frame.h"

void test_wslay_frame_context_init(void)
{
  wslay_frame_context_ptr ctx;
  struct wslay_frame_callbacks callbacks;
  int user_data;
  CU_ASSERT_FALSE(wslay_frame_context_init(&ctx, &callbacks, &user_data));

  wslay_frame_context_free(ctx);
}

struct scripted_data_feed {
  uint8_t data[8192];
  uint8_t* datamark;
  uint8_t* datalimit;
  size_t feedseq[8192];
  size_t seqidx;
};

static void scripted_data_feed_init(struct scripted_data_feed *df,
                                    uint8_t *data, size_t data_length)
{
  memset(df, 0, sizeof(struct scripted_data_feed));
  memcpy(df->data, data, data_length);
  df->datamark = df->data;
  df->datalimit = df->data+data_length;
  df->feedseq[0] = data_length;
}

static ssize_t scripted_recv_callback(uint8_t* data, size_t len, int flags,
                                      void *user_data)
{
  struct scripted_data_feed *df = (struct scripted_data_feed*)user_data;
  size_t wlen = df->feedseq[df->seqidx] > len ? len : df->feedseq[df->seqidx];
  memcpy(data, df->datamark, wlen);
  df->datamark += wlen;
  if(wlen <= len) {
    ++df->seqidx;
  } else {
    df->feedseq[df->seqidx] -= wlen;
  }
  return wlen;
}

static ssize_t scripted_send_callback(const uint8_t* data, size_t len,
                                      int flags, void *user_data)
{
  struct scripted_data_feed *df = (struct scripted_data_feed*)user_data;
  size_t wlen = df->feedseq[df->seqidx] > len ? len : df->feedseq[df->seqidx];
  memcpy(df->datamark, data, wlen);
  df->datamark += wlen;
  if(wlen <= len) {
    ++df->seqidx;
  } else {
    df->feedseq[df->seqidx] -= wlen;
  }
  return wlen;
}

void test_wslay_frame_recv(void)
{
  wslay_frame_context_ptr ctx;
  struct wslay_frame_callbacks callbacks = { NULL,
                                             scripted_recv_callback,
                                             NULL };
  struct scripted_data_feed df;
  struct wslay_frame_iocb iocb;
  /* Masked text frame containing "Hello" */
  uint8_t msg[] = { 0x81u, 0x85u, 0x37u, 0xfau, 0x21u, 0x3du, 0x7fu, 0x9fu,
                    0x4du, 0x51u, 0x58u };
  scripted_data_feed_init(&df, msg, sizeof(msg));
  wslay_frame_context_init(&ctx, &callbacks, &df);

  CU_ASSERT(5 == wslay_frame_recv(ctx, &iocb));
  CU_ASSERT_EQUAL(1, iocb.fin);
  CU_ASSERT_EQUAL(0, iocb.rsv);
  CU_ASSERT_EQUAL(0x1, iocb.opcode);
  CU_ASSERT_EQUAL(5, iocb.payload_length);
  CU_ASSERT_EQUAL(1, iocb.mask);
  CU_ASSERT_EQUAL(5, iocb.data_length);
  CU_ASSERT(memcmp("Hello", iocb.data, iocb.data_length) == 0);

  wslay_frame_context_free(ctx);
}

void test_wslay_frame_recv_1byte(void)
{
  wslay_frame_context_ptr ctx;
  struct wslay_frame_callbacks callbacks = { NULL,
                                             scripted_recv_callback,
                                             NULL };
  struct scripted_data_feed df;
  struct wslay_frame_iocb iocb;
  size_t i;
  /* Masked text frame containing "Hello" */
  uint8_t msg[] = { 0x81u, 0x85u, 0x37u, 0xfau, 0x21u, 0x3du, 0x7fu, 0x9fu,
                    0x4du, 0x51u, 0x58u };
  scripted_data_feed_init(&df, msg, sizeof(msg));
  for(i = 0; i < sizeof(msg); ++i) {
    df.feedseq[i] = 1;
  }
  wslay_frame_context_init(&ctx, &callbacks, &df);

  for(i = 0; i < 4; ++i) {
    CU_ASSERT(WSLAY_ERR_WANT_READ == wslay_frame_recv(ctx, &iocb));
  }
  for(i = 0; i < 5; ++i) {
    CU_ASSERT(1 == wslay_frame_recv(ctx, &iocb));
    CU_ASSERT_EQUAL(1, iocb.fin);
    CU_ASSERT_EQUAL(0, iocb.rsv);
    CU_ASSERT_EQUAL(0x1, iocb.opcode);
    CU_ASSERT_EQUAL(5, iocb.payload_length);
    CU_ASSERT_EQUAL(1, iocb.mask);
    CU_ASSERT_EQUAL(1, iocb.data_length);
    CU_ASSERT_EQUAL(msg[6+i]^msg[2+i%4], iocb.data[0]);
  }
  CU_ASSERT(WSLAY_ERR_WANT_READ == wslay_frame_recv(ctx, &iocb));

  wslay_frame_context_free(ctx);
}

void test_wslay_frame_recv_fragmented(void)
{
  wslay_frame_context_ptr ctx;
  struct wslay_frame_callbacks callbacks = { NULL,
                                             scripted_recv_callback,
                                             NULL };
  struct scripted_data_feed df;
  struct wslay_frame_iocb iocb;
  /* Unmasked message */
  uint8_t msg[] = { 0x01, 0x03, 0x48, 0x65, 0x6c, /* "Hel" */
                    0x80, 0x02, 0x6c, 0x6f }; /* "lo" */
  scripted_data_feed_init(&df, msg, sizeof(msg));
  df.feedseq[0] = 5;
  df.feedseq[1] = 4;
  wslay_frame_context_init(&ctx, &callbacks, &df);

  CU_ASSERT(3 == wslay_frame_recv(ctx, &iocb));
  CU_ASSERT_EQUAL(0, iocb.fin);
  CU_ASSERT_EQUAL(0, iocb.rsv);
  CU_ASSERT_EQUAL(WSLAY_TEXT_FRAME, iocb.opcode);
  CU_ASSERT_EQUAL(3, iocb.payload_length);
  CU_ASSERT_EQUAL(0, iocb.mask);
  CU_ASSERT_EQUAL(3, iocb.data_length);
  CU_ASSERT(memcmp("Hel", iocb.data, iocb.data_length) == 0);

  CU_ASSERT(2 == wslay_frame_recv(ctx, &iocb));
  CU_ASSERT_EQUAL(1, iocb.fin);
  CU_ASSERT_EQUAL(0, iocb.rsv);
  CU_ASSERT_EQUAL(WSLAY_CONTINUATION_FRAME, iocb.opcode);
  CU_ASSERT_EQUAL(2, iocb.payload_length);
  CU_ASSERT_EQUAL(0, iocb.mask);
  CU_ASSERT_EQUAL(2, iocb.data_length);
  CU_ASSERT(memcmp("lo", iocb.data, iocb.data_length) == 0);

  wslay_frame_context_free(ctx);
}

void test_wslay_frame_recv_interleaved_ctrl_frame(void)
{
  wslay_frame_context_ptr ctx;
  struct wslay_frame_callbacks callbacks = { NULL,
                                             scripted_recv_callback,
                                             NULL };
  struct scripted_data_feed df;
  struct wslay_frame_iocb iocb;
  /* Unmasked message */
  uint8_t msg[] = { 0x01, 0x03, 0x48, 0x65, 0x6c, /* "Hel" */
                    /* ping with "Hello" */
                    0x89, 0x05, 0x48, 0x65, 0x6c, 0x6c, 0x6f,
                    0x80, 0x02, 0x6c, 0x6f }; /* "lo" */
  scripted_data_feed_init(&df, msg, sizeof(msg));
  df.feedseq[0] = 5;
  df.feedseq[1] = 7,
    df.feedseq[2] = 4;
  wslay_frame_context_init(&ctx, &callbacks, &df);

  CU_ASSERT(3 == wslay_frame_recv(ctx, &iocb));
  CU_ASSERT_EQUAL(0, iocb.fin);
  CU_ASSERT_EQUAL(0, iocb.rsv);
  CU_ASSERT_EQUAL(WSLAY_TEXT_FRAME, iocb.opcode);
  CU_ASSERT_EQUAL(3, iocb.payload_length);
  CU_ASSERT_EQUAL(0, iocb.mask);
  CU_ASSERT_EQUAL(3, iocb.data_length);
  CU_ASSERT(memcmp("Hel", iocb.data, iocb.data_length) == 0);

  CU_ASSERT(5 == wslay_frame_recv(ctx, &iocb));
  CU_ASSERT_EQUAL(1, iocb.fin);
  CU_ASSERT_EQUAL(0, iocb.rsv);
  CU_ASSERT_EQUAL(WSLAY_PING, iocb.opcode);
  CU_ASSERT_EQUAL(5, iocb.payload_length);
  CU_ASSERT_EQUAL(0, iocb.mask);
  CU_ASSERT_EQUAL(5, iocb.data_length);
  CU_ASSERT(memcmp("Hello", iocb.data, iocb.data_length) == 0);

  CU_ASSERT(2 == wslay_frame_recv(ctx, &iocb));
  CU_ASSERT_EQUAL(1, iocb.fin);
  CU_ASSERT_EQUAL(0, iocb.rsv);
  CU_ASSERT_EQUAL(WSLAY_CONTINUATION_FRAME, iocb.opcode);
  CU_ASSERT_EQUAL(2, iocb.payload_length);
  CU_ASSERT_EQUAL(0, iocb.mask);
  CU_ASSERT_EQUAL(2, iocb.data_length);
  CU_ASSERT(memcmp("lo", iocb.data, iocb.data_length) == 0);

  wslay_frame_context_free(ctx);
}

void test_wslay_frame_recv_zero_payloadlen(void)
{
  wslay_frame_context_ptr ctx;
  struct wslay_frame_callbacks callbacks = { NULL,
                                             scripted_recv_callback,
                                             NULL };
  struct scripted_data_feed df;
  struct wslay_frame_iocb iocb;
  /* Unmasked message */
  uint8_t msg[] = { 0x81, 0x00 }; /* "" */
  scripted_data_feed_init(&df, msg, sizeof(msg));
  df.feedseq[0] = 2;
  wslay_frame_context_init(&ctx, &callbacks, &df);

  CU_ASSERT(0 == wslay_frame_recv(ctx, &iocb));
  CU_ASSERT_EQUAL(1, iocb.fin);
  CU_ASSERT_EQUAL(0, iocb.rsv);
  CU_ASSERT_EQUAL(WSLAY_TEXT_FRAME, iocb.opcode);
  CU_ASSERT_EQUAL(0, iocb.payload_length);
  CU_ASSERT_EQUAL(0, iocb.mask);
  CU_ASSERT_EQUAL(0, iocb.data_length);

  wslay_frame_context_free(ctx);
}

void test_wslay_frame_recv_too_large_payload(void)
{
  wslay_frame_context_ptr ctx;
  struct wslay_frame_callbacks callbacks = { NULL,
                                             scripted_recv_callback,
                                             NULL };
  struct scripted_data_feed df;
  struct wslay_frame_iocb iocb;
  uint8_t msg[] = { 0x81, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  scripted_data_feed_init(&df, msg, sizeof(msg));
  df.feedseq[0] = sizeof(msg);
  wslay_frame_context_init(&ctx, &callbacks, &df);
  CU_ASSERT_EQUAL(WSLAY_ERR_PROTO, wslay_frame_recv(ctx, &iocb));

  wslay_frame_context_free(ctx);
}

void test_wslay_frame_recv_ctrl_frame_too_large_payload(void)
{
  wslay_frame_context_ptr ctx;
  struct wslay_frame_callbacks callbacks = { NULL,
                                             scripted_recv_callback,
                                             NULL };
  struct scripted_data_feed df;
  struct wslay_frame_iocb iocb;
  uint8_t msg[] = { 0x88, 0x7e };
  scripted_data_feed_init(&df, msg, sizeof(msg));
  df.feedseq[0] = sizeof(msg);
  wslay_frame_context_init(&ctx, &callbacks, &df);
  CU_ASSERT_EQUAL(WSLAY_ERR_PROTO, wslay_frame_recv(ctx, &iocb));

  wslay_frame_context_free(ctx);
}

void test_wslay_frame_recv_minimum_ext_payload16(void)
{
  wslay_frame_context_ptr ctx;
  struct wslay_frame_callbacks callbacks = { NULL,
                                             scripted_recv_callback,
                                             NULL };
  struct scripted_data_feed df;
  struct wslay_frame_iocb iocb;
  uint8_t msg[] = { 0x81, 0x7e, 0x00, 0x7d };
  scripted_data_feed_init(&df, msg, sizeof(msg));
  df.feedseq[0] = sizeof(msg);
  wslay_frame_context_init(&ctx, &callbacks, &df);
  CU_ASSERT_EQUAL(WSLAY_ERR_PROTO, wslay_frame_recv(ctx, &iocb));

  wslay_frame_context_free(ctx);
}

void test_wslay_frame_recv_minimum_ext_payload64(void)
{
  wslay_frame_context_ptr ctx;
  struct wslay_frame_callbacks callbacks = { NULL,
                                             scripted_recv_callback,
                                             NULL };
  struct scripted_data_feed df;
  struct wslay_frame_iocb iocb;
  uint8_t msg[] = { 0x81, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff};
  scripted_data_feed_init(&df, msg, sizeof(msg));
  df.feedseq[0] = sizeof(msg);
  wslay_frame_context_init(&ctx, &callbacks, &df);
  CU_ASSERT_EQUAL(WSLAY_ERR_PROTO, wslay_frame_recv(ctx, &iocb));

  wslay_frame_context_free(ctx);
}

struct accumulator {
  uint8_t buf[4096];
  size_t length;
};

static ssize_t accumulator_send_callback(const uint8_t *buf, size_t len,
                                         int flags, void* user_data)
{
  struct accumulator *acc = (struct accumulator*)user_data;
  assert(acc->length+len < sizeof(acc->buf));
  memcpy(acc->buf+acc->length, buf, len);
  acc->length += len;
  return len;
}

static int static_genmask_callback(uint8_t *buf, size_t len,
                                   void* user_data)
{
  static const uint8_t makskey[] = { 0x37u, 0xfau, 0x21u, 0x3du };
  memcpy(buf, makskey, 4);
  return 0;
}

void test_wslay_frame_send(void)
{
  wslay_frame_context_ptr ctx;
  struct wslay_frame_callbacks callbacks = { accumulator_send_callback,
                                             NULL,
                                             static_genmask_callback };
  struct accumulator acc;
  struct wslay_frame_iocb iocb;
  /* Masked text frame containing "Hello" */
  uint8_t msg[] = { 0x81u, 0x85u, 0x37u, 0xfau, 0x21u, 0x3du, 0x7fu, 0x9fu,
                    0x4du, 0x51u, 0x58u };
  wslay_frame_context_init(&ctx, &callbacks, &acc);
  memset(&iocb, 0, sizeof(iocb));
  acc.length = 0;
  iocb.fin = 1;
  iocb.opcode = WSLAY_TEXT_FRAME;
  iocb.mask = 1;
  iocb.payload_length = 5;
  iocb.data = (const uint8_t*)"Hello";
  iocb.data_length = 5;
  CU_ASSERT(5 == wslay_frame_send(ctx, &iocb));
  CU_ASSERT_EQUAL(sizeof(msg), acc.length);
  CU_ASSERT(memcmp(msg, acc.buf, sizeof(msg)) == 0);

  wslay_frame_context_free(ctx);
}

void test_wslay_frame_send_fragmented(void)
{
  wslay_frame_context_ptr ctx;
  struct wslay_frame_callbacks callbacks = { accumulator_send_callback,
                                             NULL,
                                             static_genmask_callback };
  struct accumulator acc;
  struct wslay_frame_iocb iocb;
  /* Unmasked message */
  uint8_t msg1[] = { 0x01, 0x03, 0x48, 0x65, 0x6c }; /* "Hel" */
  uint8_t msg2[] = { 0x80, 0x02, 0x6c, 0x6f }; /* "lo" */
  wslay_frame_context_init(&ctx, &callbacks, &acc);
  memset(&iocb, 0, sizeof(iocb));
  acc.length = 0;
  iocb.fin = 0;
  iocb.opcode = WSLAY_TEXT_FRAME;
  iocb.mask = 0;
  iocb.payload_length = 3;
  iocb.data = (const uint8_t*)"Hel";
  iocb.data_length = 3;
  CU_ASSERT(3 == wslay_frame_send(ctx, &iocb));
  CU_ASSERT_EQUAL(sizeof(msg1), acc.length);
  CU_ASSERT(memcmp(msg1, acc.buf, sizeof(msg1)) == 0);

  acc.length = 0;
  iocb.fin = 1;
  iocb.opcode = WSLAY_CONTINUATION_FRAME;
  iocb.payload_length = 2;
  iocb.data = (const uint8_t*)"lo";
  iocb.data_length = 2;
  CU_ASSERT(2 == wslay_frame_send(ctx, &iocb));
  CU_ASSERT_EQUAL(sizeof(msg2), acc.length);
  CU_ASSERT(memcmp(msg2, acc.buf, sizeof(msg2)) == 0);

  wslay_frame_context_free(ctx);
}

void test_wslay_frame_send_interleaved_ctrl_frame(void)
{
  wslay_frame_context_ptr ctx;
  struct wslay_frame_callbacks callbacks = { accumulator_send_callback,
                                             NULL,
                                             static_genmask_callback };
  struct accumulator acc;
  struct wslay_frame_iocb iocb;
  /* Unmasked message */
  /* text with "Hel", with fin = 0 */
  uint8_t msg1[] = { 0x01, 0x03, 0x48, 0x65, 0x6c };
  /* ping with "Hello" */
  uint8_t msg2[] = { 0x89, 0x05, 0x48, 0x65, 0x6c, 0x6c, 0x6f };
  /* text with "lo", continuation frame for msg1, with fin = 1 */
  uint8_t msg3[] = { 0x80, 0x02, 0x6c, 0x6f };
  wslay_frame_context_init(&ctx, &callbacks, &acc);
  memset(&iocb, 0, sizeof(iocb));
  acc.length = 0;
  iocb.fin = 0;
  iocb.opcode = WSLAY_TEXT_FRAME;
  iocb.mask = 0;
  iocb.payload_length = 3;
  iocb.data = (const uint8_t*)"Hel";
  iocb.data_length = 3;
  CU_ASSERT(3 == wslay_frame_send(ctx, &iocb));
  CU_ASSERT_EQUAL(sizeof(msg1), acc.length);
  CU_ASSERT(memcmp(msg1, acc.buf, sizeof(msg1)) == 0);

  acc.length = 0;
  iocb.fin = 1;
  iocb.opcode = WSLAY_PING;
  iocb.payload_length = 5;
  iocb.data = (const uint8_t*)"Hello";
  iocb.data_length = 5;
  CU_ASSERT(5 == wslay_frame_send(ctx, &iocb));
  CU_ASSERT_EQUAL(sizeof(msg2), acc.length);
  CU_ASSERT(memcmp(msg2, acc.buf, sizeof(msg2)) == 0);

  acc.length = 0;
  iocb.fin = 1;
  iocb.opcode = WSLAY_CONTINUATION_FRAME;
  iocb.payload_length = 2;
  iocb.data = (const uint8_t*)"lo";
  iocb.data_length = 2;
  CU_ASSERT(2 == wslay_frame_send(ctx, &iocb));
  CU_ASSERT_EQUAL(sizeof(msg3), acc.length);
  CU_ASSERT(memcmp(msg3, acc.buf, sizeof(msg3)) == 0);

  wslay_frame_context_free(ctx);
}

void test_wslay_frame_send_1byte_masked(void)
{
  wslay_frame_context_ptr ctx;
  struct wslay_frame_callbacks callbacks = { scripted_send_callback,
                                             NULL,
                                             static_genmask_callback };
  struct wslay_frame_iocb iocb;
  /* Masked text frame containing "Hello" */
  uint8_t msg[] = { 0x81u, 0x85u, 0x37u, 0xfau, 0x21u, 0x3du, 0x7fu, 0x9fu,
                    0x4du, 0x51u, 0x58u };
  uint8_t hello[] = "Hello";
  struct scripted_data_feed df;
  size_t i;
  scripted_data_feed_init(&df, NULL, 0);
  for(i = 0; i < sizeof(msg); ++i) {
    df.feedseq[i] = 1;
  }
  wslay_frame_context_init(&ctx, &callbacks, &df);
  memset(&iocb, 0, sizeof(iocb));
  iocb.fin = 1;
  iocb.opcode = WSLAY_TEXT_FRAME;
  iocb.mask = 1;
  iocb.payload_length = 5;
  iocb.data = hello;
  iocb.data_length = sizeof(hello)-1;
  for(i = 0; i < 5; ++i) {
    CU_ASSERT_EQUAL(WSLAY_ERR_WANT_WRITE, wslay_frame_send(ctx, &iocb));
  }
  CU_ASSERT_EQUAL(5, wslay_frame_send(ctx, &iocb));

  wslay_frame_context_free(ctx);
}

void test_wslay_frame_send_zero_payloadlen(void)
{
  wslay_frame_context_ptr ctx;
  struct wslay_frame_callbacks callbacks = { accumulator_send_callback,
                                             NULL,
                                             static_genmask_callback };
  struct accumulator acc;
  struct wslay_frame_iocb iocb;
  /* Unmasked message */
  uint8_t msg[] = { 0x81, 0x00 }; /* "" */
  acc.length = 0;
  wslay_frame_context_init(&ctx, &callbacks, &acc);
  memset(&iocb, 0, sizeof(iocb));
  iocb.fin = 1;
  iocb.opcode = WSLAY_TEXT_FRAME;
  iocb.mask = 0;
  iocb.payload_length = 0;
  iocb.data_length = 0;
  CU_ASSERT(0 == wslay_frame_send(ctx, &iocb));
  CU_ASSERT_EQUAL(sizeof(msg), acc.length);
  CU_ASSERT(memcmp(msg, acc.buf, sizeof(msg)) == 0);

  wslay_frame_context_free(ctx);
}

void test_wslay_frame_send_too_large_payload(void)
{
  wslay_frame_context_ptr ctx;
  struct wslay_frame_callbacks callbacks;
  struct wslay_frame_iocb iocb;
  wslay_frame_context_init(&ctx, &callbacks, NULL);
  memset(&iocb, 0, sizeof(iocb));
  iocb.fin = 1;
  iocb.opcode = WSLAY_TEXT_FRAME;
  iocb.mask = 0;
  iocb.payload_length = UINT64_MAX;
  CU_ASSERT_EQUAL(WSLAY_ERR_INVALID_ARGUMENT,
                  wslay_frame_send(ctx, &iocb));

  wslay_frame_context_free(ctx);
}

void test_wslay_frame_send_ctrl_frame_too_large_payload(void)
{
  wslay_frame_context_ptr ctx;
  struct wslay_frame_callbacks callbacks;
  struct wslay_frame_iocb iocb;
  wslay_frame_context_init(&ctx, &callbacks, NULL);
  memset(&iocb, 0, sizeof(iocb));
  iocb.fin = 1;
  iocb.opcode = WSLAY_PING;
  iocb.mask = 0;
  iocb.payload_length = 1024;
  CU_ASSERT_EQUAL(WSLAY_ERR_INVALID_ARGUMENT,
                  wslay_frame_send(ctx, &iocb));

  wslay_frame_context_free(ctx);
}
